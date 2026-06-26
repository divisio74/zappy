#include "core/Core.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr int RESOURCE_SPAWN_INTERVAL = 2000;
constexpr int FOOD_CONSUMPTION_INTERVAL = 126;

std::string getCommandName(const std::string &command)
{
    std::istringstream stream(command);
    std::string name;

    stream >> name;
    return name;
}
}

Core::Core(const ServerConfig &config)
    : _worldHandling(_worldData),
      _socketRegistry("0.0.0.0", config.port, SocketRegistry::MaxSockets),
      _guiHandler(_worldData),
      _aiHandler(_worldData, [this](const GuiEvent &event) {
          queueGuiEvent(event);
      }),
      _teamNames(config.teamNames),
      _tickDuration(std::chrono::milliseconds(1000 / config.freq)),
      _lastTickTime(std::chrono::steady_clock::now()),
      _resourceSpawnTicks(0),
      _foodConsumptionTicks(0),
      _nextPlayerId(1)
{
    _worldData.freq = static_cast<int>(config.freq);
    _worldHandling.createMap(config.width, config.height, config.teamNames,
        config.clientsNb);
}

void Core::runServer()
{
    while (true)
        processServerTick();
}

void Core::processServerTick()
{
    std::vector<std::size_t> readySockets =
        _socketRegistry.pollSockets(getPollTimeoutMs());

    for (std::size_t socketId : readySockets)
        processClientCommands(socketId);
    for (std::size_t socketId : _socketRegistry.getNewConnections())
        greetNewClient(socketId);
    for (std::size_t socketId : _socketRegistry.getLostConnections())
        cleanupDisconnectedClient(socketId);

    processAiActions();
    processTimedWorldEvents();
    sendGuiMessages();
}

void Core::greetNewClient(std::size_t socketId)
{
    _socketRegistry.write(socketId, "WELCOME");
}

void Core::processClientCommands(std::size_t socketId)
{
    std::vector<std::string> commands = _socketRegistry.read(socketId);

    for (const std::string &command : commands) {
        switch (_socketRegistry.getSocketState(socketId)) {
        case SocketRegistry::SocketState::Unknown:
            identifyClient(socketId, command);
            break;
        case SocketRegistry::SocketState::Ai:
            processAiClientCommand(socketId, command);
            break;
        case SocketRegistry::SocketState::Gui:
            processGuiClientCommand(socketId, command);
            break;
        }
    }
}

void Core::identifyClient(std::size_t socketId, const std::string &command)
{
    if (command == "GRAPHIC") {
        _socketRegistry.setSocketState(socketId, SocketRegistry::SocketState::Gui);
        sendGuiInitialState(socketId);
        return;
    }

    std::optional<std::size_t> teamId = findTeamByName(command);

    if (!teamId.has_value()) {
        _socketRegistry.write(socketId, "ko");
        return;
    }

    std::optional<std::size_t> playerId = spawnPlayerFromTeamEgg(teamId.value());

    if (!playerId.has_value()) {
        _socketRegistry.write(socketId, "ko");
        return;
    }
    _socketRegistry.setSocketState(socketId, SocketRegistry::SocketState::Ai);
    _socketRegistry.assignBusinessId(socketId, playerId.value());
    sendAiConnectionData(socketId, playerId.value());
}

void Core::processAiClientCommand(std::size_t socketId,
    const std::string &command)
{
    std::optional<std::size_t> businessId = _socketRegistry.getBusinessId(socketId);

    if (!businessId.has_value())
        return;

    std::vector<std::string> commands = {command};

    _aiHandler.parseCommand(businessId.value(), commands);
}

void Core::processGuiClientCommand(std::size_t socketId,
    const std::string &command)
{
    std::vector<std::string> responses = _guiHandler.handleCommand(command);

    if (getCommandName(command) == "sst" && responses.size() == 1
        && getCommandName(responses[0]) == "sgt") {
        updateTickDuration();
    }
    for (const std::string &response : responses)
        _socketRegistry.write(socketId, response);
}

void Core::cleanupDisconnectedClient(std::size_t socketId)
{
    if (_socketRegistry.getSocketState(socketId) == SocketRegistry::SocketState::Ai) {
        std::optional<std::size_t> playerId = _socketRegistry.getBusinessId(socketId);

        if (playerId.has_value()) {
            auto player = _worldData.players.find(playerId.value());

            if (player != _worldData.players.end()) {
                auto team = _worldData.teams.find(player->second.getTeamId());

                queueGuiEvent({.type = GuiEventType::PlayerDeath,
                    .playerId = playerId.value()});
                _worldData.players.erase(player);
            }
        }
    }
    _socketRegistry.removeSocket(socketId);
}

void Core::processTimedWorldEvents()
{
    auto now = std::chrono::steady_clock::now();

    while (now - _lastTickTime >= _tickDuration) {
        _lastTickTime += _tickDuration;
        _resourceSpawnTicks++;
        _foodConsumptionTicks++;
        if (_foodConsumptionTicks >= FOOD_CONSUMPTION_INTERVAL) {
            processFoodConsumption();
            _foodConsumptionTicks = 0;
        }
        if (_resourceSpawnTicks >= RESOURCE_SPAWN_INTERVAL) {
            _worldHandling.spawnRessources();
            _resourceSpawnTicks = 0;
        }
    }
}

void Core::processFoodConsumption()
{
    std::vector<std::size_t> deadPlayers;

    for (auto &[id, player] : _worldData.players) {
        Inventory &inventory = player.getInventory();

        if (inventory.food > 0)
            inventory.food--;
        if (inventory.food <= 0)
            deadPlayers.push_back(id);
    }
    for (std::size_t playerId : deadPlayers)
        killPlayer(playerId);
}

void Core::killPlayer(std::size_t playerId)
{
    auto player = _worldData.players.find(playerId);

    if (player == _worldData.players.end())
        return;
    _aiHandler.addMessage(playerId, "dead");
    sendAiMessages();

    auto team = _worldData.teams.find(player->second.getTeamId());

    if (team != _worldData.teams.end() && team->second.getUsedSlot() > 0)
        team->second.setUsedSlot(team->second.getUsedSlot() - 1);
    queueGuiEvent({.type = GuiEventType::PlayerDeath,
        .playerId = playerId});
    _worldData.players.erase(player);
}

void Core::processAiActions()
{
    _aiHandler.run();
    sendAiMessages();
}

void Core::sendAiMessages()
{
    std::vector<message> messages = _aiHandler.getMessages();

    for (const message &aiMessage : messages) {
        std::optional<std::size_t> socketId =
            _socketRegistry.getSocketIdByBusinessId(aiMessage.playerId);

        if (socketId.has_value())
            _socketRegistry.write(socketId.value(), aiMessage.message);
    }
}

void Core::queueGuiEvent(const GuiEvent &event)
{
    _guiEvents.push(event);
}

void Core::sendGuiMessages()
{
    std::vector<std::size_t> guiSockets =
        _socketRegistry.getSocketIdsByState(SocketRegistry::SocketState::Gui);

    while (!_guiEvents.empty()) {
        std::vector<std::string> messages = formatGuiEvent(_guiEvents.front());

        _guiEvents.pop();
        for (std::size_t socketId : guiSockets) {
            for (const std::string &message : messages)
                _socketRegistry.write(socketId, message);
        }
    }
}

void Core::sendGuiInitialState(std::size_t socketId)
{
    _socketRegistry.write(socketId, _guiHandler.formatMapSize());
    _socketRegistry.write(socketId, _guiHandler.formatTimeUnit());
}

std::vector<std::string> Core::formatGuiEvent(const GuiEvent &event) const
{
    switch (event.type) {
    case GuiEventType::MapSize:
        return {_guiHandler.formatMapSize()};
    case GuiEventType::TileContent:
        return {_guiHandler.formatTileContent(event.x, event.y)};
    case GuiEventType::MapContent:
        return _guiHandler.formatMapContent();
    case GuiEventType::TeamNames:
        return _guiHandler.formatTeamNames();
    case GuiEventType::PlayerConnection: {
        auto player = _worldData.players.find(event.playerId);

        if (player == _worldData.players.end())
            return {};
        return {_guiHandler.formatPlayerConnection(player->second)};
    }
    case GuiEventType::PlayerPosition:
        return {_guiHandler.formatPlayerPosition(event.playerId)};
    case GuiEventType::PlayerLevel:
        return {_guiHandler.formatPlayerLevel(event.playerId)};
    case GuiEventType::PlayerInventory:
        return {_guiHandler.formatPlayerInventory(event.playerId)};
    case GuiEventType::Expulsion:
        return {_guiHandler.formatExpulsion(event.playerId)};
    case GuiEventType::Broadcast:
        return {_guiHandler.formatBroadcast(event.playerId, event.message)};
    case GuiEventType::IncantationStart:
        return {_guiHandler.formatIncantationStart(event.x, event.y,
            event.level, event.playerIds)};
    case GuiEventType::IncantationEnd:
        return {_guiHandler.formatIncantationEnd(event.x, event.y,
            event.result)};
    case GuiEventType::EggLaying:
        return {_guiHandler.formatEggLaying(event.playerId)};
    case GuiEventType::ResourceDrop:
        return {_guiHandler.formatResourceDrop(event.playerId, event.resource)};
    case GuiEventType::ResourceCollect:
        return {_guiHandler.formatResourceCollect(event.playerId, event.resource)};
    case GuiEventType::PlayerDeath:
        return {_guiHandler.formatPlayerDeath(event.playerId)};
    case GuiEventType::EggLaid:
        return {_guiHandler.formatEggLaid(event.eggId, event.playerId,
            event.x, event.y)};
    case GuiEventType::EggConnection:
        return {_guiHandler.formatEggConnection(event.eggId)};
    case GuiEventType::EggDeath:
        return {_guiHandler.formatEggDeath(event.eggId)};
    case GuiEventType::TimeUnit:
        return {_guiHandler.formatTimeUnit()};
    case GuiEventType::TimeUnitModification:
        return {_guiHandler.formatTimeUnitModification(event.timeUnit)};
    case GuiEventType::EndGame:
        return {_guiHandler.formatEndGame(event.teamName)};
    case GuiEventType::ServerMessage:
        return {_guiHandler.formatServerMessage(event.message)};
    case GuiEventType::UnknownCommand:
        return {"suc"};
    case GuiEventType::BadParameter:
        return {"sbp"};
    }
    return {};
}

void Core::sendAiConnectionData(std::size_t socketId, std::size_t playerId)
{
    auto player = _worldData.players.find(playerId);

    if (player == _worldData.players.end())
        return;

    auto team = _worldData.teams.find(player->second.getTeamId());

    if (team == _worldData.teams.end())
        return;

    std::size_t availableSlots = team->second.getMaxSlot() - team->second.getUsedSlot();

    _socketRegistry.write(socketId, std::to_string(availableSlots));
    _socketRegistry.write(socketId,
        std::to_string(_worldData.size.x) + " " + std::to_string(_worldData.size.y));
}

void Core::updateTickDuration()
{
    if (_worldData.freq <= 0)
        return;
    _tickDuration = std::chrono::milliseconds(std::max(1, 1000 / _worldData.freq));
}

std::optional<std::size_t> Core::findTeamByName(const std::string &teamName) const
{
    for (std::size_t i = 0; i < _teamNames.size(); ++i) {
        if (_teamNames[i] == teamName)
            return i;
    }
    return std::nullopt;
}

std::optional<std::size_t> Core::spawnPlayerFromTeamEgg(std::size_t teamId)
{
    auto teamIt = _worldData.teams.find(teamId);

    if (teamIt == _worldData.teams.end())
        return std::nullopt;

    Team &team = teamIt->second;

    if (team.getUsedSlot() >= team.getMaxSlot())
        return std::nullopt;

    auto eggIt = std::find_if(_worldData.eggs.begin(), _worldData.eggs.end(),
        [teamId](const Egg &egg) {
            return egg.getTeamId() == teamId;
        });

    if (eggIt == _worldData.eggs.end())
        return std::nullopt;

    Vector2 position = eggIt->getCoordinate();
    std::size_t eggId = eggIt->getId();
    std::size_t playerId = _nextPlayerId++;

    team.setUsedSlot(team.getUsedSlot() + 1);
    _worldData.players.emplace(playerId,
        Player(position, Direction::North, playerId, teamId));

    _worldData.eggs.erase(eggIt);
    queueGuiEvent({.type = GuiEventType::EggConnection,
        .eggId = eggId});
    queueGuiEvent({.type = GuiEventType::PlayerConnection,
        .playerId = playerId});
    queueGuiEvent({.type = GuiEventType::PlayerInventory,
        .playerId = playerId});

    // TODO: Keep Team/Tile player shared_ptr storage in sync with WorldData.players.
    return playerId;
}

int Core::getPollTimeoutMs() const
{
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - _lastTickTime);

    if (elapsed >= _tickDuration)
        return 0;
    return static_cast<int>((_tickDuration - elapsed).count());
}
