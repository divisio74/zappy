#pragma once

#include <cstddef>

#include "core/types/Direction.hpp"
#include "core/types/Inventory.hpp"
#include "core/types/Vector2.hpp"
#include <string>
#include <chrono>
#include <queue>
#include <unordered_map>

struct EvolutionCost {
    int nbPlayers;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
};

std::unordered_map<int, EvolutionCost> evolutionCosts = {
    {1, {1, 1, 0, 0, 0, 0, 0}},  // 1->2
    {2, {2, 1, 1, 1, 0, 0, 0}},  // 2->3
    {3, {2, 2, 0, 1, 0, 2, 0}},  // 3->4
    {4, {4, 1, 1, 2, 0, 1, 0}},  // 4->5
    {5, {4, 1, 2, 1, 3, 0, 0}},  // 5->6
    {6, {6, 1, 2, 3, 0, 1, 0}},  // 6->7
    {7, {6, 2, 2, 2, 2, 2, 1}},  // 7->8
};

struct playerAction
{
    std::string command;
    std::chrono::steady_clock::time_point executeAt;
    std::chrono::milliseconds time;
};

class Player {
public:
    Player() = default;
    Player(Vector2 coordinate, Direction direction, std::size_t id, std::size_t teamId);

    const Vector2 &getCoordinate() const;
    void setCoordinate(Vector2 coordinate);
    Inventory &getInventory();
    const Inventory &getInventory() const;
    Direction getDirection() const;
    void setDirection(Direction direction);
    std::chrono::steady_clock::time_point getWaitTime() const;
    void setWaitTime(std::chrono::steady_clock::time_point);
    bool isAlive() const;
    void setAlive(bool alive);
    int getLevel() const;
    void setLevel(int);
    bool isFrozen() const;
    void setFrozen(bool frozen);
    std::queue<playerAction>& getPlayerAction();
    void addPlayerAction(playerAction action);
    std::size_t getId() const;
    std::size_t getTeamId() const;

private:
    std::queue<playerAction> _actionQueue;
    std::chrono::steady_clock::time_point _waitTime;
    Vector2 _coordinate;
    Inventory _inventory;
    Direction _direction = Direction::North;
    bool _alive = true;
    std::size_t _id = 0;
    std::size_t _teamId = 0;
    bool _isFrozen = false;
    int _level = 1;
};
