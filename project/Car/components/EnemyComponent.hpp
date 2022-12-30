#pragma once

#include "../Component.hpp"

class EnemyComponent : public Component
{
public:
    explicit EnemyComponent(GameObject *gameObject);

    void onCollisionStart(PhysicsComponent *comp) override;

    void onCollisionEnd(PhysicsComponent *comp) override;

    void update(float deltaTime) override;

    void init(std::shared_ptr<GameObject> player, glm::vec2 position);

private:
    std::shared_ptr<GameObject> player;
    float speed = 1000000;
};