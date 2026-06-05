#pragma once

#include <cstddef>
#include <string>
#include <vector>

/**
 * Small framed I/O wrapper over a socket file descriptor.
 *
 * IOSocket exposes command-based reads and writes for the project protocol.
 * Public callers work with plain commands such as "STATUS" or "ORDER 1 2 3".
 * The "\r\n" network delimiter is handled internally.
 *
 * IOSocket does not own the file descriptor. The socket class that created the
 * fd is responsible for closing it.
 */
class IOSocket {
public:
    /**
     * Stores fd for framed reads and writes.
     * fd may be -1 for an empty socket object.
     */
    explicit IOSocket(int fd = -1);
    ~IOSocket() = default;

    IOSocket(const IOSocket &) = delete;
    IOSocket &operator=(const IOSocket &) = delete;
    IOSocket(IOSocket &&other);
    IOSocket &operator=(IOSocket &&other);

    /**
     * Reads available bytes and returns every complete command found.
     *
     * Commands are returned without the trailing "\r\n".
     * If no complete command is available yet, returns an empty vector and
     * keeps the partial data for the next read().
     *
     * A single read() can return several commands if several complete messages
     * were received at once.
     */
    std::vector<std::string> read(std::size_t size = 4096);

    /**
     * Writes one command to the socket.
     *
     * Callers should pass the command without "\r\n"; IOSocket appends it
     * automatically. Passing a command that already ends with "\r\n" is also
     * accepted and will not duplicate the delimiter.
     */
    void write(const std::string &message) const;

    /**
     * Returns the file descriptor, or -1 if none is currently set.
     */
    int getFd() const;

    /**
     * Replaces the file descriptor and clears any buffered partial input.
     */
    void setFd(int fd);

private:
    int _fd;
    std::string _buffer;
};
