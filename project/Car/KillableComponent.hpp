#pragma once

#include "Component.hpp"

class KillableComponent : public Component
{
public:
    explicit KillableComponent(GameObject *gameObject);

    void onCollisionStart(PhysicsComponent *comp) override;

    void onCollisionEnd(PhysicsComponent *comp) override;

    void update(float deltaTime) override;

private:
    float maxHealth = 100;
    float health = maxHealth;
    float damageSpeedThreshold = 100;
};
