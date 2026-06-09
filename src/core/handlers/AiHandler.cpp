#include "core/handlers/AiHandler.hpp"

AiHandler::AiHandler(WorldData &worldData)
    : _worldData(worldData)
{
}

void AiHandler::move()
{
    static_cast<void>(_worldData);
}
