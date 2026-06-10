#include "App.hpp"

App::App()
{
    _renderer.init();
}

void App::run()
{
    while (_renderer.getWindow().isOpen()) {
        while (const std::optional eventOptional = _renderer.getWindow().pollEvent()) {
            if (eventOptional->is<sf::Event::Closed>())
                _renderer.getWindow().close();
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        _renderer.render();
        _renderer.getWindow().display();
    }
}