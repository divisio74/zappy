#include "../include/network/IOSocket.hpp"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

IOSocket::IOSocket(int fd)
    : _fd(fd)
{
}

IOSocket::IOSocket(IOSocket &&other)
    : _fd(other._fd),
      _buffer(std::move(other._buffer))
{
    other._fd = -1;
}

IOSocket &IOSocket::operator=(IOSocket &&other)
{
    if (this != &other) {
        _fd = other._fd;
        _buffer = std::move(other._buffer);
        other._fd = -1;
    }
    return *this;
}

std::vector<std::string> IOSocket::read(std::size_t size)
{
    std::vector<std::string> commands;
    std::string buffer(size, '\0');
    std::string currentOperation;
    ssize_t bytesRead = ::read(_fd, buffer.data(), buffer.size());

    if (bytesRead == -1)
        throw std::runtime_error(std::strerror(errno));
    if (bytesRead == 0)
        throw std::runtime_error("connection closed");
    buffer.resize(bytesRead);
    currentOperation = _buffer;

    for (std::size_t i = 0; i < buffer.size(); i++) {
        if (buffer[i] == '\n') {
            if (!currentOperation.empty() && currentOperation.back() == '\r')
                currentOperation.pop_back();
            commands.push_back(currentOperation);
            currentOperation.clear();
            continue;
        }
        if (currentOperation.size() + 1 < 1024)
            currentOperation += buffer[i];
    }
    _buffer = currentOperation;
    return commands;
}

void IOSocket::write(const std::string &message) const
{
    std::size_t total = 0;
    std::string command = message;

    if (command.empty() || command.back() != '\n')
        command += "\n";
    while (total < command.size()) {
        ssize_t bytesWritten = ::send(_fd, command.data() + total, command.size() - total, MSG_NOSIGNAL);

        if (bytesWritten == -1)
            throw std::runtime_error(std::strerror(errno));
        total += bytesWritten;
    }
}

int IOSocket::getFd() const
{
    return _fd;
}

void IOSocket::setFd(int fd)
{
    _fd = fd;
    _buffer.clear();
}