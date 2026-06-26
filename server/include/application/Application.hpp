#pragma once

#include "core/Core.hpp"
#include "parser/Parser.hpp"

class Application {
public:
    Application() = default;
    int run(int argc, char **argv);

private:
    Parser _parser;
};
