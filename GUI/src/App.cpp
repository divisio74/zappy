#include "App.hpp"

App::App()
{
    _renderer.init();
}

void App::run()
{
    sf::Clock move_clock;
    float x = 0.f;
    float y = 0.f;

    while (_renderer.getWindow().isOpen()) {
        while (const std::optional eventOptional = _renderer.getWindow().pollEvent()) {
            if (eventOptional->is<sf::Event::Closed>())
                _renderer.getWindow().close();
        }

        float deltaTime = _clock.restart().asSeconds();
        _renderer.getCamera().processInput(deltaTime);
        _renderer.getCamera().processMouse(_renderer.getWindow());


        for (auto& model : _renderer.getPlayersModels()) {
            model->updateAnimation(deltaTime, "walk");
        
            if (move_clock.getElapsedTime().asMicroseconds() >= 5) {
                x += 0.005f;
                y += 0.005f;

                model->moveTo(glm::vec3(x, 0.f, y));
                move_clock.restart();
            }
        }

        glClearColor(0.1f, 0.1f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        _renderer.render();
        _renderer.getWindow().display();
    }
}

// encapsuler libs graphiques