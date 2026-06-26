#pragma once

#include <string>

struct Inventory {
    int food = 10;
    int linemate = 0;
    int deraumere = 0;
    int sibur = 0;
    int mendiane = 0;
    int phiras = 0;
    int thystame = 0;

    std::string toString() const {
        return "[food " + std::to_string(food) +
               ", linemate " + std::to_string(linemate) +
               ", deraumere " + std::to_string(deraumere) +
               ", sibur " + std::to_string(sibur) +
               ", mendiane " + std::to_string(mendiane) +
               ", phiras " + std::to_string(phiras) +
               ", thystame " + std::to_string(thystame) +
               "]";
    }
};
