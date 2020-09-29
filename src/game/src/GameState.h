#pragma once

#include "ComponentOwner.h"
#include "State.h"

namespace game
{
class GameState : public State
{
public:
    explicit GameState(const std::shared_ptr<window::Window>&, const std::shared_ptr<input::InputManager>&,
                           const std::shared_ptr<graphics::RendererPool>&, std::stack<std::unique_ptr<State>>&);

    void initialize();
    void update(const utils::DeltaTime&) override;
    void lateUpdate(const utils::DeltaTime&) override;
    void render() override;
    std::string getName() const override;

private:
    std::shared_ptr<components::ComponentOwner> player;
    std::shared_ptr<components::ComponentOwner> background;
};
}