#include "network/ClientSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

sockaddr_in ClientSocket::makeAddress(const std::string &host, std::uint16_t port)
{
    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1)
        throw std::runtime_error("invalid IPv4 address");
    return address;
}

ClientSocket::ClientSocket(std::uint16_t port)
    : ClientSocket("127.0.0.1", port)
{
}

ClientSocket::ClientSocket(const std::string &host, std::uint16_t port)
    : _ioSocket(::socket(AF_INET, SOCK_STREAM, 0))
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error(std::strerror(errno));

    sockaddr_in address{};

    try {
        address = ClientSocket::makeAddress(host, port);
    } catch (const std::runtime_error &) {
        closeSocket();
        throw;
    }

    if (::connect(_ioSocket.getFd(), reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
        closeSocket();
        throw std::runtime_error(std::strerror(errno));
    }
}

ClientSocket::ClientSocket(int fd)
    : _ioSocket(fd)
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error("invalid socket fd");
}

ClientSocket::~ClientSocket()
{
    closeSocket();
}

ClientSocket::ClientSocket(ClientSocket &&other)
    : _ioSocket(std::move(other._ioSocket))
{
}

ClientSocket &ClientSocket::operator=(ClientSocket &&other)
{
    if (this != &other) {
        closeSocket();
        _ioSocket = std::move(other._ioSocket);
    }
    return *this;
}

std::vector<std::string> ClientSocket::read(std::size_t size)
{
    return _ioSocket.read(size);
}

void ClientSocket::write(const std::string &message) const
{
    _ioSocket.write(message);
}

int ClientSocket::getFd() const
{
    return _ioSocket.getFd();
}

void ClientSocket::closeSocket()
{
    int fd = _ioSocket.getFd();

    if (fd != -1) {
        ::close(fd);
        _ioSocket.setFd(-1);
    }
}
