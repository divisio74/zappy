#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "core/handlers/AiHandler.hpp"
#include "core/handlers/GuiEvent.hpp"
#include "core/handlers/GuiHandler.hpp"
#include "core/world/WorldData.hpp"
#include "core/world/WorldHandling.hpp"
#include "network/SocketRegistry.hpp"
#include "parser/Parser.hpp"

class Core {
public:
    explicit Core(const ServerConfig &config);

    void runServer();
    void processServerTick();
    void queueGuiEvent(const GuiEvent &event);

private:
    void processClientCommands(std::size_t socketId);
    void greetNewClient(std::size_t socketId);
    void identifyClient(std::size_t socketId, const std::string &command);
    void processAiClientCommand(std::size_t socketId,
        const std::string &command);
    void processGuiClientCommand(std::size_t socketId,
        const std::string &command);
    void cleanupDisconnectedClient(std::size_t socketId);
    void processTimedWorldEvents();
    void processFoodConsumption();
    void killPlayer(std::size_t playerId);
    void processAiActions();
    void sendAiMessages();
    void sendGuiMessages();
    void sendGuiInitialState(std::size_t socketId);
    std::vector<std::string> formatGuiEvent(const GuiEvent &event) const;
    void sendAiConnectionData(std::size_t socketId, std::size_t playerId);
    void updateTickDuration();
    std::optional<std::size_t> findTeamByName(const std::string &teamName) const;
    std::optional<std::size_t> spawnPlayerFromTeamEgg(std::size_t teamId);
    int getPollTimeoutMs() const;

    WorldData _worldData;
    WorldHandling _worldHandling;
    SocketRegistry _socketRegistry;
    GuiHandler _guiHandler;
    AiHandler _aiHandler;
    std::queue<GuiEvent> _guiEvents;
    std::vector<std::string> _teamNames;
    std::chrono::milliseconds _tickDuration;
    std::chrono::steady_clock::time_point _lastTickTime;
    int _resourceSpawnTicks;
    int _foodConsumptionTicks;
    std::size_t _nextPlayerId;
};
