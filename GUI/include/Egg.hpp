#pragma once

#include <cstdint>
#include <memory>
#include <Model.hpp>

struct Egg
{
    int _id;
    int _playerId;
    int _x;
    int _y;
    bool _isAlive;

    std::shared_ptr<Model> _model;

    Egg();
};
