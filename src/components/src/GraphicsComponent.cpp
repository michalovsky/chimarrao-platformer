#include "GraphicsComponent.h"

#include "ComponentOwner.h"

namespace components
{

GraphicsComponent::GraphicsComponent(ComponentOwner* owner,
                                     std::shared_ptr<graphics::RendererPool> rendererPoolInit,
                                     const utils::Vector2f& size, const utils::Vector2f& position,
                                     graphics::VisibilityLayer layer)
    : Component{owner}, rendererPool{std::move(rendererPoolInit)}
{
    id = rendererPool->acquire(size, position, graphics::Color::Red, layer);
}

GraphicsComponent::GraphicsComponent(ComponentOwner* owner,
                                     std::shared_ptr<graphics::RendererPool> rendererPoolInit,
                                     const utils::Vector2f& size, const utils::Vector2f& position,
                                     const graphics::TexturePath& texturePath, graphics::VisibilityLayer layer)
    : Component{owner}, rendererPool{std::move(rendererPoolInit)}
{
    id = rendererPool->acquire(size, position, texturePath, layer);
}

void GraphicsComponent::lateUpdate(utils::DeltaTime deltaTime)
{
    rendererPool->setPosition(id, owner->transform->getPosition());
}

const graphics::GraphicsId& GraphicsComponent::getGraphicsId()
{
    return id;
}

void GraphicsComponent::setColor(const graphics::Color& color)
{
    // TODO: test
    rendererPool->setColor(id, color);
}
}