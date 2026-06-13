#pragma once

#define HEIGHT  5
#define WIDTH   5

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Camera.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"

class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        void init();
        void render();
        void renderMap(uint width, uint height);        // temporaire apres ca prendra les infos de gamestate
        void renderTile(float x, float z);
        std::unique_ptr<Mesh> initTile();

        const sf::VideoMode& getVideoMode() const;
        const sf::Window&    getWindow   () const;
        sf::Window&          getWindow   ();
        Camera&              getCamera   ();

    private:
        sf::VideoMode                   _videoMode;
        sf::ContextSettings             _settings;
        sf::Window                      _window;
        Camera                          _camera;
        std::unique_ptr<Shader>         _shader;
        std::unique_ptr<Mesh>           _tileMesh;
};