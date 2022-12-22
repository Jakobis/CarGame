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

class PowerupComponent : public Component
{
public:
    explicit PowerupComponent(GameObject *gameObject);
    void init(PowerupType type);
    PowerupType getType();
    void onCollisionStart(PhysicsComponent *comp) override;
private:
    PowerupType type;
};
