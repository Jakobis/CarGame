//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include "SpriteAnimationComponent.hpp"
#include "GameObject.hpp"
#include <memory>
#include <iostream>

SpriteAnimationComponent::SpriteAnimationComponent(GameObject *gameObject) : Component(gameObject) {}

void SpriteAnimationComponent::update(float deltaTime) {
    auto spriteComponent = gameObject->getComponent<SpriteComponent>();

    assert(spriteComponent != nullptr);

    time += deltaTime;

    if (time > animationTime){
        time = fmod(time, animationTime);
        auto last = spriteIndex;
        spriteIndex = (spriteIndex + 1) % sprites.size();
        if (!repeating && (spriteIndex < last)) {
            gameObject->remove();
        }
        //std::cout << !repeating << " " << (spriteIndex < last) << "\n";
        spriteComponent->setSprite(sprites[spriteIndex]);
    }
}

void SpriteAnimationComponent::setSprites(std::vector<sre::Sprite> sprites) {
    this->sprites = sprites;
}

float SpriteAnimationComponent::getAnimationTime() const {
    return animationTime;
}

void SpriteAnimationComponent::setAnimationTime(float animationTime) {
    SpriteAnimationComponent::animationTime = animationTime;
}
void SpriteAnimationComponent::setRepeating(bool repeating) {
    this->repeating = repeating;
}
