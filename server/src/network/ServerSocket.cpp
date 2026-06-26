#include "network/ServerSocket.hpp"

#include <arpa/inet.h>
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

sockaddr_in ServerSocket::makeAddress(const std::string &host, std::uint16_t port)
{
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1)
        throw std::runtime_error("invalid IPv4 address");
    return address;
}

ServerSocket::ServerSocket(std::uint16_t port, ConnectionType type)
    : ServerSocket("0.0.0.0", port, type)
{
}

ServerSocket::ServerSocket(const std::string &host, std::uint16_t port,
    ConnectionType type)
    : _ioSocket(::socket(AF_INET, ServerSocket::toNativeType(type), 0))
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error(std::strerror(errno));

    int reuse = 1;
    sockaddr_in address{};

    try {
        address = ServerSocket::makeAddress(host, port);
    } catch (const std::runtime_error &) {
        closeSocket();
        throw;
    }
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
    : _ioSocket(std::move(other._ioSocket))
{
}

ServerSocket &ServerSocket::operator=(ServerSocket &&other)
{
    if (this != &other) {
        closeSocket();
        _ioSocket = std::move(other._ioSocket);
    }
    return *this;
}

ClientSocket ServerSocket::acceptConnection() const
{
    int clientFd = ::accept(_ioSocket.getFd(), nullptr, nullptr);

    if (clientFd == -1)
        throw std::runtime_error(std::strerror(errno));
    return ClientSocket(clientFd);
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
