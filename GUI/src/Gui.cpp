#include "Gui.hpp"

Gui::Gui()
{
    _renderer.init();
}

void Gui::run()
{
    while (const std::optional eventOptional = _renderer.getWindow().pollEvent()) {
        if (eventOptional->is<sf::Event::Closed>())
            _renderer.getWindow().close();
    }

    float deltaTime = _clock.restart().asSeconds();
    _renderer.getCamera().processInput(deltaTime);
    _renderer.getCamera().processMouse(_renderer.getWindow());

    glClearColor(0.1f, 0.1f, 0.15f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _renderer.render(_state);
    _renderer.getWindow().display();
}

Renderer &Gui::getRenderer()
{
    return _renderer;
}

GameState &Gui::getState()
{
    return _state;
}

NetworkProtocol &Gui::getNetwork()
{
    return _network;
}

// encapsuler libs graphiques
