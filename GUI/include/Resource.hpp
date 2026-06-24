#pragma once

#include "Model.hpp"

struct Resource {

    Resource();
    Resource(std::shared_ptr<Model> model, glm::vec3 _pos);

    std::shared_ptr<Model> _model;
    glm::vec3              _pos;
};