#include "core/entities/Team.hpp"

Team::Team(const std::string &name, std::size_t maxSlot)
    : _name(name),
      _maxSlot(maxSlot)
{
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

const std::string &Team::getName() const
{
    return _name;
}
