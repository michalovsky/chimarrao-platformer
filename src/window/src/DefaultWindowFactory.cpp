#include "DefaultWindowFactory.h"

#include "DefaultWindowObservationHandler.h"
#include "WindowSfml.h"

namespace window
{

std::unique_ptr<Window> DefaultWindowFactory::createWindow(const utils::Vector2u& windowSize,
                                                           const std::string& title) const
{
    return std::make_unique<WindowSfml>(windowSize, title,
                                        std::make_unique<DefaultWindowObservationHandler>());
}
}