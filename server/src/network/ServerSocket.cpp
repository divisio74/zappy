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

sockaddr_in ServerSocket::makeAddress(std::uint16_t port)
{
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    return address;
}

ServerSocket::ServerSocket(std::uint16_t port, ConnectionType type)
    : _ioSocket(::socket(AF_INET, ServerSocket::toNativeType(type), 0)),
      _connected(false)
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error(std::strerror(errno));

    int reuse = 1;
    sockaddr_in address = ServerSocket::makeAddress(port);

    if (::setsockopt(_ioSocket.getFd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        closeSocket();
        throw std::runtime_error(std::strerror(errno));
    }
    if (::bind(_ioSocket.getFd(), reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
        closeSocket();
        throw std::runtime_error(std::strerror(errno));
    }
    if (::listen(_ioSocket.getFd(), 128) == -1) {
        closeSocket();
        throw std::runtime_error(std::strerror(errno));
    }
}

ServerSocket::~ServerSocket()
{
    closeSocket();
}

ServerSocket::ServerSocket(ServerSocket &&other)
    : _ioSocket(std::move(other._ioSocket)),
      _connected(std::exchange(other._connected, false))
{
}

ServerSocket &ServerSocket::operator=(ServerSocket &&other)
{
    if (this != &other) {
        closeSocket();
        _ioSocket = std::move(other._ioSocket);
        _connected = std::exchange(other._connected, false);
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
