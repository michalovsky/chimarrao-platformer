#include "GraphicsComponent.h"

#include "ComponentOwner.h"

namespace components::core
{

GraphicsComponent::GraphicsComponent(ComponentOwner* ownerInit,
                                     std::shared_ptr<graphics::RendererPool> rendererPoolInit,
                                     const utils::Vector2f& size, const utils::Vector2f& position,
                                     const graphics::Color& color, graphics::VisibilityLayer layer)
    : Component{ownerInit}, rendererPool{std::move(rendererPoolInit)}, visibilityLayer{layer}
{
    id = rendererPool->acquire(size, position, color, layer);
}

GraphicsComponent::GraphicsComponent(ComponentOwner* owner,
                                     std::shared_ptr<graphics::RendererPool> rendererPoolInit,
                                     const utils::Vector2f& size, const utils::Vector2f& position,
                                     const graphics::TexturePath& texturePath,
                                     graphics::VisibilityLayer layer)
    : Component{owner}, rendererPool{std::move(rendererPoolInit)}, visibilityLayer{layer}
{
    id = rendererPool->acquire(size, position, texturePath, layer);
}

GraphicsComponent::~GraphicsComponent()
{
    rendererPool->release(id);
}

void GraphicsComponent::lateUpdate(utils::DeltaTime deltaTime)
{
    if (enabled)
    {
        rendererPool->setPosition(id, owner->transform->getPosition());
    }
}

const graphics::GraphicsId& GraphicsComponent::getGraphicsId()
{
    return id;
}

void GraphicsComponent::setColor(const graphics::Color& color)
{
    rendererPool->setColor(id, color);
}

void GraphicsComponent::setVisibility(graphics::VisibilityLayer layer)
{
    rendererPool->setVisibility(id, layer);
    visibilityLayer = layer;
}

void GraphicsComponent::enable()
{
    Component::enable();
    setVisibility(visibilityLayer);
}

void GraphicsComponent::disable()
{
    Component::disable();
    rendererPool->setVisibility(id, graphics::VisibilityLayer::Invisible);
}
void GraphicsComponent::setOutline(float thickness, const sf::Color& color)
{
    rendererPool->setOutline(id, thickness, color);
}
void GraphicsComponent::setTexture(const std::string& texturePath)
{
    rendererPool->setTexture(id, texturePath);
}
}