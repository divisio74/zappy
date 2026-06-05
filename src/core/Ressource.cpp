#include "core/Ressource.hpp"

Ressource::Ressource(Vector2 coordinate, ResourceType type, std::size_t id)
    : _coordinate(coordinate), _type(type), _id(id)
{
}

const Vector2 &Ressource::getCoordinate() const
{
    return _coordinate;
}

void Ressource::setCoordinate(Vector2 coordinate)
{
    _coordinate = coordinate;
}

ResourceType Ressource::getType() const
{
    return _type;
}

void Ressource::setType(ResourceType type)
{
    _type = type;
}

std::size_t Ressource::getId() const
{
    return _id;
}
