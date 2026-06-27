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

Renderer::Renderer()
{
    _videoMode.size.x  = 1920;
    _videoMode.size.y  = 1080;

    _settings.depthBits      = 24;
    _settings.majorVersion   = 3;
    _settings.minorVersion   = 3;
   // _settings.attributeFlags = sf::ContextSettings::Core;

    _window = sf::RenderWindow(_videoMode, "Zappy", sf::State::Windowed, _settings);
    _window.setMouseCursorVisible(false);
    _window.setFramerateLimit(60);
}

void Renderer::init()
{
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    srand(time(0));

    _font.openFromFile("fonts/arial.ttf");

    _shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    _groundTextures.push_back(TextureLoader::load("ground_1.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_2.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_2.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_2.jpg"));
    _groundTextures.push_back(TextureLoader::load("ground_3.jpg"));
    _tileMesh = initTile();
}

void Renderer::render(GameState& state, float deltaTime)
{
    glm::mat4 view = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
    std::vector<std::vector<Tile>>& map = state.getMap();

    _shader->use();
    _shader->setMat4("view", view);
    _shader->setMat4("projection", projection);

    for (int playerId : state._playerToUpt) {
        auto it = _playersModels.find(playerId);

        if (it != _playersModels.end()) {
            uptPlayerModel(it->first, it->second, state);
        } else {
            Player *player = state.findPlayer(playerId);

            if (player != nullptr) {
                _playersModels.insert({playerId, std::make_shared<Model>( getTeamModel(*player, state), getTileRandPos(state.getTile(player->_x, player->_y)), glm::vec3(0.15f), getPlayerRotation(player->_orientation), glm::vec3(0.f))});
            }
        }
    }

    for (auto& [playerId, playerModel] : _playersModels) {
        std::string animation = "idle";

        if (_playerAnimationTimers[playerId] > 0.f) {
            animation = _playerAnimations[playerId];
            _playerAnimationTimers[playerId] -= deltaTime;
        } else {
            _playerAnimations[playerId] = "idle";
            _playerAnimationTimers[playerId] = 0.f;
        }
        playerModel->updateAnimation(deltaTime, animation);
        playerModel->Draw(*_shader);
    }

    state._playerToUpt.clear();

    renderMap(map);
    renderHUD(state);
}

void Renderer::renderHUD(const GameState &state)
{
    glDisable(GL_DEPTH_TEST);
    _window.resetGLStates();

    float W = _window.getSize().x;
    float H = _window.getSize().y;

    std::map<std::string, int> teamLevel;
    for (auto &[id, p] : state.getPlayers()) {
        if (teamLevel[p._teamName] < p._level)
            teamLevel[p._teamName] = p._level;
    }

    std::vector<std::pair<std::string, int>> teams(teamLevel.begin(), teamLevel.end());
    std::sort(teams.begin(), teams.end(), [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b){
        return a.second > b.second;
    });
    int nbTeams = teams.size();
    if (nbTeams > 5)
        nbTeams = 5;
    float leaderboardHeight = 36.f + nbTeams * 22.f + 10.f;

    sf::RectangleShape leaderboardBg(sf::Vector2f(220.f, leaderboardHeight));
    leaderboardBg.setPosition({5.f, 5.f});
    leaderboardBg.setFillColor(sf::Color(15, 15, 38));
    _window.draw(leaderboardBg);

    sf::Text title(_font, "=== Top equipes ===", 16);
    title.setFillColor(sf::Color::Yellow);
    title.setPosition({10.f, 10.f});
    _window.draw(title);

    for (int i = 0; i < nbTeams; i++) {
        std::string line = std::to_string(i + 1) + ". " + teams[i].first + " - Lv " + std::to_string(teams[i].second);
        sf::Text text(_font, line, 14);
        text.setFillColor(sf::Color::White);
        text.setPosition({10.f, 36.f + i * 22.f});
        _window.draw(text);
    }

    const std::vector<std::pair<std::string, std::string>> &bc = state.getBroadcasts();
    float broadcastHeight = bc.size() * 22.f + 10.f;

    sf::RectangleShape broadcastBg(sf::Vector2f(410.f, broadcastHeight));
    broadcastBg.setPosition({W - 410.f, H - 130.f});
    broadcastBg.setFillColor(sf::Color(15, 15, 38));
    _window.draw(broadcastBg);

    for (int i = 0; i < bc.size(); i++) {
        std::string line = "[" + bc[i].first + "] " + bc[i].second;
        sf::Text text(_font, line, 15);
        text.setFillColor(sf::Color::White);
        text.setPosition({W - 400.f, H - 120.f + i * 22.f});
        _window.draw(text);
    }

    glEnable(GL_DEPTH_TEST);
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

    if (tile._resources.empty()) {
        for (int i = 0; i < tile.linemate; i++)
            tile._resources.push_back(Resource(getModel("model/Linemate.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.deraumere; i++)
            tile._resources.push_back(Resource(getModel("model/Deraumere.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.mendiane; i++)
            tile._resources.push_back(Resource(getModel("model/Mendiane.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.phiras; i++)
            tile._resources.push_back(Resource(getModel("model/Phiras.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.sibur; i++)
            tile._resources.push_back(Resource(getModel("model/Sibur.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.thystame; i++)
            tile._resources.push_back(Resource(getModel("model/Thystame.glb", glm::vec3(0.15f)), getTileRandPos(tile)));
        for (int i = 0; i < tile.food; i++)
            tile._resources.push_back(Resource(getModel("model/Food.glb", glm::vec3(200.f)), getTileRandPos(tile)));
    }

    for (auto& resource : tile._resources) {
        glm::mat4 model = glm::translate(glm::mat4(1.f), resource._pos);
        _shader->setMat4("model", model);
        resource._model->Draw(*_shader, resource._pos);
    }
}

std::unique_ptr<Mesh> Renderer::initTile()
{
    //Doc : https://learnopengl.com/Getting-started/Hello-Triangle#:~:text=Vertex%20input

    std::vector<Vertex> vertices = {
        {{0.f,  0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}},
        {{1.f,  0.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 0.f}},
        {{1.f,  0.f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 1.f}},
        {{0.f,  0.f, 1.f}, {0.f, 0.f, 0.f}, {0.f, 1.f}},

        {{0.f, -1.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}},
        {{1.f, -1.f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 0.f}},
        {{1.f, -1.f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 1.f}},
        {{0.f, -1.f, 1.f}, {0.f, 0.f, 0.f}, {0.f, 1.f}},
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
            std::make_shared<Model>(getTeamModel(player, state), getTileRandPos(state.getTile(player._x, player._y)), glm::vec3(0.15f), getPlayerRotation(player._orientation), glm::vec3(0.f))
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
        _playersModels.erase(playerId);
        _playerAnimations.erase(playerId);
        _playerAnimationTimers.erase(playerId);
    }
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
    float offsetX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float offsetZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    return {tile.x + offsetX, 0.f, tile.z + offsetZ};
}

std::shared_ptr<Model> Renderer::getModel(const std::string path, glm::vec3 scale)
{
    if (_modelsCache.find(path) == _modelsCache.end())
        _modelsCache[path] = std::make_shared<Model>(path, glm::vec3(0.f, 0.f, 0.f), scale);
    return _modelsCache[path];;
}
