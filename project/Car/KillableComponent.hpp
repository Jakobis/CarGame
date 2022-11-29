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
    bool shouldDelete = false;
    float health = 200;
    float maxHealth = 200; 
    float damageSpeedThreshold = 100;
};
