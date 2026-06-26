#include "parser/Parser.hpp"

#include <limits>
#include <stdexcept>

namespace {
const char *USAGE = "USAGE: ./zappy_server -p port -x width -y height -n name1 ... -c clientsNb -f freq";
const int REQUIRED_ARG_COUNT = 6;
const int MAX_FREQ = 1000;

bool isArgExist(int index, int argc)
{
    return index + 1 < argc;
}

bool isFlag(const std::string &arg)
{
    return !arg.empty() && arg[0] == '-';
}

int parseNumber(const std::string &value, const std::string &option)
{
    std::size_t position = 0;
    int number = 0;

    try {
        number = std::stoi(value, &position);
    } catch (const std::exception &) {
        throw std::invalid_argument("Invalid value for " + option);
    }
    if (position != value.size() || number < 0)
        throw std::invalid_argument("Invalid value for " + option);
    return number;
}

void parseArgument(const std::string &arg, int argc, char **argv, int &index,
    ServerConfig &config, int &argCount)
{
    if (arg == "-p" && isArgExist(index, argc)) {
        int port = parseNumber(argv[++index], "-p");

        if (port > std::numeric_limits<std::uint16_t>::max())
            throw std::invalid_argument("Invalid value for -p");
        config.port = static_cast<std::uint16_t>(port);
        argCount++;
    } else if (arg == "-x" && isArgExist(index, argc)) {
        config.width = static_cast<std::size_t>(parseNumber(argv[++index], "-x"));
        argCount++;
    } else if (arg == "-y" && isArgExist(index, argc)) {
        config.height = static_cast<std::size_t>(parseNumber(argv[++index], "-y"));
        argCount++;
    } else if (arg == "-n" && isArgExist(index, argc)) {
        while (isArgExist(index, argc) && !isFlag(argv[index + 1]))
            config.teamNames.push_back(argv[++index]);
        if (config.teamNames.empty())
            throw std::invalid_argument(USAGE);
        argCount++;
    } else if (arg == "-c" && isArgExist(index, argc)) {
        config.clientsNb = static_cast<std::size_t>(parseNumber(argv[++index], "-c"));
        argCount++;
    } else if (arg == "-f" && isArgExist(index, argc)) {
        int freq = parseNumber(argv[++index], "-f");

        if (freq == 0 || freq > MAX_FREQ)
            throw std::invalid_argument("Invalid value for -f");
        config.freq = static_cast<std::size_t>(freq);
        argCount++;
    } else {
        throw std::invalid_argument(USAGE);
    }
}
}

ServerConfig Parser::parse(int argc, char **argv) const
{
    ServerConfig config;
    int argCount = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        parseArgument(arg, argc, argv, i, config, argCount);
    }
    if (argCount != REQUIRED_ARG_COUNT)
        throw std::invalid_argument(USAGE);
    return config;
}
