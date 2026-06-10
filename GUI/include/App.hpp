#pragma once

#include "Renderer.hpp"

class App {
    public:
        App();
        ~App() = default;

        void run();

    private:
        Renderer _renderer;
};