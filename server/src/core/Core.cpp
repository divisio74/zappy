#include "core/Core.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

Core::Core()
    : _worldHandling(_worldData),
      _socketRegistry("0.0.0.0", 0, 0),
      _guiHandler(_worldData),
      _aiHandler(_worldData),
      _nextPlayerId(1)
{
}

Core::Core(const ServerConfig &config)
    : _worldHandling(_worldData),
      _socketRegistry("0.0.0.0", config.port, config.clientsNb),
      _guiHandler(_worldData),
      _aiHandler(_worldData),
      _teamNames(config.teamNames),
      _nextPlayerId(1)
{
    _worldData.freq = config.freq;
    _worldHandling.createMap(config.width, config.height, config.teamNames,
        config.clientsNb);
}

void Core::runServer()
{
    while (true) {
        processServerTick();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Core::processServerTick()
{
    for (std::size_t socketId : _socketRegistry.pollSockets())
        processClientCommands(socketId);
    for (std::size_t socketId : _socketRegistry.getNewConnections())
        greetNewClient(socketId);
    for (std::size_t socketId : _socketRegistry.getLostConnections())
        cleanupDisconnectedClient(socketId);

    // TODO: Run timed world events and flush handler responses to sockets.
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
        std::cout << "placeholder: call graphical fetch" << std::endl;
        // TODO: Register this socket as the GUI client and send GUI bootstrap data.
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
    std::cout << "placeholder: call AI connect for player "
              << playerId.value() << std::endl;
    // TODO: Send the Zappy AI connection response: remaining slots and map size.
}

void Core::processAiClientCommand(std::size_t socketId,
    const std::string &command)
{
    std::optional<std::size_t> businessId = _socketRegistry.getBusinessId(socketId);

    if (!businessId.has_value())
        return;

    static_cast<void>(command);
    std::cout << "placeholder: call AI handler for player "
              << businessId.value() << std::endl;
    _socketRegistry.write(socketId, "ko");
    // TODO: Call _aiHandler.parseCommand(playerId, commands) after the socket
    // has been attached to a real Player in _worldData.players.
}

void Core::processGuiClientCommand(std::size_t socketId,
    const std::string &command)
{
    static_cast<void>(command);
    std::cout << "placeholder: call GUI handler" << std::endl;
    _socketRegistry.write(socketId, "ko");
    // TODO: Dispatch GUI protocol commands to GuiHandler.
}

void Core::cleanupDisconnectedClient(std::size_t socketId)
{
    static_cast<void>(socketId);
    // TODO: Remove the matching GUI/player state once socketId -> client kind is known.
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
        [teamId](const std::shared_ptr<Egg> &egg) {
            return egg != nullptr && egg->getTeamId() == teamId;
        });

    if (eggIt == _worldData.eggs.end())
        return std::nullopt;

    std::shared_ptr<Egg> egg = *eggIt;
    Vector2 position = egg->getCoordinate();
    std::size_t playerId = _nextPlayerId++;

    team.setUsedSlot(team.getUsedSlot() + 1);
    _worldData.players.emplace(playerId,
        Player(position, Direction::North, playerId, teamId));

    auto removeEgg = [&egg](std::vector<std::shared_ptr<Egg>> &eggs) {
        eggs.erase(std::remove(eggs.begin(), eggs.end(), egg), eggs.end());
    };

    removeEgg(_worldData.eggs);
    if (position.y >= 0 && position.y < static_cast<int>(_worldData.map.size())
        && position.x >= 0
        && position.x < static_cast<int>(_worldData.map[position.y].size()))
        removeEgg(_worldData.map[position.y][position.x].getEggs());

    // TODO: Keep Team/Tile player shared_ptr storage in sync with WorldData.players.
    return playerId;
}

WorldData &Core::getWorldData()
{
    return _worldData;
}

const WorldData &Core::getWorldData() const
{
    return _worldData;
}

SocketRegistry &Core::getSocketRegistry()
{
    return _socketRegistry;
}

const SocketRegistry &Core::getSocketRegistry() const
{
    return _socketRegistry;
}

GuiHandler &Core::getGuiHandler()
{
    return _guiHandler;
}

AiHandler &Core::getAiHandler()
{
    return _aiHandler;
}

WorldHandling &Core::getWorldHandling()
{
    return _worldHandling;
}
