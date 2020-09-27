#include "KeyboardMovementComponent.h"

#include "AnimationComponent.h"
#include "ComponentOwner.h"
#include "exceptions/DependentComponentNotFound.h"

namespace components
{

KeyboardMovementComponent::KeyboardMovementComponent(ComponentOwner* owner,
                                                     std::shared_ptr<input::InputManager> inputManagerInit)
    : Component{owner}, inputManager{std::move(inputManagerInit)}, movementSpeed{10.f}
{
    inputManager->registerObserver(this);
}

KeyboardMovementComponent::~KeyboardMovementComponent()
{
    inputManager->removeObserver(this);
}

void KeyboardMovementComponent::loadDependentComponents()
{
    animation = owner->getComponent<AnimationComponent>();
    if (not animation)
    {
        throw exceptions::DependentComponentNotFound{"KeyboardMovementComponent: Animation component not found"};
    }
}

void KeyboardMovementComponent::update(utils::DeltaTime deltaTime)
{
    float xFrameMove = currentMovementSpeed.x * deltaTime.count();
    float yFrameMove = currentMovementSpeed.y * deltaTime.count();
    owner->transform->addPosition(xFrameMove, yFrameMove);
}

void KeyboardMovementComponent::handleInputStatus(const input::InputStatus& inputStatus)
{
    currentMovementSpeed.x = 0;
    if (inputStatus.isKeyPressed(input::InputKey::Left))
    {
        currentMovementSpeed.x = -movementSpeed;
        animation->setAnimationDirection(animations::AnimationDirection::Left);
    }
    else if (inputStatus.isKeyPressed(input::InputKey::Right))
    {
        currentMovementSpeed.x = movementSpeed;
        animation->setAnimationDirection(animations::AnimationDirection::Right);
    }

    currentMovementSpeed.y = 0;
    if (inputStatus.isKeyPressed(input::InputKey::Up))
    {
        currentMovementSpeed.y = -movementSpeed;
    }
    else if (inputStatus.isKeyPressed(input::InputKey::Down))
    {
        currentMovementSpeed.y = movementSpeed;
    }

    if (currentMovementSpeed.x == 0 && currentMovementSpeed.y == 0)
    {
        animation->setAnimation(animations::AnimationType::Idle);
    }
    else
    {
        animation->setAnimation(animations::AnimationType::Walk);
    }
}

void KeyboardMovementComponent::setMovementSpeed(float speed)
{
    movementSpeed = speed;
}

float KeyboardMovementComponent::getMovementSpeed() const
{
    return movementSpeed;
}

}