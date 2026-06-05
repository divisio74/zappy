#pragma once

#include <cstddef>

#include "core/Inventory.hpp"
#include "core/Vector2.hpp"

class Player {
public:
    Player() = default;
    Player(Vector2 coordinate, std::size_t id);

    const Vector2 &getCoordinate() const;
    void setCoordinate(Vector2 coordinate);
    Inventory &getInventory();
    const Inventory &getInventory() const;
    std::size_t getId() const;

private:
    Vector2 _coordinate;
    Inventory _inventory;
    std::size_t _id = 0;
};
