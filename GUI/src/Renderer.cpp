#include "Renderer.hpp"

Renderer::Renderer()
{
    _videoMode.size.x  = 1200;
    _videoMode.size.y  = 800;

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
}

void Renderer::render()
{
    glm::mat4 view       = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);

    (*_shader).setMat4("view", view);
    (*_shader).setMat4("projection", projection);

    _shader->use();

    renderMap(WIDTH, HEIGHT);
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

std::unique_ptr<Mesh> Renderer::initTile()
{
    //Doc : https://learnopengl.com/Getting-started/Hello-Triangle#:~:text=Vertex%20input

    std::vector<Vertex> vertices = {
        {{0.f,  0.5f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}},
        {{1.f,  0.5f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 0.f}},
        {{1.f,  0.5f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 1.f}},
        {{0.f,  0.5f, 1.f}, {0.f, 0.f, 0.f}, {0.f, 1.f}},

        {{0.f, -0.5f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}},
        {{1.f, -0.5f, 0.f}, {0.f, 0.f, 0.f}, {1.f, 0.f}},
        {{1.f, -0.5f, 1.f}, {0.f, 0.f, 0.f}, {1.f, 1.f}},
        {{0.f, -0.5f, 1.f}, {0.f, 0.f, 0.f}, {0.f, 1.f}},
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        0, 1, 5,  5, 4, 0,
        2, 3, 7,  7, 6, 2,
        0, 3, 7,  7, 4, 0,
        1, 2, 6,  6, 5, 1,
    };

    sf::Image image;
    image.loadFromFile("textures/grass.jpg");

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

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