#include "Camera.hpp"

Camera::Camera()
    : _position(0.f, 3.f, 0.f), _front(0.f, 0.f, -1.f), _up(0.f, 1.f, 0.f) {}

glm::mat4 Camera::getView() const
{
    return glm::lookAt(_position, _position + _front, _up);
}

void Camera::processInput(float deltaTime)
{
    const float speed = 5.f * deltaTime;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
        _position += speed * _front;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        _position -= speed * _front;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
        _position -= glm::normalize(glm::cross(_front, _up)) * speed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        _position += glm::normalize(glm::cross(_front, _up)) * speed;
}