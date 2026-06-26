#include "Resource.hpp"

Resource::Resource()
{
}

Resource::Resource(std::shared_ptr<Model> model, glm::vec3 pos)
{
    _model = model;
    _pos = pos;
}