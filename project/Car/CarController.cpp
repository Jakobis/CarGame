//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"
#include "CarController.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"
#include "SpriteComponent.hpp"

CarController::CarController(GameObject *gameObject) : Component(gameObject)
{
    // initiate Car physics
}

bool CarController::onKey(SDL_Event &event)
{
    bool keyDown = event.type == SDL_KEYDOWN;
    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        forward = keyDown;
        break;
    case SDLK_s:
        backwards = keyDown;
        break;
    case SDLK_a:
        left = keyDown;
        break;
    case SDLK_d:
        right = keyDown;
        break;
    }
    if (event.type == SDL_KEYDOWN)
    {
    }
    else if (event.type == SDL_KEYUP)
    {
    }
    return false;
}

void CarController::onCollisionStart(PhysicsComponent *comp)
{
    std::cout << "Car collided with something" << std::endl;
}

void CarController::onCollisionEnd(PhysicsComponent *comp)
{
}

#define SPEED 10

void CarController::update(float deltaTime)
{
    auto phys = gameObject->getComponent<PhysicsComponent>();
    auto oldVel = phys->getLinearVelocity();
    auto newVel = glm::vec2(oldVel);
    if (newVel == glm::vec2({0, 0}))
        newVel = glm::vec2({0, 1});
    // if (forward)
    //     newVel += SPEED * 1.05 * deltaTime;
    // if (backwards)
    //     newVel -= SPEED * 1.05 * deltaTime;
    if (left)
        newVel = glm::rotate(glm::mat4(1), SPEED * deltaTime, {0, 0, 1}) * glm::vec4(newVel, 0, 1);
    if (right)
        newVel = glm::rotate(glm::mat4(1), -SPEED * deltaTime, {0, 0, 1}) * glm::vec4(newVel, 0, 1);
    // phys->addTorque(-SPEED * deltaTime);
    std::cout << glm::sin(newVel.x) << "\n";
    phys->setLinearVelocity(newVel);
    gameObject->setRotation(newVel.g);
}
