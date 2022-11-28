#pragma once

#include "Component.hpp"

class EnemyComponent : public Component
{
public:
    explicit EnemyComponent(GameObject *gameObject);

    void onCollisionStart(PhysicsComponent *comp) override;

    void onCollisionEnd(PhysicsComponent *comp) override;

    void update(float deltaTime) override;

private:
    
};
