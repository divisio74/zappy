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
#include <optional>

#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "Cubemap.hpp"
#include "Tile.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "GameState.hpp"
#include "Hud.hpp"
#include "NetworkProtocol.hpp"
#include "interface/IRenderer.hpp"


class Renderer : public IRenderer {
    public:
        Renderer();
        ~Renderer() override;

        const sf::VideoMode&    getVideoMode() const override;
        const sf::RenderWindow& getWindow() const override;
        sf::RenderWindow&       getWindow() override;
        Camera&                 getCamera() override;

        void init() override;
        void shutdown() override;
        void render(GameState& state, float deltaTime) override;
        void toggleGroundOutline() override;
        void selectTileAt(GameState& state, sf::Vector2i mousePosition) override;

    private:
        void renderMap(std::vector<std::vector<Tile>>& map);
        void renderTile(Tile& tile, float x, float z);
        std::unique_ptr<Mesh> initTile();
        void initPlayersModels(GameState& state);
        void uptPlayerModel(int playerID, std::shared_ptr<Model> &playerModel, GameState& state);
        glm::vec3 getTileCenter (const Tile& tile);
        glm::vec3 getTileRandPos(const Tile& tile);
        glm::vec3 getTileRandPos(glm::vec3 tilePos);
        std::shared_ptr<Model> createPlayerModel(const Player& player, const GameState& state);
        std::shared_ptr<Model> getModel(const std::string &path, glm::vec3 scale = glm::vec3(1.f));

        void updatePlayers(GameState& state, float deltaTime);
        void renderEggs(GameState& state);
        void processResources(Tile& tile);
        void addTileResources(Tile& tile, int count, const std::string &modelPath, const glm::vec3 &scale);
        void cleanupPlayerModel(int playerId);
        void releaseResources();
        void deleteTexture(GLuint &texture);
        bool isSelectedTile(int x, int z) const;
        std::optional<glm::vec3> getGroundHitPoint(sf::Vector2i mousePosition) const;
        glm::mat4 getProjectionMatrix() const;
        std::string makeModelCacheKey(const std::string &path, const glm::vec3 &scale) const;

        sf::RenderWindow            _window;
        sf::VideoMode               _videoMode;
        Camera                      _camera;
        Cubemap                     _cubemap;
        Hud                         _hud;
        sf::ContextSettings         _settings;
        std::unique_ptr<Shader>     _shader;
        std::unique_ptr<Mesh>       _tileMesh;
        std::vector<GLuint>         _groundTextures;
        std::vector<GLuint>         _outlinedGroundTextures;
        bool                        _showOutlinedGround = false;
        bool                        _isShutdown = false;
        std::optional<sf::Vector2i> _selectedTile;
        std::map<int, glm::vec3>    _playerRenderPositions;
        std::map<int, glm::vec3>    _eggRenderPositions;
        std::map<int, std::string>  _playerAnimations;
        std::vector<Egg>            _eggsModels;
        std::map<int, float>        _playerAnimationTimers;
        std::map<int, std::shared_ptr<Model>> _playersModels;
        std::map<std::string, std::shared_ptr<Model>> _modelsCache;
};
