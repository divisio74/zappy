#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "core/types/ResourceType.hpp"

enum class GuiEventType {
    MapSize,
    TileContent,
    MapContent,
    TeamNames,
    PlayerConnection,
    PlayerPosition,
    PlayerLevel,
    PlayerInventory,
    Expulsion,
    Broadcast,
    IncantationStart,
    IncantationEnd,
    EggLaying,
    ResourceDrop,
    ResourceCollect,
    PlayerDeath,
    EggLaid,
    EggConnection,
    EggDeath,
    TimeUnit,
    TimeUnitModification,
    EndGame,
    ServerMessage,
    UnknownCommand,
    BadParameter
};

struct GuiEvent {
    GuiEventType type = GuiEventType::ServerMessage;
    int x = 0;
    int y = 0;
    int level = 0;
    int result = 0;
    int timeUnit = 0;
    std::size_t playerId = 0;
    std::size_t eggId = 0;
    ResourceType resource = ResourceType::Food;
    std::string teamName;
    std::string message;
    std::vector<std::size_t> playerIds;
};
