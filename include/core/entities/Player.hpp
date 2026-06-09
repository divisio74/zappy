#pragma once

#include <cstddef>

#include "core/types/Direction.hpp"
#include "core/types/Inventory.hpp"
#include "core/types/Vector2.hpp"

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
    bool isAlive() const;
    void setAlive(bool alive);
    std::size_t getId() const;
    std::size_t getTeamId() const;

private:
    Vector2 _coordinate;
    Inventory _inventory;
    Direction _direction = Direction::North;
    bool _alive = true;
    std::size_t _id = 0;
    std::size_t _teamId = 0;
};
