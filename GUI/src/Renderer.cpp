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

    _shader = _createProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    //Doc : https://learnopengl.com/Getting-started/Hello-Triangle#:~:text=Vertex%20input

    float vertices[] = {

        -3.f, 0.5f, -2.f, 0.f, 0.f,
         3.f, 0.5f, -2.f, 1.f, 0.f,
         3.f, 0.5f,  2.f, 1.f, 1.f,
        -3.f, 0.5f,  2.f, 0.f, 1.f,

        -3.f, -0.5f, -2.f, 0.f, 0.f,
         3.f, -0.5f, -2.f, 1.f, 0.f,
         3.f, -0.5f,  2.f, 1.f, 1.f,
        -3.f, -0.5f,  2.f, 0.f, 1.f,
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0,
        4, 5, 6,  6, 7, 4,
        0, 1, 5,  5, 4, 0,
        2, 3, 7,  7, 6, 2,
        0, 3, 7,  7, 4, 0,
        1, 2, 6,  6, 5, 1,
    };

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);

    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void*>(0));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);

    sf::Image image;
    image.loadFromFile("textures/rock.jpg");

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

}

void Renderer::render()
{
    glUseProgram(_shader);

    glm::mat4 model      = glm::mat4(1.f);
    glm::mat4 view       = _camera.getView();
    glm::mat4 projection = glm::perspective(glm::radians(45.f), 800.f / 600.f, 0.1f, 100.f);

    glUniformMatrix4fv(glGetUniformLocation(_shader, "model"),      1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(_shader, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(_shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(_VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glUniform1i(glGetUniformLocation(_shader, "texture1"), 0);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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

Camera& Renderer::getCamera()
{
    return _camera;
}