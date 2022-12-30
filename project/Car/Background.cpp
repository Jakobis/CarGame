//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include <sre/SpriteAtlas.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "Background.hpp"
#include "CarGame.hpp"

using namespace sre;
using namespace glm;

Background::Background()
{
}

void Background::renderBackground(sre::RenderPass &renderPass, float offsetX, float offsetY)
{
    // Move the tile grid, but only by integer size of sprite, giving no hint to player that batch is reused
    float x = offsetX - (std::fmod(offsetX, spriteSize.x));
    float y = offsetY - (std::fmod(offsetY, spriteSize.y));
    renderPass.draw(batch, glm::translate(vec3(x, y, 0)));
}

void Background::init(sre::Sprite sprite)
{
    // Setup sprite batch, creating a grid of background tiles which can be moved around the play space
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
