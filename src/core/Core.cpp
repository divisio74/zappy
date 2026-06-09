#include "core/Core.hpp"

Core::Core()
    : _guiHandler(_worldData),
      _aiHandler(_worldData)
{
}

Core::Core(std::uint16_t port, std::size_t width, std::size_t height,
    const std::vector<std::string> &teamNames, std::size_t clientsNb,
    std::size_t frequency)
    : _guiHandler(_worldData),
      _aiHandler(_worldData)
{
    static_cast<void>(port);
    static_cast<void>(width);
    static_cast<void>(height);
    static_cast<void>(teamNames);
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
