//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "EnemyComponent.hpp"
#include "../GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "../CarGame.hpp"
#include "SpriteComponent.hpp"

EnemyComponent::EnemyComponent(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;
}

void EnemyComponent::onCollisionStart(PhysicsComponent *comp) {}

void EnemyComponent::onCollisionEnd(PhysicsComponent *comp) {}

void EnemyComponent::update(float deltaTime)
{
    // Simple AI -- Always push enemy towards the player
    auto phys = gameObject->getComponent<PhysicsComponent>();
    auto desiredPosition = player->getPosition();
    phys->addForce(glm::normalize(desiredPosition - gameObject->getPosition()) * speed * deltaTime);
}

void EnemyComponent::init(std::shared_ptr<GameObject> player, glm::vec2 position)
{
    this->player = player;
    auto phys = gameObject->addComponent<PhysicsComponent>();
    glm::vec2 size(30 / 10, 80 / 10);
    phys->initBox(b2_dynamicBody, size, position, 5);
}
