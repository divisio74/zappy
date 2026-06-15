#include "network/SocketRegistry.hpp"

#include <poll.h>
#include <stdexcept>

SocketRegistry::SocketRegistry()
    : _nextSocketId(0),
      _hasPendingConnection(false)
{
    _fds.fill(-1);
}

void SocketRegistry::addServerSocket(std::uint16_t port)
{
    if (_serverSocket.has_value())
        throw std::runtime_error("server socket already exists");

    _serverSocket.emplace(port);
}

bool SocketRegistry::hasPendingConnection() const
{
    return _hasPendingConnection;
}

std::size_t SocketRegistry::acceptConnection()
{
    if (!_serverSocket.has_value())
        throw std::runtime_error("server socket is not listening");
    if (_sockets.size() >= MaxSockets)
        throw std::runtime_error("maximum number of client sockets reached");

    std::size_t id = _nextSocketId++;

    _sockets.push_back(
        {id, _serverSocket->acceptConnection(), SocketState::Undefined,
         std::nullopt});
    _hasPendingConnection = false;
    refreshFds();
    return id;
}

void SocketRegistry::removeSocket(std::size_t id)
{
    std::size_t index = getIndex(id);

    _sockets.erase(_sockets.begin() + index);
    refreshFds();
}

std::vector<std::size_t> SocketRegistry::pollSockets()
{
    std::vector<pollfd> pollFds;
    std::vector<std::size_t> readyIds;
    std::size_t clientOffset = 0;

    _hasPendingConnection = false;
    _pendingIdentifications.clear();
    if (_serverSocket.has_value()) {
        pollFds.push_back({_serverSocket->getFd(), POLLIN, 0});
        clientOffset = 1;
    }
    for (std::size_t i = 0; i < _sockets.size(); ++i)
        pollFds.push_back({_fds[i], POLLIN, 0});
    if (pollFds.empty())
        return readyIds;
    if (::poll(pollFds.data(), pollFds.size(), 0) == -1)
        throw std::runtime_error("poll failed");
    if (_serverSocket.has_value() && pollFds[0].fd != -1
        && (pollFds[0].revents & POLLIN))
        _hasPendingConnection = true;
    for (std::size_t i = clientOffset; i < pollFds.size(); ++i) {
        if (pollFds[i].fd == -1)
            continue;

        std::size_t id = _idByFd.at(pollFds[i].fd);

        if (pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            markLostConnection(id);
            continue;
        }
        if (!(pollFds[i].revents & POLLIN))
            continue;
        if (getSocketState(id) == SocketState::Undefined) {
            _pendingIdentifications.push_back(id);
            continue;
        }
        readyIds.push_back(id);
    }
    return readyIds;
}

std::vector<std::size_t> SocketRegistry::getPendingIdentificationSockets()
{
    std::vector<std::size_t> pendingIdentifications = _pendingIdentifications;

    _pendingIdentifications.clear();
    return pendingIdentifications;
}

std::vector<std::size_t> SocketRegistry::getLostConnections()
{
    std::vector<std::size_t> lostConnections = _lostConnections;

    for (std::size_t id : lostConnections) {
        auto index = _indexById.find(id);

        if (index != _indexById.end()) {
            _sockets.erase(_sockets.begin() + index->second);
            refreshFds();
        }
    }
    _lostConnections.clear();
    refreshFds();
    return lostConnections;
}

std::vector<std::string> SocketRegistry::read(std::size_t id, std::size_t size)
{
    std::size_t index = getIndex(id);

    try {
        return _sockets[index].socket.read(size);
    } catch (const std::runtime_error &) {
        markLostConnection(id);
        return {};
    }
}

void SocketRegistry::write(std::size_t id, const std::string &message)
{
    std::size_t index = getIndex(id);

    try {
        _sockets[index].socket.write(message);
    } catch (const std::runtime_error &) {
        markLostConnection(id);
    }
}

SocketRegistry::SocketState SocketRegistry::getSocketState(std::size_t id) const
{
    return _sockets[getIndex(id)].state;
}

std::optional<std::size_t> SocketRegistry::getBusinessId(std::size_t id) const
{
    return _sockets[getIndex(id)].businessId;
}

void SocketRegistry::setSocketIdentity(std::size_t id, SocketState state,
                                       std::size_t businessId)
{
    SocketEntry &entry = _sockets[getIndex(id)];

    entry.state = state;
    entry.businessId = businessId;
}

std::size_t SocketRegistry::size() const
{
    return _sockets.size();
}

std::size_t SocketRegistry::getIndex(std::size_t id) const
{
    auto index = _indexById.find(id);

    if (index == _indexById.end())
        throw std::runtime_error("server socket id is invalid");
    return index->second;
}

void SocketRegistry::markLostConnection(std::size_t id)
{
    for (std::size_t lostId : _lostConnections) {
        if (lostId == id)
            return;
    }
    _lostConnections.push_back(id);
}

void SocketRegistry::refreshFds()
{
    _fds.fill(-1);
    _indexById.clear();
    _idByFd.clear();
    for (std::size_t i = 0; i < _sockets.size() && i < MaxSockets; ++i) {
        int fd = _sockets[i].socket.getFd();

        _fds[i] = fd;
        _indexById[_sockets[i].id] = i;
        if (fd != -1)
            _idByFd[fd] = _sockets[i].id;
    }
}
