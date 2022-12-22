//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include <sre/SpriteAtlas.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "PowerupComponent.hpp"
#include "CarGame.hpp"

using namespace sre;
using namespace glm;

PowerupComponent::PowerupComponent()
{
}

void PowerupComponent::init(PowerupType type)
{
    this->type = type;
}

PowerupType PowerupComponent::getType() 
{
    return type;
}
