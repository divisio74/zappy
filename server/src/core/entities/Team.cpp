#include "core/entities/Team.hpp"

Team::Team(std::size_t id)
    : _id(id)
{
}

Team::Team(std::size_t id, const std::string &name, std::size_t maxSlot)
    : _name(name),
      _maxSlot(maxSlot),
      _id(id)
{
}

std::vector<std::shared_ptr<Player>> &Team::getPlayers()
{
    return _players;
}

const std::vector<std::shared_ptr<Player>> &Team::getPlayers() const
{
    return _players;
}

std::size_t Team::getMaxSlot() const
{
    return _maxSlot;
}

void Team::setMaxSlot(std::size_t maxSlot)
{
    _maxSlot = maxSlot;
}

std::size_t Team::getUsedSlot() const
{
    return _usedSlot;
}

void Team::setUsedSlot(std::size_t usedSlot)
{
    _usedSlot = usedSlot;
}

std::size_t Team::getId() const
{
    return _id;
}

const std::string &Team::getName() const
{
    return _name;
}
