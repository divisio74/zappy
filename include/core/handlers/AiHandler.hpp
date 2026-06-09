#pragma once

#include "core/world/WorldData.hpp"

class AiHandler {
public:
    explicit AiHandler(WorldData &worldData);

    void move();

private:
    WorldData &_worldData;
};
