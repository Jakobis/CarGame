//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include <sre/SpriteAtlas.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "BackgroundComponent.hpp"
#include "CarGame.hpp"

using namespace sre;
using namespace glm;

BackgroundComponent::BackgroundComponent()
{
}

void BackgroundComponent::renderBackground(sre::RenderPass &renderPass, float offset)
{
    renderPass.draw(batch, glm::translate(vec3(offset, 0, 0)));
}

void BackgroundComponent::init(sre::Sprite sprite)
{
    float scale = CarGame::windowSize.y / sprite.getSpriteSize().y;
    sprite.setScale({scale, scale});
    auto batchBuilder = SpriteBatch::create();
    for (int i = 0; i < 100; i++)
    {
        sprite.setPosition(vec2(sprite.getSpriteSize().x * (i - 1) * scale, 0));
        batchBuilder.addSprite(sprite);
    }
    batch = batchBuilder.build();
}
