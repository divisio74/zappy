#include "core/entities/Egg.hpp"

Egg::Egg(Vector2 coordinate, std::size_t teamId, std::size_t id)
    : _coordinate(coordinate),
      _teamId(teamId),
      _id(id)
{
}

const Vector2 &Egg::getCoordinate() const
{
    return _coordinate;
}

std::size_t Egg::getTeamId() const
{
    return _teamId;
}

std::size_t Egg::getId() const
{
    return _id;
}
