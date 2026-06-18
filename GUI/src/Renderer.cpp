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

    _shader = std::make_unique<Shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    _tileMesh = initTile();
    _playersModels = initPlayersModels(NB_PLAYERS);

}

void Renderer::render()
{
    glm::mat4 view = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);

    _shader->use();
    _shader->setMat4("view", view);
    _shader->setMat4("projection", projection);

    renderMap(WIDTH, HEIGHT);

    for (uint i = 0; i < NB_PLAYERS; i++)
        _playersModels[i]->Draw(*_shader);
}

void Renderer::renderMap(uint width, uint height)
{
    for (uint y = 0; y < height; y++)
        for (uint x = 0; x < width; x++)
            renderTile(x, y);
}

void Renderer::renderTile(float x, float z)
{
    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(x, 0.f, z));
    _shader->setMat4("model", model);
    _tileMesh->Draw(*_shader);
}

std::vector<std::unique_ptr<Model>> Renderer::initPlayersModels(uint nbPlayers)
{
    std::vector<std::unique_ptr<Model>> players;
    for (uint i = 0; i < nbPlayers; i++) {
        glm::vec3 position = glm::vec3((i + 5) - 4.5f, 0.5f, 1.f);
        players.push_back(std::make_unique<Model>("model/Mecha.glb", position, glm::vec3(0.15f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f)));
    }
    return std::move(players);
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

std::vector<std::unique_ptr<Model>>& Renderer::getPlayersModels()
{
    return _playersModels;
}

glm::vec3 Renderer::getTileCenter(int x, int z)
{
    if (x < WIDTH && z < HEIGHT)
        return {x + 0.5f, 0.f, z + 0.5f};
    return {0.5f, 0.f, 0.5f};
}
