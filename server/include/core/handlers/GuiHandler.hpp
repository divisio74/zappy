#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/entities/Player.hpp"
#include "core/types/ResourceType.hpp"
#include "core/world/Tile.hpp"
#include "core/world/WorldData.hpp"

class GuiHandler {
public:
    explicit GuiHandler(WorldData &worldData);

    std::vector<std::string> handleCommand(const std::string &command);
    const std::vector<std::vector<Tile>> &getMap() const;

    std::string formatMapSize() const;
    std::string formatTileContent(int x, int y) const;
    std::vector<std::string> formatMapContent() const;
    std::vector<std::string> formatTeamNames() const;
    std::string formatPlayerConnection(const Player &player) const;
    std::string formatPlayerPosition(std::size_t playerId) const;
    std::string formatPlayerLevel(std::size_t playerId) const;
    std::string formatPlayerInventory(std::size_t playerId) const;
    std::string formatExpulsion(std::size_t playerId) const;
    std::string formatBroadcast(std::size_t playerId, const std::string &message) const;
    std::string formatIncantationStart(int x, int y, int level,
        const std::vector<std::size_t> &playerIds) const;
    std::string formatIncantationEnd(int x, int y, int result) const;
    std::string formatEggLaying(std::size_t playerId) const;
    std::string formatResourceDrop(std::size_t playerId, ResourceType type) const;
    std::string formatResourceCollect(std::size_t playerId, ResourceType type) const;
    std::string formatPlayerDeath(std::size_t playerId) const;
    std::string formatEggLaid(std::size_t eggId, std::size_t playerId,
        int x, int y) const;
    std::string formatEggConnection(std::size_t eggId) const;
    std::string formatEggDeath(std::size_t eggId) const;
    std::string formatTimeUnit() const;
    std::string formatEndGame(const std::string &teamName) const;
    std::string formatServerMessage(const std::string &message) const;

private:
    WorldData &_worldData;

    std::string handleMapSize(const std::vector<std::string> &args) const;
    std::string handleTileContent(const std::vector<std::string> &args) const;
    std::vector<std::string> handleMapContent(const std::vector<std::string> &args) const;
    std::vector<std::string> handleTeamNames(const std::vector<std::string> &args) const;
    std::string handlePlayerPosition(const std::vector<std::string> &args) const;
    std::string handlePlayerLevel(const std::vector<std::string> &args) const;
    std::string handlePlayerInventory(const std::vector<std::string> &args) const;
    std::string handleTimeUnit(const std::vector<std::string> &args) const;
    std::string handleTimeUnitModification(const std::vector<std::string> &args);

    bool isPositionValid(int x, int y) const;
};
