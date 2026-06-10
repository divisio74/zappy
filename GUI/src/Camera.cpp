#include "Camera.hpp"

Camera::Camera()
{
    glm::vec3 _position = glm::vec3(0.0f, 0.0f, 3.0f);

    glm::vec3 _target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _direction = glm::normalize(_position - _target);

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); 
    glm::vec3 _right = glm::normalize(glm::cross(up, _direction));

    _up = glm::cross(_direction, _right);

    _view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), 
  		   glm::vec3(0.0f, 0.0f, 0.0f), 
  		   glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getView() const
{
    float time = _clock.getElapsedTime().asSeconds();
    float camX = sin(time) * 10.f;
    float camZ = cos(time) * 10.f;
    glm::mat4 view = glm::lookAt(
        glm::vec3(camX, 5.f, camZ),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f)
    );
    return view;
}