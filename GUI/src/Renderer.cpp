#include "Renderer.hpp"
#include <cmath>

static glm::vec3 getPlayerRotation(int orientation)
{
    switch (orientation) {
        case 1:
            return glm::vec3(0.f, 180.f, 0.f);
        case 2:
            return glm::vec3(0.f, 90.f, 0.f);
        case 3:
            return glm::vec3(0.f, 0.f, 0.f);
        case 4:
            return glm::vec3(0.f, -90.f, 0.f);
        default:
            return glm::vec3(0.f, 180.f, 0.f);
    }
}

static float getAnimationDuration(const std::string &animation)
{
    if (animation == "Death")
        return 1.4f;
    if (animation == "Dance")
        return 2.5f;
    if (animation == "kick")
        return 0.7f;
    if (animation == "pickup")
        return 0.8f;
    if (animation == "walk")
        return 0.35f;
    return 0.f;
}

static std::string getTeamModel(const Player& player, const GameState& state)
{
    const auto& teams = state.getTeams();

    if (teams.size() > 4)
        return "model/FrogMech.glb";

    auto it = std::find(teams.begin(), teams.end(), player._teamName);

    if (it == teams.end())
        return "model/Astronaut.glb";

    int teamNum = static_cast<int>(std::distance(teams.begin(), it));

    switch (teamNum)
    {
        case 0: return "model/FrogMech.glb";
        case 1: return "model/BeeMech.glb";
        case 2: return "model/FlamMech.glb";
        case 3: return "model/FoxMech.glb";
        default:
            return "model/FrogMech.glb";
    }
}

static Vertex makeTileVertex(glm::vec3 position, glm::vec2 texCoords)
{
    return {position, glm::vec3(0.f), texCoords, glm::ivec4(0), glm::vec4(0.f)};
}

Renderer::Renderer()
{
    _videoMode.size.x  = 1920;
    _videoMode.size.y  = 1080;

    _settings.depthBits      = 24;
    _settings.majorVersion   = 3;
    _settings.minorVersion   = 3;

    _window = sf::RenderWindow(_videoMode, "Zappy", sf::State::Windowed, _settings);
    _window.setMouseCursorVisible(false);
    _window.setFramerateLimit(60);
}

Renderer::~Renderer()
{
    shutdown();
}

void Renderer::shutdown()
{
    if (_isShutdown)
        return;
    _isShutdown = true;
    if (_window.isOpen() && !_window.setActive(true))
        std::cerr << "Failed to activate OpenGL context before shutdown" << std::endl;
    releaseResources();
    if (_window.isOpen())
        _window.close();
}

void Renderer::releaseResources()
{
    _playersModels.clear();
    _playerRenderPositions.clear();
    _eggRenderPositions.clear();
    _playerAnimations.clear();
    _playerAnimationTimers.clear();
    _modelsCache.clear();
    if (_tileMesh)
        _tileMesh.reset();
    _shader.reset();
    _cubemap.release();
    _hud.release();

    for (GLuint &texture : _groundTextures)
        deleteTexture(texture);
    std::vector<GLuint>().swap(_groundTextures);

    for (GLuint &texture : _outlinedGroundTextures)
        deleteTexture(texture);
    std::vector<GLuint>().swap(_outlinedGroundTextures);

}

void Renderer::deleteTexture(GLuint &texture)
{
    if (texture == 0)
        return;
    glDeleteTextures(1, &texture);
    texture = 0;
}

void Renderer::init()
{
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    srand(time(0));

    _hud.init();

    _shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    _cubemap.init();

    _groundTextures.push_back(TextureLoader::load("ground_1.jpg"));
    for (int i = 0; i < 3; i++)
        _groundTextures.push_back(TextureLoader::load("ground_2.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_3.jpg"));

    _outlinedGroundTextures.push_back(TextureLoader::load("outlined_ground_1.jpg"));
    for (int i = 0; i < 3; i++)
        _outlinedGroundTextures.push_back(TextureLoader::load("outlined_ground_2.jpg"));
    _outlinedGroundTextures.push_back(TextureLoader::load("outlined_ground_3.jpg"));

    _tileMesh = initTile();

}

void Renderer::toggleGroundOutline()
{
    _showOutlinedGround = !_showOutlinedGround;
}

void Renderer::selectTileAt(GameState& state, sf::Vector2i mousePosition)
{
    if (state.getWidth() <= 0 || state.getHeight() <= 0) {
        _selectedTile.reset();
        return;
    }

    const std::optional<glm::vec3> hit = getGroundHitPoint(mousePosition);
    if (!hit) {
        _selectedTile.reset();
        return;
    }

    const int tileX = static_cast<int>(std::floor(hit->x));
    const int tileZ = static_cast<int>(std::floor(hit->z));

    if (tileX < 0 || tileX >= state.getWidth() || tileZ < 0 || tileZ >= state.getHeight()) {
        _selectedTile.reset();
        return;
    }
    _selectedTile = sf::Vector2i(tileX, tileZ);
}

void Renderer::render(GameState& state, float deltaTime)
{
    glm::mat4 view = _camera.getView();
    glm::mat4 projection = getProjectionMatrix();
    std::vector<std::vector<Tile>>& map = state.getMap();

    _cubemap.render(view, projection);

    _shader->use();
    _shader->setMat4("view", view);
    _shader->setMat4("projection", projection);

    updatePlayers(state, deltaTime);
    renderEggs(state);
    renderMap(map);
    _hud.render(_window, state, _selectedTile);
}

void Renderer::updatePlayers(GameState& state, float deltaTime)
{
    for (int playerId : state.getPlayersToUpdate()) {
        auto it = _playersModels.find(playerId);

        if (it != _playersModels.end()) {
            uptPlayerModel(it->first, it->second, state);
        } else {
            Player *player = state.findPlayer(playerId);

            if (player != nullptr) {
                _playersModels.insert({playerId, createPlayerModel(*player, state)});
            }
        }
    }

    for (auto it = _playersModels.begin(); it != _playersModels.end();) {
        int playerId = it->first;
        std::shared_ptr<Model> &playerModel = it->second;
        Player *player = state.findPlayer(playerId);
        std::string animation = "idle";

        if (player == nullptr) {
            it = _playersModels.erase(it);
            _playerRenderPositions.erase(playerId);
            _playerAnimations.erase(playerId);
            _playerAnimationTimers.erase(playerId);
            continue;
        }
        if (!player->_isAlive && _playerAnimationTimers[playerId] <= 0.f) {
            it = _playersModels.erase(it);
            _playerRenderPositions.erase(playerId);
            _playerAnimations.erase(playerId);
            _playerAnimationTimers.erase(playerId);
            continue;
        }
        if (_playerAnimationTimers[playerId] > 0.f) {
            animation = _playerAnimations[playerId];
            _playerAnimationTimers[playerId] -= deltaTime;
        } else {
            _playerAnimations[playerId] = "idle";
            _playerAnimationTimers[playerId] = 0.f;
        }
        glm::vec3 tilePos(player->_x, 0.f, player->_y);
        auto posIt = _playerRenderPositions.find(playerId);
        if (posIt == _playerRenderPositions.end()
            || static_cast<int>(std::floor(posIt->second.x)) != player->_x
            || static_cast<int>(std::floor(posIt->second.z)) != player->_y) {
            _playerRenderPositions[playerId] = getTileRandPos(tilePos);
        }
        playerModel->moveTo(
            _playerRenderPositions[playerId],
            getPlayerRotation(player->_orientation)
        );
        playerModel->updateAnimation(deltaTime, animation, animation != "Death");
        playerModel->Draw(*_shader);
        ++it;
    }

    state.clearPlayersToUpdate();
}

void Renderer::renderEggs(GameState& state)
{
    for (auto& egg : state.getEggs()) {
        if (!egg._isAlive) {
            _eggRenderPositions.erase(egg._id);
            continue;
        }
        auto posIt = _eggRenderPositions.find(egg._id);
        if (posIt == _eggRenderPositions.end()
            || static_cast<int>(std::floor(posIt->second.x)) != egg._x
            || static_cast<int>(std::floor(posIt->second.z)) != egg._y) {
            _eggRenderPositions[egg._id] = getTileRandPos(glm::vec3(egg._x, 0.2f, egg._y));
        }
        glm::vec3 eggPos = _eggRenderPositions[egg._id];
        glm::mat4 model = glm::translate(glm::mat4(1.f), eggPos);
        _shader->setMat4("model", model);
        if(!egg._model)
            egg._model = getModel("model/Egg.glb", glm::vec3(0.5f));
        egg._model->Draw(*_shader, eggPos);
    }

    for (auto it = _eggRenderPositions.begin(); it != _eggRenderPositions.end();) {
        if (state.findEgg(it->first) == nullptr)
            it = _eggRenderPositions.erase(it);
        else
            ++it;
    }
}

void Renderer::renderMap(std::vector<std::vector<Tile>>& map)
{
    for (std::size_t z = 0; z < map.size(); z++) {
        for (std::size_t x = 0; x < map[z].size(); x++)
            renderTile(map[z][x], static_cast<float>(x), static_cast<float>(z));
    }
}

void Renderer::renderTile(Tile& tile, float x, float z)
{
    tile.x = x;
    tile.z = z;

    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z));
    _shader->setMat4("model", model);
    if (tile.textureIndex == -1)
        tile.textureIndex = rand() % 5;

    const bool outlined = _showOutlinedGround || isSelectedTile(static_cast<int>(x), static_cast<int>(z));
    const std::vector<GLuint> &textures = outlined ? _outlinedGroundTextures : _groundTextures;

    _tileMesh->textures[0].id = textures[tile.textureIndex];
    _tileMesh->Draw(*_shader);

    processResources(tile);

    for (auto& resource : tile._resources) {
        glm::mat4 model = glm::translate(glm::mat4(1.f), resource._pos);
        _shader->setMat4("model", model);
        resource._model->Draw(*_shader, resource._pos);
    }
}

void Renderer::processResources(Tile& tile)
{
    if (!tile._resources.empty())
        return;
    tile._resources.reserve(tile.linemate + tile.deraumere + tile.mendiane
        + tile.phiras + tile.sibur + tile.thystame + tile.food);
    addTileResources(tile, tile.linemate, "model/Linemate.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.deraumere, "model/Deraumere.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.mendiane, "model/Mendiane.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.phiras, "model/Phiras.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.sibur, "model/Sibur.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.thystame, "model/Thystame.glb", glm::vec3(0.15f));
    addTileResources(tile, tile.food, "model/Food.glb", glm::vec3(200.f));
}

void Renderer::addTileResources(Tile& tile, int count, const std::string &modelPath, const glm::vec3 &scale)
{
    for (int i = 0; i < count; i++)
        tile._resources.emplace_back(getModel(modelPath, scale), getTileRandPos(tile));
}

std::unique_ptr<Mesh> Renderer::initTile()
{
    //Doc : https://learnopengl.com/Getting-started/Hello-Triangle#:~:text=Vertex%20input

    std::vector<Vertex> vertices = {
        makeTileVertex({0.f,  0.f, 0.f}, {0.f, 0.f}),
        makeTileVertex({1.f,  0.f, 0.f}, {1.f, 0.f}),
        makeTileVertex({1.f,  0.f, 1.f}, {1.f, 1.f}),
        makeTileVertex({0.f,  0.f, 1.f}, {0.f, 1.f}),

        makeTileVertex({0.f, -1.f, 0.f}, {0.f, 0.f}),
        makeTileVertex({1.f, -1.f, 0.f}, {1.f, 0.f}),
        makeTileVertex({1.f, -1.f, 1.f}, {1.f, 1.f}),
        makeTileVertex({0.f, -1.f, 1.f}, {0.f, 1.f}),
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        0, 1, 5,  5, 4, 0,
        2, 3, 7,  7, 6, 2,
        0, 3, 7,  7, 4, 0,
        1, 2, 6,  6, 5, 1,
    };

    GLuint texId;
    texId = TextureLoader::load("rock.jpg");

    std::vector<Texture> textures = {{ texId, "texture_diffuse" }};
    return std::make_unique<Mesh>(vertices, indices, textures);
}

void Renderer::initPlayersModels(GameState& state)
{
    std::vector<std::string> teams = state.getTeams();

    for (auto& [id, player] : state.getPlayers()) {
        _playersModels.insert({
            id,
            createPlayerModel(player, state)
        });
    }
}

void Renderer::uptPlayerModel(int playerId, std::shared_ptr<Model> &playerModel, GameState& state)
{
    Player *player = state.findPlayer(playerId);

    if (player == nullptr)
        return;
    if (player->_isAlive) {
        playerModel->moveTo(
            glm::vec3(player->_x, 0.f, player->_y),
            getPlayerRotation(player->_orientation)
        );
        _playerAnimations[playerId] = player->_animation;
        _playerAnimationTimers[playerId] = getAnimationDuration(player->_animation);
        player->_animation = "idle";
    } else {
        auto it = _playerRenderPositions.find(playerId);
        if (it == _playerRenderPositions.end()
            || static_cast<int>(std::floor(it->second.x)) != player->_x
            || static_cast<int>(std::floor(it->second.z)) != player->_y) {
            _playerRenderPositions[playerId] = getTileRandPos(glm::vec3(player->_x, 0.f, player->_y));
        }
        playerModel->moveTo(
            _playerRenderPositions[playerId],
            getPlayerRotation(player->_orientation)
        );
        if (_playerAnimations[playerId] != "Death") {
            _playerAnimations[playerId] = "Death";
            _playerAnimationTimers[playerId] = getAnimationDuration("Death");
        }
        player->_animation = "idle";
    }
}

void Renderer::cleanupPlayerModel(int playerId)
{
    _playersModels.erase(playerId);
    _playerRenderPositions.erase(playerId);
    _playerAnimations.erase(playerId);
    _playerAnimationTimers.erase(playerId);
}

bool Renderer::isSelectedTile(int x, int z) const
{
    return _selectedTile && _selectedTile->x == x && _selectedTile->y == z;
}

std::optional<glm::vec3> Renderer::getGroundHitPoint(sf::Vector2i mousePosition) const
{
    const sf::Vector2u size = _window.getSize();

    if (size.x == 0 || size.y == 0)
        return std::nullopt;

    const float x = (2.f * static_cast<float>(mousePosition.x)) / static_cast<float>(size.x) - 1.f;
    const float y = 1.f - (2.f * static_cast<float>(mousePosition.y)) / static_cast<float>(size.y);
    const glm::mat4 inverseViewProjection = glm::inverse(getProjectionMatrix() * _camera.getView());

    glm::vec4 nearPoint = inverseViewProjection * glm::vec4(x, y, -1.f, 1.f);
    glm::vec4 farPoint = inverseViewProjection * glm::vec4(x, y, 1.f, 1.f);
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;

    const glm::vec3 origin = glm::vec3(nearPoint);
    const glm::vec3 direction = glm::normalize(glm::vec3(farPoint - nearPoint));

    if (std::abs(direction.y) < 0.0001f)
        return std::nullopt;

    const float distance = -origin.y / direction.y;
    if (distance < 0.f)
        return std::nullopt;
    return origin + direction * distance;
}

glm::mat4 Renderer::getProjectionMatrix() const
{
    const sf::Vector2u size = _window.getSize();
    const float height = size.y == 0 ? 1.f : static_cast<float>(size.y);
    const float aspect = static_cast<float>(size.x) / height;

    return glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.f);
}

const sf::VideoMode& Renderer::getVideoMode() const
{
    return _videoMode;
}

const sf::RenderWindow& Renderer::getWindow() const
{
    return _window;
}

sf::RenderWindow& Renderer::getWindow()
{
    return _window;
}

Camera& Renderer::getCamera()
{
    return _camera;
}

glm::vec3 Renderer::getTileCenter(const Tile& tile)
{
    return {tile.x + 0.5f, 0.f, tile.z + 0.5f};
}

glm::vec3 Renderer::getTileRandPos(const Tile& tile)
{
    float offsetX = 0.1f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f;
    float offsetZ = 0.1f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f;

    return {tile.x + offsetX, 0.f, tile.z + offsetZ};
}

glm::vec3 Renderer::getTileRandPos(glm::vec3 tilePos)
{
    float offsetX = 0.1f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f;
    float offsetZ = 0.1f + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.8f;

    return {tilePos.x + offsetX, tilePos.y, tilePos.z + offsetZ};
}

std::shared_ptr<Model> Renderer::createPlayerModel(const Player& player, const GameState& state)
{
    return std::make_shared<Model>(
        getTeamModel(player, state),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.25f)
    );
}

std::shared_ptr<Model> Renderer::getModel(const std::string &path, glm::vec3 scale)
{
    const std::string key = makeModelCacheKey(path, scale);

    if (_modelsCache.find(key) == _modelsCache.end())
        _modelsCache[key] = std::make_shared<Model>(path, glm::vec3(0.f, 0.f, 0.f), scale);
    return _modelsCache[key];
}

std::string Renderer::makeModelCacheKey(const std::string &path, const glm::vec3 &scale) const
{
    std::ostringstream key;

    key << path << ':' << scale.x << ':' << scale.y << ':' << scale.z;
    return key.str();
}
