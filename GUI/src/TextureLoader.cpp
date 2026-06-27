#include "TextureLoader.hpp"
#include <iostream>

GLuint TextureLoader::load(const std::string& path, const std::string& directory, const aiScene *scene)
{
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sf::Image image;

    bool loaded = false;

    if (scene) {
        const aiTexture* embeddedTex = scene->GetEmbeddedTexture(path.c_str());
        if (embeddedTex)
            loaded = image.loadFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
    } else {
        std::string fullPath = directory.empty() ? path : directory + "/" + path;
        loaded = image.loadFromFile(fullPath);
    }

    if (!loaded) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        glDeleteTextures(1, &texId);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);

    return texId;
}
