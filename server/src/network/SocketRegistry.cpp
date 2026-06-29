#include "network/SocketRegistry.hpp"

#include <algorithm>
#include <poll.h>
#include <stdexcept>
#include <utility>

SocketRegistry::SocketRegistry()
    : _listener("0.0.0.0", 0),
      _maxClients(0),
      _nextSocketId(ListenerSocketId + 1)
{
}

SocketRegistry::SocketRegistry(const std::string &host, std::uint16_t port,
    std::size_t maxClients)
    : _listener(host, port),
      _maxClients(maxClients),
      _nextSocketId(ListenerSocketId + 1)
{
    if (_maxClients > MaxSockets)
        throw std::runtime_error("maximum number of client sockets reached");
}

void SocketRegistry::removeSocket(std::size_t socketId)
{
    std::size_t index = getIndex(socketId);

    _sockets.erase(_sockets.begin() + index);
    _newConnections.erase(std::remove(_newConnections.begin(), _newConnections.end(),
        socketId), _newConnections.end());
    _lostConnections.erase(std::remove(_lostConnections.begin(), _lostConnections.end(),
        socketId), _lostConnections.end());
    refreshIndexes();
}

bool SocketRegistry::hasSocket(std::size_t socketId) const
{
    return _indexById.find(socketId) != _indexById.end();
}

std::vector<std::size_t> SocketRegistry::pollSockets(int timeoutMs)
{
    std::vector<pollfd> pollFds;
    std::vector<std::size_t> readyIds;

    pollFds.reserve(_sockets.size() + 1);
    pollFds.push_back({_listener.getFd(), POLLIN, 0});
    for (const auto &entry : _sockets)
        pollFds.push_back({entry.socket.getFd(), POLLIN, 0});
    if (::poll(pollFds.data(), pollFds.size(), timeoutMs) == -1)
        throw std::runtime_error("poll failed");

    if (pollFds[0].revents & (POLLHUP | POLLERR | POLLNVAL))
        throw std::runtime_error("listener socket failed");
    if (pollFds[0].revents & POLLIN)
        acceptNewConnection();

    for (std::size_t i = 1; i < pollFds.size(); ++i) {
        std::size_t socketId = _idByFd.at(pollFds[i].fd);

        if (pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            markLostConnection(socketId);
            continue;
        }
        if (pollFds[i].revents & POLLIN)
            readyIds.push_back(socketId);
    }
    return readyIds;
}

std::vector<std::size_t> SocketRegistry::getNewConnections()
{
    std::vector<std::size_t> newConnections = _newConnections;

    _newConnections.clear();
    return newConnections;
}

void SocketRegistry::assignBusinessId(std::size_t socketId,
    std::size_t businessId)
{
    std::size_t index = getIndex(socketId);
    auto existingSocketId = _socketIdByBusinessId.find(businessId);

    if (existingSocketId != _socketIdByBusinessId.end()
        && existingSocketId->second != socketId)
        throw std::runtime_error("business socket id already exists");
    if (_sockets[index].businessId.has_value())
        _socketIdByBusinessId.erase(_sockets[index].businessId.value());
    _sockets[index].businessId = businessId;
    _socketIdByBusinessId[businessId] = socketId;
}

bool SocketRegistry::hasBusinessId(std::size_t socketId) const
{
    return getBusinessId(socketId).has_value();
}

std::optional<std::size_t> SocketRegistry::getBusinessId(std::size_t socketId) const
{
    std::size_t index = getIndex(socketId);

    return _sockets[index].businessId;
}

std::optional<std::size_t> SocketRegistry::getSocketIdByBusinessId(
    std::size_t businessId) const
{
    auto socketId = _socketIdByBusinessId.find(businessId);

    if (socketId == _socketIdByBusinessId.end())
        return std::nullopt;
    return socketId->second;
}

void SocketRegistry::setSocketState(std::size_t socketId, SocketState state)
{
    std::size_t index = getIndex(socketId);

    _sockets[index].state = state;
}

SocketRegistry::SocketState SocketRegistry::getSocketState(std::size_t socketId) const
{
    std::size_t index = getIndex(socketId);

    return _sockets[index].state;
}

std::vector<std::size_t> SocketRegistry::getSocketIdsByState(
    SocketRegistry::SocketState state) const
{
    std::vector<std::size_t> socketIds;

    for (const auto &entry : _sockets) {
        if (entry.state == state)
            socketIds.push_back(entry.socketId);
    }
    return socketIds;
}

std::vector<std::size_t> SocketRegistry::getLostConnections()
{
    std::vector<std::size_t> lostConnections = _lostConnections;

    _lostConnections.clear();
    return lostConnections;
}

std::vector<std::string> SocketRegistry::read(std::size_t socketId,
    std::size_t size)
{
    std::size_t index = getIndex(socketId);

    try {
        return _sockets[index].socket.read(size);
    } catch (const std::runtime_error &) {
        markLostConnection(socketId);
        return {};
    }
}

void SocketRegistry::write(std::size_t socketId, const std::string &message)
{
    std::size_t index = getIndex(socketId);

    try {
        _sockets[index].socket.write(message);
    } catch (const std::runtime_error &) {
        markLostConnection(socketId);
    }
}

std::size_t SocketRegistry::size() const
{
    return _sockets.size();
}

std::size_t SocketRegistry::getIndex(std::size_t socketId) const
{
    auto index = _indexById.find(socketId);

    if (index == _indexById.end())
        throw std::runtime_error("socket id is invalid");
    return index->second;
}

void SocketRegistry::acceptNewConnection()
{
    ClientSocket client = _listener.acceptConnection();

    if (_sockets.size() >= _maxClients)
        return;

    std::size_t socketId = _nextSocketId++;

    _sockets.push_back({socketId, SocketState::Unknown, std::nullopt,
        std::move(client)});
    _newConnections.push_back(socketId);
    refreshIndexes();
}

void SocketRegistry::markLostConnection(std::size_t socketId)
{
    for (std::size_t lostId : _lostConnections) {
        if (lostId == socketId)
            return;
    }
    _lostConnections.push_back(socketId);
}

void SocketRegistry::refreshIndexes()
{
    _indexById.clear();
    _socketIdByBusinessId.clear();
    _idByFd.clear();
    for (std::size_t i = 0; i < _sockets.size(); ++i) {
        const auto &entry = _sockets[i];
        int fd = entry.socket.getFd();

        _indexById[entry.socketId] = i;
        _idByFd[fd] = entry.socketId;
        if (entry.businessId.has_value())
            _socketIdByBusinessId[entry.businessId.value()] = entry.socketId;
    }
}
