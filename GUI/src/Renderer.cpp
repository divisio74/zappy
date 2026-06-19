#include "Renderer.hpp"

Renderer::Renderer()
{
    _videoMode.size.x  = 1920;
    _videoMode.size.y  = 1080;

    _settings.depthBits      = 24;
    _settings.majorVersion   = 3;
    _settings.minorVersion   = 3;
    _settings.attributeFlags = sf::ContextSettings::Core;

    _window = sf::Window(_videoMode, "Zappy", sf::State::Windowed, _settings);
    _window.setMouseCursorVisible(false);
    _window.setFramerateLimit(60);
}

void Renderer::init()
{
    glewExperimental = GL_TRUE;
    glewInit();
    glEnable(GL_DEPTH_TEST);
    srand(time(0));

    _shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    _tileMesh = initTile();
}

void Renderer::render(GameState& state)
{
    glm::mat4 view = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);
    std::vector<std::vector<Tile>>& map = state.getMap();

    _shader->use();
    _shader->setMat4("view", view);
    _shader->setMat4("projection", projection);

    renderMap(map);
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
    _tileMesh->Draw(*_shader);

    if (tile.models.empty()) {
        for (int i = 0; i < tile.linemate; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_1.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
        for (int i = 0; i < tile.deraumere; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_2.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
        for (int i = 0; i < tile.sibur; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_3.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
        for (int i = 0; i < tile.mendiane; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_4.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
        for (int i = 0; i < tile.phiras; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_5.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
        for (int i = 0; i < tile.thystame; i++)
            tile.models.push_back(std::make_shared<Model>("model/min_6.glb", getTileRandPos(tile), glm::vec3(0.15f), glm::vec3(0.f), glm::vec3(0.f)));
    }

    for (auto& resource : tile.models)
        resource->Draw(*_shader);
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
    texId = TextureLoader::load("grass.jpg");

    std::vector<Texture> textures = {{ texId, "texture_diffuse" }};
    return std::make_unique<Mesh>(vertices, indices, textures);
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
