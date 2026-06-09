#include "core/entities/Player.hpp"

Player::Player(Vector2 coordinate, Direction direction, std::size_t id, std::size_t teamId)
    : _coordinate(coordinate),
      _direction(direction),
      _id(id),
      _teamId(teamId)
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

Direction Player::getDirection() const
{
    return _direction;
}

void Player::setDirection(Direction direction)
{
    _direction = direction;
}

bool Player::isAlive() const
{
    return _alive;
}

void Player::setAlive(bool alive)
{
    _alive = alive;
}

std::size_t Player::getId() const
{
    return _id;
}

std::size_t Player::getTeamId() const
{
    return _teamId;
}
