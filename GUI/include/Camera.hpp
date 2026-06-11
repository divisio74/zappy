#pragma once

#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    // https://learnopengl.com/Getting-started/Camera
    public:
        Camera();
        ~Camera() = default;

        glm::mat4 getView() const;

        void processInput(float deltaTime);
        void processMouse(sf::Window &window);

    private:
        glm::vec3 _position;
        glm::vec3 _front;
        glm::vec3 _up;
        float _yaw   = -90.f;
        float _pitch = 0.f;
};