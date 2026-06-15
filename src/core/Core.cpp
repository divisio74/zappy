#include "core/Core.hpp"

Core::Core()
    : _worldData({}, 0, 0),
      _guiHandler(_worldData),
      _aiHandler(_worldData),
      _nextBusinessId(0)
{
}

Core::Core(std::uint16_t port, std::size_t width, std::size_t height,
           const std::vector<std::string> &teamNames, std::size_t clientsNb,
           std::size_t frequency)
    : _worldData(teamNames, width, height),
      _guiHandler(_worldData),
      _aiHandler(_worldData),
      _nextBusinessId(0)
{
    _socketRegistry.addServerSocket(port);
    static_cast<void>(clientsNb);
    static_cast<void>(frequency);
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

void Core::dispatche()
{
    std::vector<std::size_t> readySockets = _socketRegistry.pollSockets();

    acceptPendingConnection();
    removeLostConnections();
    assignSocketTypes();
    dispatchReadySockets(readySockets);
}

void Core::acceptPendingConnection()
{
    if (!_socketRegistry.hasPendingConnection())
        return;

    std::size_t socketId = _socketRegistry.acceptConnection();

    _socketRegistry.write(socketId, "WELCOME");
}

void Core::removeLostConnections()
{
    _socketRegistry.getLostConnections();
}

void Core::assignSocketTypes()
{
    for (std::size_t socketId :
         _socketRegistry.getPendingIdentificationSockets()) {
        std::vector<std::string> commands = _socketRegistry.read(socketId);

        if (commands.empty())
            continue;

        assignSocketType(socketId, commands.front());
    }
}

void Core::assignSocketType(std::size_t socketId, const std::string &command)
{
    if (command == "GUI")
        _socketRegistry.setSocketIdentity(
            socketId, SocketRegistry::SocketState::Gui, createBusinessId());
    else if (command.starts_with("IA"))
        _socketRegistry.setSocketIdentity(
            socketId, SocketRegistry::SocketState::Ai, createBusinessId());
}

void Core::dispatchReadySockets(const std::vector<std::size_t> &readySockets)
{
    for (std::size_t id : readySockets) {
        std::vector<std::string> commands = _socketRegistry.read(id);

        if (commands.empty())
            continue;
    }
}

std::size_t Core::createBusinessId()
{
    return _nextBusinessId++;
}
