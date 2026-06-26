#include "parser/Parser.hpp"

#include <limits>
#include <stdexcept>

namespace {
const char *USAGE = "USAGE: ./zappy_server -p port -x width -y height -n name1 ... -c clientsNb -f freq";
const int MAX_FREQ = 1000;
const std::size_t MIN_MAP_SIZE = 10;
const std::size_t MAX_MAP_SIZE = 42;

bool isArgExist(int index, int argc)
{
    return index + 1 < argc;
}

bool isFlag(const std::string &arg)
{
    return !arg.empty() && arg[0] == '-';
}

bool isKnownOption(const std::string &arg)
{
    return arg == "-p" || arg == "-x" || arg == "-y" || arg == "-n"
        || arg == "-c" || arg == "-f";
}

std::string optionName(const std::string &option)
{
    if (option == "-p")
        return "port";
    if (option == "-x")
        return "map width";
    if (option == "-y")
        return "map height";
    if (option == "-c")
        return "clients number";
    if (option == "-f")
        return "frequency";
    return option;
}

void ensureValueExists(const std::string &option, int index, int argc, char **argv)
{
    if (!isArgExist(index, argc) || isKnownOption(argv[index + 1]))
        throw std::invalid_argument("Missing value after " + option);
}

int parseNumber(const std::string &value, const std::string &option)
{
    std::size_t position = 0;
    int number = 0;

    try {
        number = std::stoi(value, &position);
    } catch (const std::exception &) {
        throw std::invalid_argument("Invalid " + optionName(option) + ": '" + value
            + "' is not a number");
    }
    if (position != value.size())
        throw std::invalid_argument("Invalid " + optionName(option) + ": '" + value
            + "' is not a number");
    if (number < 0)
        throw std::invalid_argument("Invalid " + optionName(option)
            + ": value cannot be negative");
    return number;
}

void parseArgument(const std::string &arg, int argc, char **argv, int &index,
    ServerConfig &config)
{
    if (arg == "-p") {
        ensureValueExists(arg, index, argc, argv);
        int port = parseNumber(argv[++index], "-p");

        if (port == 0 || port > std::numeric_limits<std::uint16_t>::max())
            throw std::invalid_argument("Invalid port: value must be between 1 and 65535");
        config.port = static_cast<std::uint16_t>(port);
    } else if (arg == "-x") {
        ensureValueExists(arg, index, argc, argv);
        config.width = static_cast<std::size_t>(parseNumber(argv[++index], "-x"));
    } else if (arg == "-y") {
        ensureValueExists(arg, index, argc, argv);
        config.height = static_cast<std::size_t>(parseNumber(argv[++index], "-y"));
    } else if (arg == "-n") {
        ensureValueExists(arg, index, argc, argv);
        config.teamNames.clear();
        while (isArgExist(index, argc) && !isFlag(argv[index + 1]))
            config.teamNames.push_back(argv[++index]);
        if (config.teamNames.empty())
            throw std::invalid_argument("Invalid teams: -n needs at least one team name");
    } else if (arg == "-c") {
        ensureValueExists(arg, index, argc, argv);
        config.clientsNb = static_cast<std::size_t>(parseNumber(argv[++index], "-c"));
    } else if (arg == "-f") {
        ensureValueExists(arg, index, argc, argv);
        int freq = parseNumber(argv[++index], "-f");

        if (freq == 0 || freq > MAX_FREQ)
            throw std::invalid_argument("Invalid frequency: value must be between 1 and "
                + std::to_string(MAX_FREQ));
        config.freq = static_cast<std::size_t>(freq);
    } else {
        throw std::invalid_argument("Unknown option '" + arg + "'.\n" + USAGE);
    }
}

void validateTeamNames(const std::vector<std::string> &teamNames)
{
    if (teamNames.empty())
        throw std::invalid_argument("Invalid teams: at least one team is required");
    for (const std::string &teamName : teamNames) {
        if (teamName == "GRAPHIC")
            throw std::invalid_argument("Invalid team name: GRAPHIC is reserved for the GUI");
    }
}

void validateWorldSize(std::size_t width, std::size_t height)
{
    if (width < MIN_WORLD_SIZE || width > MAX_WORLD_SIZE
        || height < MIN_WORLD_SIZE || height > MAX_WORLD_SIZE)
        throw std::invalid_argument("Invalid map size: width and height must be between "
            + std::to_string(MIN_WORLD_SIZE) + " and "
            + std::to_string(MAX_WORLD_SIZE));
}

void validateClientsNumber(std::size_t clientsNb)
{
    if (clientsNb == 0)
        throw std::invalid_argument("Invalid clients number: value must be greater than 0");
}
}

ServerConfig Parser::parse(int argc, char **argv) const
{
    ServerConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg(argv[i]);
        parseArgument(arg, argc, argv, i, config);
    }
    if (argCount != REQUIRED_ARG_COUNT)
        throw std::invalid_argument(USAGE);
    return config;
}
