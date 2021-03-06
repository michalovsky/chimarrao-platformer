#include "KeyboardMovementComponent.h"

#include "gtest/gtest.h"

#include "AnimatorMock.h"
#include "InputManagerMock.h"

#include "AnimationComponent.h"
#include "ComponentOwner.h"
#include "DeltaTime.h"

using namespace ::testing;
using namespace input;
using namespace components::core;
using namespace animations;

class KeyboardMovementComponentTest_Base : public Test
{
public:
    KeyboardMovementComponentTest_Base()
    {
        EXPECT_CALL(*inputManager, registerObserver(_));
        EXPECT_CALL(*inputManager, removeObserver(_));
    }

    std::shared_ptr<StrictMock<InputManagerMock>> inputManager =
        std::make_shared<StrictMock<InputManagerMock>>();
};

class KeyboardMovementComponentTest : public KeyboardMovementComponentTest_Base
{
public:
    KeyboardMovementComponentTest()
    {
        componentOwner.addComponent<AnimationComponent>(animator);
        keyboardMovementComponent.loadDependentComponents();
    }

    InputStatus prepareInputStatus(InputKey inputKey)
    {
        InputStatus inputStatus;
        inputStatus.setKeyPressed(inputKey);
        return inputStatus;
    }

    void expectRightKeyPressed(const InputStatus& inputStatus)
    {
        EXPECT_CALL(*animator, setAnimationDirection(AnimationDirection::Right));
        EXPECT_CALL(*animator, setAnimation(AnimationType::Walk));
        keyboardMovementComponent.handleInputStatus(inputStatus);
    }

    void expectLeftKeyPressed(const InputStatus& inputStatus)
    {
        EXPECT_CALL(*animator, setAnimationDirection(AnimationDirection::Left));
        EXPECT_CALL(*animator, setAnimation(AnimationType::Walk));
        keyboardMovementComponent.handleInputStatus(inputStatus);
    }

    const float movementSpeed{10.0};
    utils::DeltaTime deltaTime{3};
    const utils::Vector2f position{0.0, 11.0};
    ComponentOwner componentOwner{position};
    std::shared_ptr<StrictMock<AnimatorMock>> animator = std::make_shared<StrictMock<AnimatorMock>>();

    KeyboardMovementComponent keyboardMovementComponent{&componentOwner, inputManager};
};

TEST_F(KeyboardMovementComponentTest, givenInputStatusWithNoKeyPressed_shouldSetAnimationTypeToIdle)
{
    const auto noKeyInputStatus = InputStatus{};
    keyboardMovementComponent.handleInputStatus(noKeyInputStatus);
    EXPECT_CALL(*animator, setAnimation(AnimationType::Idle));

    keyboardMovementComponent.update(deltaTime);
}

TEST_F(KeyboardMovementComponentTest,
       givenInputStatusWithRightKeyPressed_shouldSetAnimationDirectionToRightAndSetAnimationTypeToWalk)
{
    InputStatus rightKeyInputStatus = prepareInputStatus(InputKey::Right);
    keyboardMovementComponent.handleInputStatus(rightKeyInputStatus);
    EXPECT_CALL(*animator, setAnimationDirection(AnimationDirection::Right));
    EXPECT_CALL(*animator, setAnimation(AnimationType::Walk));

    keyboardMovementComponent.update(deltaTime);
}

TEST_F(KeyboardMovementComponentTest,
       givenInputStatusWithLeftKeyPressed_shouldSetAnimationDirectionToLeftAndSetAnimationTypeToWalk)
{
    const auto leftKeyInputStatus = prepareInputStatus(InputKey::Left);
    keyboardMovementComponent.handleInputStatus(leftKeyInputStatus);
    EXPECT_CALL(*animator, setAnimationDirection(AnimationDirection::Left));
    EXPECT_CALL(*animator, setAnimation(AnimationType::Walk));

    keyboardMovementComponent.update(deltaTime);
}

TEST_F(KeyboardMovementComponentTest, setMovementSpeed)
{
    keyboardMovementComponent.setMovementSpeed(movementSpeed);

    ASSERT_EQ(keyboardMovementComponent.getMovementSpeed(), movementSpeed);
}

TEST_F(KeyboardMovementComponentTest,
       givenRightKeyPressed_update_shouldAddPositionChangeFromInputToTransformComponent)
{
    InputStatus rightKeyInputStatus = prepareInputStatus(InputKey::Right);
    expectRightKeyPressed(rightKeyInputStatus);
    const auto positionBeforeUpdate = componentOwner.transform->getPosition();

    keyboardMovementComponent.update(deltaTime);

    const auto positionChangeToRight = deltaTime.count() * keyboardMovementComponent.getMovementSpeed();
    const auto expectedPositionAfterUpdate =
        utils::Vector2f{positionBeforeUpdate.x + positionChangeToRight, positionBeforeUpdate.y};
    const auto positionAfterUpdate = componentOwner.transform->getPosition();
    ASSERT_EQ(positionAfterUpdate, expectedPositionAfterUpdate);
}

TEST_F(KeyboardMovementComponentTest,
       givenLeftKeyPressed_update_shouldAddPositionChangeFromInputToTransformComponent)
{
    const auto leftKeyInputStatus = prepareInputStatus(InputKey::Left);
    expectLeftKeyPressed(leftKeyInputStatus);
    const auto positionBeforeUpdate = componentOwner.transform->getPosition();

    keyboardMovementComponent.update(deltaTime);

    const auto positionChangeToLeft = -deltaTime.count() * keyboardMovementComponent.getMovementSpeed();
    const auto expectedPositionAfterUpdate =
        utils::Vector2f{positionBeforeUpdate.x + positionChangeToLeft, positionBeforeUpdate.y};
    const auto positionAfterUpdate = componentOwner.transform->getPosition();
    ASSERT_EQ(positionAfterUpdate, expectedPositionAfterUpdate);
}