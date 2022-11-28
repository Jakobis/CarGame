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
    this->gameObject = gameObject;
    auto phys = gameObject->addComponent<PhysicsComponent>();
    glm::vec2 size(200 / 10, 500 / 10);
    phys->initBox(b2_dynamicBody, size, {10 / 10, 25 / 10}, 10);
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
    //std::cout << nullptr<< "\n";
    auto desiredPosition = player->getPosition();
    phys->addForce(glm::normalize(desiredPosition - gameObject->getPosition()) * speed * deltaTime);
    //gameObject->setPosition(gameObject->getPosition() + glm::normalize(desiredPosition - gameObject->getPosition()) * speed * deltaTime);
    //std::cout << "Enemy: " << gameObject->getPosition().x << " " << gameObject->getPosition().y << "\n";
}

void EnemyComponent::init(std::shared_ptr<GameObject> player) 
{
    this->player = player;
}
