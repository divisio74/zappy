#pragma once
#include <cstdint>

struct Egg
{
    uint32_t _id;
    uint32_t _playerId;
    int _x;
    int _y;
    bool _isAlive;
    Egg();
};
