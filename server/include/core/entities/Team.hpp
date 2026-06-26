#pragma once

#include <cstddef>
#include <string>

class Team {
public:
    Team() = default;
    Team(const std::string &name, std::size_t maxSlot);

    std::size_t getMaxSlot() const;
    void setMaxSlot(std::size_t maxSlot);
    std::size_t getUsedSlot() const;
    void setUsedSlot(std::size_t usedSlot);
    const std::string &getName() const;

private:
    std::string _name;
    std::size_t _maxSlot = 0;
    std::size_t _usedSlot = 0;
};
