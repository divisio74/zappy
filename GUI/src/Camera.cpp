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

void Camera::processMouse(sf::Window &window)
{
    sf::Vector2i center(window.getSize().x / 2, window.getSize().y / 2);
    sf::Vector2i mouse = sf::Mouse::getPosition(window);
    sf::Vector2i delta = mouse - center;
    sf::Mouse::setPosition(center, window);

    float sensitivity = 0.1f;
    _yaw   += delta.x * sensitivity;
    _pitch -= delta.y * sensitivity;
    _pitch  = glm::clamp(_pitch, -89.f, 89.f);

    _front = glm::normalize(glm::vec3(
        cos(glm::radians(_yaw)) * cos(glm::radians(_pitch)),
        sin(glm::radians(_pitch)),
        sin(glm::radians(_yaw)) * cos(glm::radians(_pitch))
    ));
}