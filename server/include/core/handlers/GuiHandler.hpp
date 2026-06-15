#pragma once

#include <vector>

#include "core/world/Tile.hpp"
#include "core/world/WorldData.hpp"

class GuiHandler {
public:
    explicit GuiHandler(const WorldData &worldData);

    const std::vector<std::vector<Tile>> &getMap() const;

private:
    const WorldData &_worldData;
};
