#pragma once

#include "core/Core.hpp"
#include "parser/Parser.hpp"

class Application {
public:
    Application() = default;
    int run();

private:
    Parser _parser;
    Core _core;
};
