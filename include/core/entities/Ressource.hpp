#pragma once

#include <cstddef>

#include "core/types/ResourceType.hpp"
#include "core/types/Vector2.hpp"

class Ressource {
public:
    Ressource() = default;
    Ressource(Vector2 coordinate, ResourceType type, std::size_t id);

    const Vector2 &getCoordinate() const;
    void setCoordinate(Vector2 coordinate);
    ResourceType getType() const;
    void setType(ResourceType type);
    std::size_t getId() const;

private:
    Vector2 _coordinate;
    ResourceType _type = ResourceType::Food;
    std::size_t _id = 0;
};
