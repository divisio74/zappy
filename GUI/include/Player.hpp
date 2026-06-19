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
    int _x;
    int _y;
    int _food;
    int _linemate;
    int _deraumere;
    int _sibur;
    int _mendiane;
    int _phiras;
    int _thystame;

    Player();
};
