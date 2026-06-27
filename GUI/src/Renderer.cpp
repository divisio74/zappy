#include "Renderer.hpp"

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
        return "model/Astronaut.glb";

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
            return "model/Astronaut.glb";
    }
}

static Vertex makeTileVertex(glm::vec3 position, glm::vec2 texCoords)
{
    return {position, glm::vec3(0.f), texCoords, glm::ivec4(0), glm::vec4(0.f)};
}

Renderer::Renderer()
{
    _videoMode.size.x  = 3200;
    _videoMode.size.y  = 1920;

    _settings.depthBits      = 24;
    _settings.majorVersion   = 3;
    _settings.minorVersion   = 3;
    _settings.attributeFlags = sf::ContextSettings::Core;

    _window = sf::Window(_videoMode, "Zappy", sf::State::Windowed, _settings);
    _window.setMouseCursorVisible(false);
    _window.setFramerateLimit(60);
}

Renderer::~Renderer()
{
    shutdown();
}

void Renderer::shutdown()
{
    if (_window.isOpen() && !_window.setActive(true))
        std::cerr << "Failed to activate OpenGL context before shutdown" << std::endl;
    releaseResources();
    if (_window.isOpen())
        _window.close();
}

void Renderer::releaseResources()
{
    _playersModels.clear();
    _playerAnimations.clear();
    _playerAnimationTimers.clear();
    _modelsCache.clear();
    if (_tileMesh) {
        for (Texture &texture : _tileMesh->textures)
            deleteTexture(texture.id);
        _tileMesh.reset();
    }
    _shader.reset();
    _cubemapShader.reset();

    for (GLuint &texture : _groundTextures)
        deleteTexture(texture);
    std::vector<GLuint>().swap(_groundTextures);

    deleteTexture(_cubemapTexture);

    if (skyboxVBO != 0)
        glDeleteBuffers(1, &skyboxVBO);
    if (skyboxVAO != 0)
        glDeleteVertexArrays(1, &skyboxVAO);
    skyboxVBO = 0;
    skyboxVAO = 0;
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

    _shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    _cubemapShader = std::make_unique<Shader>("shaders/cubemapVert.glsl", "shaders/cubemapFrag.glsl");

    _cubemapShader->use();
    _cubemapShader->setInt("skybox", 0);

    _groundTextures.push_back(TextureLoader::load("ground_1.jpg"));
    for (int i = 0; i < 3; i++)
        _groundTextures.push_back(TextureLoader::load("ground_2.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_3.jpg"));
    _tileMesh = initTile();

    initCubemap();

    std::vector<std::string> faces =
    { 
        "textures/right.jpg",
        "textures/left.jpg",
        "textures/top.jpg",
        "textures/bottom.jpg",
        "textures/front.jpg",
        "textures/back.jpg"
    };

    _cubemapTexture = loadCubemap(faces);
}

void Renderer::render(GameState& state, float deltaTime)
{
    glm::mat4 view = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
    std::vector<std::vector<Tile>>& map = state.getMap();

    renderSkybox(projection);

    _shader->use();
    _shader->setMat4("view", view);
    _shader->setMat4("projection", projection);

    updatePlayers(state, deltaTime);
    renderEggs(state);
    renderMap(map);
}

void Renderer::renderSkybox(const glm::mat4 &projection)
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    _cubemapShader->use();
    _cubemapShader->setMat4("view", glm::mat4(glm::mat3(_camera.getView())));
    _cubemapShader->setMat4("projection", projection);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, _cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

void Renderer::updatePlayers(GameState& state, float deltaTime)
{
    for (int playerId : state._playerToUpt) {
        auto it = _playersModels.find(playerId);

        if (it != _playersModels.end()) {
            uptPlayerModel(it->first, it->second, state);
        } else {
            Player *player = state.findPlayer(playerId);

            if (player != nullptr) {
                _playersModels.insert({playerId, getModel(getTeamModel(*player, state), glm::vec3(0.25f))});
            }
        }
    }

    for (auto it = _playersModels.begin(); it != _playersModels.end();) {
        int playerId = it->first;
        std::shared_ptr<Model> &playerModel = it->second;
        Player *player = state.findPlayer(playerId);
        std::string animation = "idle";

        if (player == nullptr || !player->_isAlive) {
            it = _playersModels.erase(it);
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
        playerModel->moveTo(
            glm::vec3(player->_x, 0.f, player->_y),
            getPlayerRotation(player->_orientation)
        );
        playerModel->updateAnimation(deltaTime, animation);
        playerModel->Draw(*_shader);
        ++it;
    }

    state._playerToUpt.clear();
}

void Renderer::renderEggs(GameState& state)
{
    for (auto& egg : state.getEggs()) {
        glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(egg._x, 0.2f, egg._y));
        _shader->setMat4("model", model);
        if(!egg._model)
            egg._model = getModel("model/Egg.glb", glm::vec3(0.5f));
        if (egg._isAlive)
            egg._model->Draw(*_shader, glm::vec3(egg._x, 0.2f, egg._y));
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

    _tileMesh->textures[0].id = _groundTextures[tile.textureIndex];
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
            getModel(getTeamModel(player, state), glm::vec3(0.25f))
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
    } else {
        cleanupPlayerModel(playerId);
    }
}

void Renderer::cleanupPlayerModel(int playerId)
{
    _playersModels.erase(playerId);
    _playerAnimations.erase(playerId);
    _playerAnimationTimers.erase(playerId);
}

const sf::VideoMode& Renderer::getVideoMode() const
{
    return _videoMode;
}

const sf::Window& Renderer::getWindow() const
{
    return _window;
}

sf::Window& Renderer::getWindow()
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
    float offsetX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float offsetZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    return {tile.x + offsetX, 0.f, tile.z + offsetZ};
}

glm::vec3 Renderer::getTileRandPos(glm::vec3 tilePos)
{
    float offsetX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float offsetZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    return {tilePos.x + offsetX, tilePos.y, tilePos.z + offsetZ};
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

unsigned int Renderer::loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        sf::Image image;

        if (!image.loadFromFile(faces[i]))
        {
            std::cout << "Failed to load: " << faces[i] << std::endl;
            continue;
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0,
            GL_RGBA,
            image.getSize().x,
            image.getSize().y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image.getPixelsPtr()
        );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void Renderer::initCubemap()
{
    float skyboxVertices[] =
    {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(skyboxVertices),
        skyboxVertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glBindVertexArray(0);
}
