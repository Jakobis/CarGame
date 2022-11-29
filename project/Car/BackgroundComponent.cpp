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

void BackgroundComponent::renderBackground(sre::RenderPass &renderPass, float offsetX, float offsetY)
{
    float x = offsetX - (std::fmod(offsetX, spriteSize.x));
    float y = offsetY - (std::fmod(offsetY, spriteSize.y));
    renderPass.draw(batch, glm::translate(vec3(x, y, 0)));
}

void BackgroundComponent::init(sre::Sprite sprite)
{
    float scale = CarGame::windowSize.y / sprite.getSpriteSize().y;
    sprite.setScale({scale, scale});
    spriteSize = sprite.getSpriteSize();
    spriteSize *= scale;
    auto batchBuilder = SpriteBatch::create();
    // 10 x 10 to be extra careful about wack screen sizes
    for (int i = -4; i < 6; i++)
        for (int j = -4; j < 6; j++)
        {
            sprite.setPosition({sprite.getSpriteSize().x * (i - 1) * scale, sprite.getSpriteSize().y * (j - 1) * scale});
            batchBuilder.addSprite(sprite);
        }
    batch = batchBuilder.build();
}
