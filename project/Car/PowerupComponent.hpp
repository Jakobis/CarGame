#pragma once

#include "Component.hpp"
#include "sre/SpriteBatch.hpp"
#include "sre/Texture.hpp"
#include "sre/RenderPass.hpp"

enum class PowerupType
{
    Heal,
    Invulnerability
};

class PowerupComponent
{
public:
    PowerupComponent();
    void init(PowerupType type);
    PowerupType getType();
private:
    PowerupType type;
};
