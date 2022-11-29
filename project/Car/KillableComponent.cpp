//

// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "KillableComponent.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"
#include "SpriteComponent.hpp"

KillableComponent::KillableComponent(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;

}


void KillableComponent::onCollisionStart(PhysicsComponent *comp)
{
    auto collisionspeed = glm::length(comp->getLinearVelocity() - gameObject->getComponent<PhysicsComponent>()->getLinearVelocity());
    std::cout << "Enemy collided with something with speed: " << collisionspeed << std::endl;
    if (collisionspeed >= damageSpeedThreshold) {
        health -= collisionspeed;
        std::cout << "Enemy crashed" << std::endl;
        if (health <= 0) {
            shouldDelete = true;
            std::cout << "Enemy has died" << std::endl;
        }
    }

}

void KillableComponent::onCollisionEnd(PhysicsComponent *comp)
{
}

void KillableComponent::update(float deltaTime)
{
}
