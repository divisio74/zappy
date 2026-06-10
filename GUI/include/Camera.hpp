#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    // https://learnopengl.com/Getting-started/Camera
    public:
        Camera();
        ~Camera() = default;

        glm::mat4 getView() const;

        void processKeyboard(float deltaTime);
        void processMouse(float offsetX, float offsetY);

    private:
        glm::vec3 _position;
        glm::vec3 _target;
        glm::vec3 _direction;
        glm::vec3 _up;
        glm::vec3 _right;
        glm::vec3 _cameraUp;
        glm::mat4 _view;
        sf::Clock _clock;
};