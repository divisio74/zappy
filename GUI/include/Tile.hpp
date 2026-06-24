#pragma once

#include <vector>
#include <memory>
#include "Resource.hpp"

struct Tile {
    int food;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;

    float x;
    float z;

    std::vector<Resource> _resources;

    Tile();
};
