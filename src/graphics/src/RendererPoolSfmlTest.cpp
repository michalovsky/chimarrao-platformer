#include "RendererPoolSfml.h"

#include "SFML/Graphics/Texture.hpp"
#include "gtest/gtest.h"

#include "ContextRendererMock.h"
#include "FontStorageMock.h"
#include "TextureStorageMock.h"

#include "RectangleShape.h"
#include "exceptions/FontNotAvailable.h"
#include "exceptions/TextureNotAvailable.h"

using namespace ::testing;
using namespace graphics;

namespace
{
const utils::Vector2f size1{20, 30};
const utils::Vector2f size2{100, 100};
const utils::Vector2f position{0, 10};
const utils::Vector2f newPosition{42, 42};
const Color color{Color::Black};
const TexturePath validTexturePath{"validTexturePath"};
const TexturePath validTexturePath2{"validTexturePath2"};
const TexturePath invalidTexturePath{"invalidTexturePath"};
const FontPath validFontPath{"validFontPath"};
const FontPath invalidFontPath{"invalidFontPath"};
const std::string text{"text"};
const unsigned characterSize = 15;
const auto invalidId = GraphicsIdGenerator::generateId();
}

class RendererPoolSfmlTest_Base : public Test
{
public:
    RendererPoolSfmlTest_Base()
    {
        EXPECT_CALL(*contextRenderer, initialize());
        EXPECT_CALL(*contextRenderer, setView());
    }

    sf::Texture texture;
    sf::Font font;
    std::unique_ptr<ContextRendererMock> contextRendererInit{
        std::make_unique<StrictMock<ContextRendererMock>>()};
    ContextRendererMock* contextRenderer{contextRendererInit.get()};
    std::unique_ptr<TextureStorageMock> textureStorageInit{
        std::make_unique<StrictMock<TextureStorageMock>>()};
    TextureStorageMock* textureStorage{textureStorageInit.get()};
    std::unique_ptr<FontStorageMock> fontStorageInit{std::make_unique<StrictMock<FontStorageMock>>()};
    FontStorageMock* fontStorage{fontStorageInit.get()};
};

class RendererPoolSfmlTest : public RendererPoolSfmlTest_Base
{
public:
    RendererPoolSfml rendererPool{std::move(contextRendererInit), std::move(textureStorageInit),
                                  std::move(fontStorageInit)};
};

TEST_F(RendererPoolSfmlTest, acquireShapeWithColor_positionShouldMatch)
{
    const auto shapeId = rendererPool.acquire(size1, position, color);

    const auto actualShapePosition = rendererPool.getPosition(shapeId);

    ASSERT_EQ(*actualShapePosition, position);
}

TEST_F(RendererPoolSfmlTest, acquireShapeWithTexture_textureNotAvailable_shouldThrowTextureNotAvailable)
{
    EXPECT_CALL(*textureStorage, getTexture(invalidTexturePath))
        .WillOnce(Throw(exceptions::TextureNotAvailable{""}));

    ASSERT_THROW(rendererPool.acquire(size1, position, invalidTexturePath), exceptions::TextureNotAvailable);
}

TEST_F(RendererPoolSfmlTest, acquireShapeWithTexture_textureAvailable_positionShouldMatch)
{
    EXPECT_CALL(*textureStorage, getTexture(validTexturePath)).WillOnce(ReturnRef(texture));

    const auto shapeId = rendererPool.acquire(size1, position, validTexturePath);

    const auto actualShapePosition = rendererPool.getPosition(shapeId);
    ASSERT_EQ(actualShapePosition, position);
}

TEST_F(RendererPoolSfmlTest, acquireText_fontNotAvailable_shouldThrowFontNotAvailable)
{
    EXPECT_CALL(*fontStorage, getFont(invalidFontPath)).WillOnce(Throw(exceptions::FontNotAvailable{""}));

    ASSERT_THROW(rendererPool.acquireText(position, text, invalidFontPath, characterSize),
                 exceptions::FontNotAvailable);
}

TEST_F(RendererPoolSfmlTest, acquireText_fontAvailable_positionShouldMatch)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));

    const auto textId = rendererPool.acquireText(position, text, validFontPath, characterSize);

    const auto actualTextPosition = rendererPool.getPosition(textId);
    ASSERT_EQ(actualTextPosition, position);
}

TEST_F(RendererPoolSfmlTest, getPositionWithInvalidId_shouldReturnNone)
{
    const auto actualPosition = rendererPool.getPosition(invalidId);

    ASSERT_EQ(actualPosition, boost::none);
}

TEST_F(RendererPoolSfmlTest, setPositionWithInvalidId_shouldDoNothingAndNoThrow)
{
    ASSERT_NO_THROW(rendererPool.setPosition(invalidId, position));
}

TEST_F(RendererPoolSfmlTest, setPositionWithShape)
{
    const auto shapeId = rendererPool.acquire(size1, position, color);

    rendererPool.setPosition(shapeId, newPosition);

    ASSERT_EQ(rendererPool.getPosition(shapeId), newPosition);
}

TEST_F(RendererPoolSfmlTest, setPositionWithText)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));
    const auto textId = rendererPool.acquireText(position, text, validFontPath, characterSize);

    rendererPool.setPosition(textId, newPosition);

    ASSERT_EQ(rendererPool.getPosition(textId), newPosition);
}

TEST_F(RendererPoolSfmlTest, renderShape)
{
    rendererPool.acquire(size1, position, color);
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_));

    rendererPool.renderAll();
}

TEST_F(RendererPoolSfmlTest, renderText)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));
    rendererPool.acquireText(position, text, validFontPath, characterSize);
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_));

    rendererPool.renderAll();
}

TEST_F(RendererPoolSfmlTest, renderShapesAndTexts)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));
    rendererPool.acquireText(position, text, validFontPath, characterSize);
    rendererPool.acquire(size1, position, color);
    rendererPool.acquire(size1, position, color);
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(3);

    rendererPool.renderAll();
}

ACTION_P(addGraphicsIdToVector, graphicsIds)
{
    if (const auto* rectangleShape = dynamic_cast<const RectangleShape*>(&arg0); rectangleShape != nullptr)
    {
        graphicsIds->push_back(rectangleShape->getGraphicsId());
        return;
    }

    if (const auto* text = dynamic_cast<const Text*>(&arg0); text != nullptr)
    {
        graphicsIds->push_back(text->getGraphicsId());
        return;
    }
}

TEST_F(RendererPoolSfmlTest, renderShapesInLayers)
{
    const auto firstLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::First);
    const auto thirdLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Third);
    const auto backgroundGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Background);
    const auto secondLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Second);
    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(4).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));

    rendererPool.renderAll();

    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
    EXPECT_EQ(graphicsIds[1], thirdLayerGraphicsId);
    EXPECT_EQ(graphicsIds[2], secondLayerGraphicsId);
    EXPECT_EQ(graphicsIds[3], firstLayerGraphicsId);
}

TEST_F(RendererPoolSfmlTest, setShapeVisibility_shouldChangeOrderOfShapes)
{
    const auto firstLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::First);
    const auto secondLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Second);
    const auto backgroundGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Background);

    rendererPool.setVisibility(firstLayerGraphicsId, VisibilityLayer::Third);

    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(3).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));
    rendererPool.renderAll();
    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
    EXPECT_EQ(graphicsIds[1], firstLayerGraphicsId);
    EXPECT_EQ(graphicsIds[2], secondLayerGraphicsId);
}

TEST_F(RendererPoolSfmlTest, setVisibilityLayerToInvisibility_shuldNotRenderThisShape)
{
    const auto firstLayerGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::First);
    const auto backgroundGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Background);

    rendererPool.setVisibility(firstLayerGraphicsId, VisibilityLayer::Invisible);

    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));
    rendererPool.renderAll();
    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
}

TEST_F(RendererPoolSfmlTest, renderTextsInLayers)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillRepeatedly(ReturnRef(font));
    const auto firstLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::First);
    const auto thirdLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Third);
    const auto backgroundGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Background);
    const auto secondLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Second);
    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(4).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));

    rendererPool.renderAll();

    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
    EXPECT_EQ(graphicsIds[1], thirdLayerGraphicsId);
    EXPECT_EQ(graphicsIds[2], secondLayerGraphicsId);
    EXPECT_EQ(graphicsIds[3], firstLayerGraphicsId);
}

TEST_F(RendererPoolSfmlTest, setTextVisibility_shouldChangeOrderOfTexts)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillRepeatedly(ReturnRef(font));
    const auto firstLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::First);
    const auto secondLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Second);
    const auto backgroundGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Background);

    rendererPool.setVisibility(firstLayerGraphicsId, VisibilityLayer::Third);

    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(3).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));
    rendererPool.renderAll();
    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
    EXPECT_EQ(graphicsIds[1], firstLayerGraphicsId);
    EXPECT_EQ(graphicsIds[2], secondLayerGraphicsId);
}

TEST_F(RendererPoolSfmlTest, setVisibilityLayerToInvisibility_shuldNotRenderThisText)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillRepeatedly(ReturnRef(font));
    const auto firstLayerGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::First);
    const auto backgroundGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Background);

    rendererPool.setVisibility(firstLayerGraphicsId, VisibilityLayer::Invisible);

    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));
    rendererPool.renderAll();
    EXPECT_EQ(graphicsIds[0], backgroundGraphicsId);
}

TEST_F(RendererPoolSfmlTest, shapesShouldBeRenderedBeforeTexts)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillRepeatedly(ReturnRef(font));
    const auto firstLayerTextGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::First);
    const auto backgroundTextGraphicsId =
        rendererPool.acquireText(position, text, validFontPath, characterSize, VisibilityLayer::Background);
    const auto secondLayerShapeGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Second);
    const auto backgroundShapeGraphicsId =
        rendererPool.acquire(size1, position, Color::Red, VisibilityLayer::Background);
    std::vector<GraphicsId> graphicsIds;
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());
    EXPECT_CALL(*contextRenderer, draw(_)).Times(4).WillRepeatedly(addGraphicsIdToVector(&graphicsIds));

    rendererPool.renderAll();

    EXPECT_EQ(graphicsIds[0], backgroundShapeGraphicsId);
    EXPECT_EQ(graphicsIds[1], secondLayerShapeGraphicsId);
    EXPECT_EQ(graphicsIds[2], backgroundTextGraphicsId);
    EXPECT_EQ(graphicsIds[3], firstLayerTextGraphicsId);
}

TEST_F(RendererPoolSfmlTest, setVisibilityWithInvalidGraphicsId_shouldNotThrow)
{
    ASSERT_NO_THROW(rendererPool.setVisibility(invalidId, VisibilityLayer::Invisible));
}

TEST_F(RendererPoolSfmlTest, releasedShape_shouldNotBeRendered)
{
    const auto id = rendererPool.acquire(size1, position, color);
    rendererPool.release(id);
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());

    rendererPool.renderAll();
}

TEST_F(RendererPoolSfmlTest, releasedText_shouldNotBeRendered)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));
    const auto id = rendererPool.acquireText(position, text, validFontPath, characterSize);
    rendererPool.release(id);
    EXPECT_CALL(*contextRenderer, clear(sf::Color::White));
    EXPECT_CALL(*contextRenderer, setView());

    rendererPool.renderAll();
}

TEST_F(RendererPoolSfmlTest, setTextureWithValidTexturePath_shouldNoThrow)
{
    const auto shapeId = rendererPool.acquire(size1, position, color);
    EXPECT_CALL(*textureStorage, getTexture(validTexturePath2)).WillOnce(ReturnRef(texture));

    ASSERT_NO_THROW(rendererPool.setTexture(shapeId, validTexturePath2));
}

TEST_F(RendererPoolSfmlTest, setTextureWithInvalidTexturePath_shouldThrowTextureNotAvailable)
{
    const auto shapeId = rendererPool.acquire(size1, position, color);
    EXPECT_CALL(*textureStorage, getTexture(invalidTexturePath))
        .WillOnce(Throw(exceptions::TextureNotAvailable{""}));

    ASSERT_THROW(rendererPool.setTexture(shapeId, invalidTexturePath), exceptions::TextureNotAvailable);
}

TEST_F(RendererPoolSfmlTest, setTextureWithInvalidGraphicsId_shouldNotThrow)
{
    ASSERT_NO_THROW(rendererPool.setTexture(invalidId, validTexturePath));
}

TEST_F(RendererPoolSfmlTest, setTextWithInvalidGraphicsId_shouldNotThrow)
{
    ASSERT_NO_THROW(rendererPool.setText(invalidId, text));
}

TEST_F(RendererPoolSfmlTest, setTextWithValidGraphicsId_shouldNotThrow)
{
    EXPECT_CALL(*fontStorage, getFont(validFontPath)).WillOnce(ReturnRef(font));
    const auto textId = rendererPool.acquireText(position, text, validFontPath, characterSize);

    ASSERT_NO_THROW(rendererPool.setText(textId, text));
}

// TODO: test setColor with validId
TEST_F(RendererPoolSfmlTest, setColorWithInvalidGraphicsId_shouldNotThrow)
{
    ASSERT_NO_THROW(rendererPool.setColor(invalidId, sf::Color::Red));
}

TEST_F(RendererPoolSfmlTest, synchronizeRenderingSize)
{
    EXPECT_CALL(*contextRenderer, synchronizeViewSize());

    rendererPool.synchronizeRenderingSize();
}