#pragma once

#include "Component.hpp"

class BirdController : public Component {
public:
    explicit BirdController(GameObject *gameObject);

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
