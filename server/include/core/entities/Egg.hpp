#pragma once

#include <cstddef>

#include "core/types/Vector2.hpp"

class Egg {
public:
    Egg() = default;
    Egg(Vector2 coordinate, std::size_t teamId, std::size_t id);

    const Vector2 &getCoordinate() const;
    std::size_t getTeamId() const;
    std::size_t getId() const;

private:
    Vector2 _coordinate;
    std::size_t _teamId = 0;
    std::size_t _id = 0;
};
