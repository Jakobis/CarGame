//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "EnemyComponent.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"
#include "SpriteComponent.hpp"

EnemyComponent::EnemyComponent(GameObject *gameObject) : Component(gameObject)
{
    // initiate Car physics
}


void EnemyComponent::onCollisionStart(PhysicsComponent *comp)
{
    std::cout << "Enemy collided with something" << std::endl;
}

void EnemyComponent::onCollisionEnd(PhysicsComponent *comp)
{
}

void EnemyComponent::update(float deltaTime)
{
    auto phys = gameObject->getComponent<PhysicsComponent>();
}
