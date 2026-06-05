#include "network/ServerSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

int ServerSocket::toNativeType(ServerSocket::ConnectionType type)
{
    switch (type) {
    case ServerSocket::ConnectionType::Reciprocal:
        return SOCK_STREAM;
    }
    return SOCK_STREAM;
}

sockaddr_un ServerSocket::makeAddress(const std::string &path)
{
    sockaddr_un address{};

    if (path.size() >= sizeof(address.sun_path))
        throw std::runtime_error("socket path is too long");
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);
    return address;
}

ServerSocket::ServerSocket(const std::string &path, ConnectionType type)
    : _ioSocket(::socket(AF_UNIX, ServerSocket::toNativeType(type), 0)),
      _path(path),
      _connected(false)
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error(std::strerror(errno));

    sockaddr_un address = ServerSocket::makeAddress(path);

    ::unlink(path.c_str());
    if (::bind(_ioSocket.getFd(), reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
        closeSocket();
        throw std::runtime_error(std::strerror(errno));
    }
    if (::listen(_ioSocket.getFd(), 128) == -1) {
        closeSocket();
        ::unlink(_path.c_str());
        throw std::runtime_error(std::strerror(errno));
    }
}

ServerSocket::~ServerSocket()
{
    closeSocket();
    if (!_path.empty())
        ::unlink(_path.c_str());
}

ServerSocket::ServerSocket(ServerSocket &&other)
    : _ioSocket(std::move(other._ioSocket)),
      _path(std::move(other._path)),
      _connected(std::exchange(other._connected, false))
{
    other._path.clear();
}

ServerSocket &ServerSocket::operator=(ServerSocket &&other)
{
    if (this != &other) {
        closeSocket();
        if (!_path.empty())
            ::unlink(_path.c_str());
        _ioSocket = std::move(other._ioSocket);
        _path = std::move(other._path);
        _connected = std::exchange(other._connected, false);
        other._path.clear();
    }
    return *this;
}

void ServerSocket::acceptConnection()
{
    int clientFd = ::accept(_ioSocket.getFd(), nullptr, nullptr);

    if (clientFd == -1)
        throw std::runtime_error(std::strerror(errno));
    closeSocket();
    _ioSocket.setFd(clientFd);
    _connected = true;
}

std::vector<std::string> ServerSocket::read(std::size_t size)
{
    if (!_connected)
        throw std::runtime_error("server socket is not connected");
    return _ioSocket.read(size);
}

void ServerSocket::write(const std::string &message) const
{
    if (!_connected)
        throw std::runtime_error("server socket is not connected");
    _ioSocket.write(message);
}

int ServerSocket::getFd() const
{
    return _ioSocket.getFd();
}

void ServerSocket::closeSocket()
{
    int fd = _ioSocket.getFd();

    if (fd != -1) {
        ::close(fd);
        _ioSocket.setFd(-1);
    }
}
