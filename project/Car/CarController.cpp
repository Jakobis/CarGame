//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
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

#define SPEED 20

void CarController::update(float deltaTime)
{
    auto phys = gameObject->getComponent<PhysicsComponent>();
    if (forward)
        phys->addForce({0, SPEED * deltaTime});
    if (backwards)
        phys->addForce({0, -SPEED * deltaTime});
    if (left)
        phys->addAngularImpulse(-SPEED * deltaTime);
    if (right)
        phys->addAngularImpulse(SPEED * deltaTime);
}
