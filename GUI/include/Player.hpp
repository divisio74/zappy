#pragma once
#include <string>
#include <tuple>
#include <cstdint>

struct Player {
    int _id;
    int _orientation;
    int _level;
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
