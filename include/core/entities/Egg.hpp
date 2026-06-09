#pragma once

#include <cstddef>

#include "core/types/Vector2.hpp"

class Egg {
public:
    Egg() = default;
    Egg(Vector2 coordinate, std::size_t teamId);

    const Vector2 &getCoordinate() const;
    void setCoordinate(Vector2 coordinate);
    std::size_t getTeamId() const;
    void setTeamId(std::size_t teamId);

private:
    Vector2 _coordinate;
    std::size_t _teamId = 0;
};
