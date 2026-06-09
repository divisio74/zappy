#include "core/entities/Egg.hpp"

Egg::Egg(Vector2 coordinate, std::size_t teamId)
    : _coordinate(coordinate),
      _teamId(teamId)
{
}

const Vector2 &Egg::getCoordinate() const
{
    return _coordinate;
}

void Egg::setCoordinate(Vector2 coordinate)
{
    _coordinate = coordinate;
}

std::size_t Egg::getTeamId() const
{
    return _teamId;
}

void Egg::setTeamId(std::size_t teamId)
{
    _teamId = teamId;
}
