#pragma once
#include <string>
#include <tuple>
#include <cstdint>

struct Player {
    uint32_t _id;
    int _orientation;
    uint32_t _level;
    std::string _teamName;
    bool _isAlive;
    std::tuple<int, int> _pos;
};
