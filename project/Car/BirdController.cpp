//
// Created by Morten Nobel-Jørgensen on 19/10/2017.
//

#include <SDL_events.h>
#include <iostream>
#include "BirdController.hpp"
#include "GameObject.hpp"
#include "SpriteComponent.hpp"
#include "PhysicsComponent.hpp"
#include "BirdGame.hpp"
#include "SpriteComponent.hpp"

BirdController::BirdController(GameObject *gameObject) : Component(gameObject) {
    // initiate bird physics
}

bool BirdController::onKey(SDL_Event &event) {
    bool keyDown = event.type == SDL_KEYDOWN;
    switch (event.key.keysym.sym) {
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
    if (event.type == SDL_KEYDOWN){

    } else if (event.type == SDL_KEYUP){
        
    }
    return false;
}

void BirdController::onCollisionStart(PhysicsComponent *comp) {
    std::cout << "bird collided with something" << std::endl;
}

void BirdController::onCollisionEnd(PhysicsComponent *comp) {

}

void BirdController::update(float deltaTime) {
    auto phys = gameObject->getComponent<PhysicsComponent>();
}


