#pragma once
#include <vector>
#include <string>
#include <mutex>
#include "Tiles.hpp"
#include "Player.hpp"

struct GameState {

    int _width;
    int _height;
    std::vector<std::vector<Tile>> _map;
    std::vector<Player> _players;
    std::mutex _mutex;
    std::vector<std::string> _teams;
    int _timeUnit;
    bool _gameOver;
    std::string _winner;

    GameState();
};
