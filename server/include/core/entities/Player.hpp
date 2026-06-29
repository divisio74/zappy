#pragma once

#include <cstddef>

#include "core/types/Direction.hpp"
#include "core/types/Inventory.hpp"
#include "core/types/Vector2.hpp"
#include <string>
#include <chrono>
#include <queue>
#include <unordered_map>
#include "core/entities/Ressource.hpp"

struct CostStruct {
    int nbPlayers;
    int linemate;
    int deraumere;
    int sibur;
    int mendiane;
    int phiras;
    int thystame;
};

extern std::unordered_map<int, CostStruct> evolutionCosts;

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
    int getLevel() const;
    void setLevel(int);
    std::queue<playerAction>& getPlayerAction();
    std::size_t getId() const;
    std::size_t getTeamId() const;
    std::chrono::milliseconds::rep getWaitTime() const;
    void setWaitTime(std::chrono::milliseconds::rep time);

    void addRessource(ResourceType type);

private:
    std::queue<playerAction> _actionQueue;
    std::chrono::milliseconds::rep _waitTime;
    Vector2 _coordinate;
    Inventory _inventory;
    Direction _direction = Direction::North;
    std::size_t _id = 0;
    std::size_t _teamId = 0;
    int _level = 1;
};
