#include "core/entities/Ressource.hpp"

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

std::string Ressource::toString() const
{
    switch (_type) {
        case ResourceType::Food:      return "food";
        case ResourceType::Linemate:  return "linemate";
        case ResourceType::Deraumere: return "deraumere";
        case ResourceType::Sibur:     return "sibur";
        case ResourceType::Mendiane:  return "mendiane";
        case ResourceType::Phiras:    return "phiras";
        case ResourceType::Thystame:  return "thystame";
        default:                      return "unknown";
    }
}
