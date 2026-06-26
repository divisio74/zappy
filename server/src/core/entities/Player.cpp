#include "core/entities/Player.hpp"

std::unordered_map<int, CostStruct> evolutionCosts = {
    {1, {1, 1, 0, 0, 0, 0, 0}},  // 1->2
    {2, {2, 1, 1, 1, 0, 0, 0}},  // 2->3
    {3, {2, 2, 0, 1, 0, 2, 0}},  // 3->4
    {4, {4, 1, 1, 2, 0, 1, 0}},  // 4->5
    {5, {4, 1, 2, 1, 3, 0, 0}},  // 5->6
    {6, {6, 1, 2, 3, 0, 1, 0}},  // 6->7
    {7, {6, 2, 2, 2, 2, 2, 1}},  // 7->8
};

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

int Player::getLevel() const
{
    return _level;
}

void Player::setLevel(int newLevel)
{
    _level = newLevel;
}

std::queue<playerAction> &Player::getPlayerAction()
{
    return _actionQueue;
}

std::size_t Player::getId() const
{
    return _id;
}

std::size_t Player::getTeamId() const
{
    return _teamId;
}

void Player::addRessource(ResourceType type)
{
    switch (type) {
        case ResourceType::Food:
            _inventory.food++;
            break;
        case ResourceType::Linemate:
            _inventory.linemate++;
            break;
        case ResourceType::Deraumere:
            _inventory.deraumere++;
            break;
        case ResourceType::Sibur:
            _inventory.sibur++;
            break;
        case ResourceType::Mendiane:
            _inventory.mendiane++;
            break;
        case ResourceType::Phiras:
            _inventory.phiras++;
            break;
        case ResourceType::Thystame:
            _inventory.thystame++;
            break;
    }
}
