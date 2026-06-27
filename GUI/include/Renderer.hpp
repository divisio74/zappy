#pragma once

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
#include <tuple>

#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "GameState.hpp"
#include "NetworkProtocol.hpp"


class Renderer {
    public:
        Renderer();
        ~Renderer();

        const sf::VideoMode&    getVideoMode    () const;
        const sf::Window&       getWindow       () const;
        sf::Window&             getWindow       ();
        Camera&                 getCamera       ();

        void init();
        void shutdown();
        void render(GameState& state, float deltaTime);
        void renderMap(std::vector<std::vector<Tile>>& map);
        void renderTile(Tile& tile, float x, float z);
        std::unique_ptr<Mesh> initTile();
        void initPlayersModels(GameState& state);
        void uptPlayerModel(int playerID, std::shared_ptr<Model> &playerModel, GameState& state);
        glm::vec3 getTileCenter (const Tile& tile);
        glm::vec3 getTileRandPos(const Tile& tile);
        glm::vec3 getTileRandPos(glm::vec3 tilePos);
        std::shared_ptr<Model> getModel(const std::string &path, glm::vec3 scale = glm::vec3(1.f));

        unsigned int loadCubemap(std::vector<std::string> faces);
        void initCubemap();
        std::map<int, std::shared_ptr<Model>> _playersModels; //TODO mettre en private et adapté partout dans le code

    private:
        void renderSkybox(const glm::mat4 &projection);
        void updatePlayers(GameState& state, float deltaTime);
        void renderEggs(GameState& state);
        void processResources(Tile& tile);
        void addTileResources(Tile& tile, int count, const std::string &modelPath, const glm::vec3 &scale);
        void cleanupPlayerModel(int playerId);
        void releaseResources();
        void deleteTexture(GLuint &texture);
        std::string makeModelCacheKey(const std::string &path, const glm::vec3 &scale) const;

        sf::VideoMode              _videoMode;
        Camera                     _camera;
        sf::ContextSettings        _settings;
        sf::Window                 _window;
        std::unique_ptr<Shader>    _shader;
        std::unique_ptr<Mesh>      _tileMesh;
        std::vector<GLuint>        _groundTextures;
        std::map<int, std::string> _playerAnimations;
        std::vector<Egg>           _eggsModels;
        std::map<int, float>       _playerAnimationTimers;
        std::map<std::string, std::shared_ptr<Model>> _modelsCache;
        
        std::unique_ptr<Shader>    _cubemapShader;
        unsigned int skyboxVAO = 0;
        unsigned int skyboxVBO = 0;
        unsigned int _cubemapTexture = 0;
};
