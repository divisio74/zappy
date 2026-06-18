#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "core/entities/Player.hpp"

class Team {
public:
    Team() = default;
    explicit Team(std::size_t id);
    Team(std::size_t id, const std::string &name, std::size_t maxSlot);

    std::vector<std::shared_ptr<Player>> &getPlayers();
    const std::vector<std::shared_ptr<Player>> &getPlayers() const;
    std::size_t getMaxSlot() const;
    void setMaxSlot(std::size_t maxSlot);
    std::size_t getUsedSlot() const;
    void setUsedSlot(std::size_t usedSlot);
    std::size_t getId() const;
    const std::string &getName() const;

private:
    std::vector<std::shared_ptr<Player>> _players;
    std::string _name;
    std::size_t _maxSlot = 0;
    std::size_t _usedSlot = 0;
    std::size_t _id = 0;
};
