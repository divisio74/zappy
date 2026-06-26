#include "core/entities/Ressource.hpp"

Ressource::Ressource(ResourceType type, std::size_t id)
    : _type(type), _id(id)
{
}

ResourceType Ressource::getType() const
{
    return _type;
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
