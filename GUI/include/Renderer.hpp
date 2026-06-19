#pragma once

#define HEIGHT     1
#define WIDTH      1
#define NB_PLAYERS 1
#define PLAYERS_MODEL {"Mineralsa.glb", "Astronaut.glb"}

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Model.hpp"


class Renderer {
    public:
        Renderer();
        ~Renderer() = default;

        void init();
        void render();
        void renderMap(uint width, uint height);        // temporaire apres ca prendra les infos de gamestate
        void renderTile(float x, float z);
        // void renderModel(const char * path);
        std::vector<std::unique_ptr<Model>> initPlayersModels(uint nbPlayers);
        std::unique_ptr<Mesh> initTile();

        const sf::VideoMode&                    getVideoMode    () const;
        const sf::Window&                       getWindow       () const;
        sf::Window&                             getWindow       ();
        Camera&                                 getCamera       ();
        std::vector<std::unique_ptr<Model>>&    getPlayersModels();
        glm::vec3 getTileCenter(int x, int z);
        glm::vec3 getTileRandPos(int x, int z);

    private:
        sf::VideoMode                           _videoMode;
        sf::ContextSettings                     _settings;
        sf::Window                              _window;
        Camera                                  _camera;
        std::unique_ptr<Shader>                 _shader;
        std::unique_ptr<Mesh>                   _tileMesh;
        std::vector<std::unique_ptr<Model>>     _playersModels;
};