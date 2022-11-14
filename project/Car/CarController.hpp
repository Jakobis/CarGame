#pragma once

#include "Component.hpp"

class CarController : public Component
{
public:
    explicit CarController(GameObject *gameObject);

    bool onKey(SDL_Event &event) override;

    void onCollisionStart(PhysicsComponent *comp) override;

    void onCollisionEnd(PhysicsComponent *comp) override;

    void update(float deltaTime) override;

private:
    bool forward = false;
    bool backwards = false;
    bool right = false;
    bool left = false;
};
