#include "core/handlers/GuiHandler.hpp"

GuiHandler::GuiHandler(const WorldData &worldData)
    : _worldData(worldData)
{
}

const std::vector<std::vector<Tile>> &GuiHandler::getMap() const
{
    return _worldData.map;
}
