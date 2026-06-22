#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct ServerConfig {
    std::uint16_t port = 0;
    std::size_t width = 0;
    std::size_t height = 0;
    std::vector<std::string> teamNames;
    std::size_t clientsNb = 0;
    std::size_t freq = 0;
};

class Parser {
public:
    Parser() = default;

    ServerConfig parse(int argc, char **argv) const;
};
