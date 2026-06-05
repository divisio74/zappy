#include "network/SocketRegistry.hpp"

#include <filesystem>
#include <poll.h>
#include <stdexcept>

SocketRegistry::SocketRegistry(const std::string &directory)
    : _directory(directory)
{
    _fds.fill(-1);
    std::filesystem::create_directories(_directory);
}

void SocketRegistry::addSocket(std::size_t id, const std::string &path)
{
    if (_sockets.size() >= MaxSockets)
        throw std::runtime_error("maximum number of server sockets reached");
    if (_indexById.contains(id))
        throw std::runtime_error("server socket id already exists");

    _sockets.push_back({id, ServerSocket(path)});
    refreshFds();
}

void SocketRegistry::acceptConnection(std::size_t id)
{
    std::size_t index = getIndex(id);

    _sockets[index].socket.acceptConnection();
    refreshFds();
}

void SocketRegistry::removeSocket(std::size_t id)
{
    std::size_t index = getIndex(id);

    _sockets.erase(_sockets.begin() + index);
    refreshFds();
}

std::vector<std::size_t> SocketRegistry::pollSockets()
{
    std::array<pollfd, MaxSockets> pollFds{};
    std::vector<std::size_t> readyIds;

    for (std::size_t i = 0; i < MaxSockets; ++i) {
        pollFds[i].fd = _fds[i];
        pollFds[i].events = POLLIN;
        pollFds[i].revents = 0;
    }
    if (::poll(pollFds.data(), pollFds.size(), 0) == -1)
        throw std::runtime_error("poll failed");
    for (std::size_t i = 0; i < MaxSockets; ++i) {
        if (pollFds[i].fd == -1)
            continue;

        std::size_t id = _idByFd.at(pollFds[i].fd);

        if (pollFds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            markLostConnection(id);
            continue;
        }
        if (pollFds[i].revents & POLLIN)
            readyIds.push_back(id);
    }
    return readyIds;
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
