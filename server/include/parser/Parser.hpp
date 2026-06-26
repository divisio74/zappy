#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct ServerConfig {
    std::uint16_t port = 4242;
    std::size_t width = 10;
    std::size_t height = 10;
    std::vector<std::string> teamNames = {"team1", "team2"};
    std::size_t clientsNb = 3;
    std::size_t freq = 100;
};

class Parser {
public:
    Parser() = default;

    ServerConfig parse(int argc, char **argv) const;
};
