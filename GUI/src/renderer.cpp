#include "renderer.hpp"

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

    _shader = _createProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint _VAO;
    GLuint _VBO;
    GLuint _EBO;
}

GLuint Renderer::_compileShader(GLenum type, const std::string& path)
{
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string src = ss.str();
    const char* c = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &c, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint Renderer::_createProgram(const std::string& vertPath, const std::string& fragPath)
{
    GLuint vert = _compileShader(GL_VERTEX_SHADER,   vertPath);
    GLuint frag = _compileShader(GL_FRAGMENT_SHADER, fragPath);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram (prog);

    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
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