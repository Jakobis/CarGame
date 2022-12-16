#pragma once

#include "Component.hpp"

class KillableComponent : public Component
{
public:
    explicit KillableComponent(GameObject *gameObject);

    void onCollisionStart(PhysicsComponent *comp) override;

    void onCollisionEnd(PhysicsComponent *comp) override;

    void update(float deltaTime) override;
    
    int getPointValue();

private:
    float maxHealth = 200;
    float health = maxHealth;
    float damageSpeedThreshold = 50;
    friend class CarGame;
    int pointValue = 1;
};
