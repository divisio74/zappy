#pragma once

#include "Model.hpp"
#include <memory>
#include <vector>

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

    std::vector<std::shared_ptr<Model>> models;

    Tile();
};
