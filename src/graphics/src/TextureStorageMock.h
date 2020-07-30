#pragma once

#include "gmock/gmock.h"

#include "TextureStorage.h"

namespace graphics{
class TextureStorageMock : public TextureStorage {
public:
    MOCK_METHOD(boost::optional<sf::Texture&>, getTexture,(const TexturePath&));
};
}