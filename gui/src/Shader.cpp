#include "Shader.hpp"

Shader::Shader(const std::string& vertPath, const std::string& fragPath)
{
    GLuint vert = _compile(GL_VERTEX_SHADER,   vertPath);
    GLuint frag = _compile(GL_FRAGMENT_SHADER, fragPath);

    _id = glCreateProgram();
    glAttachShader(_id, vert);
    glAttachShader(_id, frag);
    glLinkProgram (_id);

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader()
{
    if (_id != 0)
        glDeleteProgram(_id);
}

void Shader::use()
{
    glUseProgram(_id);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat)
{
    glUniformMatrix4fv(glGetUniformLocation(_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setInt(const std::string& name, int value)
{
    glUniform1i(glGetUniformLocation(_id, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec)
{
    glUniform3f(glGetUniformLocation(_id, name.c_str()), vec.x, vec.y, vec.z);
}

GLuint Shader::_compile(GLenum type, const std::string& path)
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
