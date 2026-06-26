#pragma once

#include <cstddef>
#include <string>

#include "core/types/ResourceType.hpp"

class Ressource {
public:
    Ressource() = default;
    Ressource(ResourceType type, std::size_t id);

    ResourceType getType() const;
    std::size_t getId() const;

    std::string toString() const;

private:
    ResourceType _type = ResourceType::Food;
    std::size_t _id = 0;
};
