#include "network/ClientSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

sockaddr_un ClientSocket::makeAddress(const std::string &path)
{
    sockaddr_un address{};

    if (path.size() >= sizeof(address.sun_path))
        throw std::runtime_error("socket path is too long");
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);
    return address;
}

ClientSocket::ClientSocket(const std::string &path)
    : _ioSocket(::socket(AF_UNIX, SOCK_STREAM, 0))
{
    if (_ioSocket.getFd() == -1)
        throw std::runtime_error(std::strerror(errno));

    sockaddr_un address = ClientSocket::makeAddress(path);

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
