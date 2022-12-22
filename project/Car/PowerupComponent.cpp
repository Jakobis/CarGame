//

// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "PowerupComponent.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"
#include "SpriteComponent.hpp"
#include "Car.hpp"
#include "GameObject.hpp"

PowerupComponent::PowerupComponent(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;
}

void PowerupComponent::onCollisionStart(PhysicsComponent *comp)
{
    if (comp->getGameObject()->getComponent<Car>() != nullptr)
    {
        gameObject->remove();
    }
}


PowerupType PowerupComponent::getType() {
    return type;
}

void PowerupComponent::init(PowerupType type) 
{
    this->type = type;
}