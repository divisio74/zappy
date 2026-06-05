#include "core/Player.hpp"

Player::Player(Vector2 coordinate, std::size_t id)
    : _coordinate(coordinate), _id(id)
{
}

const Vector2 &Player::getCoordinate() const
{
    return _coordinate;
}

void Player::setCoordinate(Vector2 coordinate)
{
    _coordinate = coordinate;
}

Inventory &Player::getInventory()
{
    return _inventory;
}

const Inventory &Player::getInventory() const
{
    return _inventory;
}

std::size_t Player::getId() const
{
    return _id;
}
