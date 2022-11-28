#pragma once
#include "GameObject.hpp"
#include "PhysicsComponent.hpp"

enum Control
{
    C_NONE = 0,
    C_UP = 0b0001,
    C_LEFT = 0b0010,
    C_DOWN = 0b0100,
    C_RIGHT = 0b1000,
    C_ALL = 0b1111,
};

class Tire
{
public:
    explicit Tire(std::shared_ptr<GameObject> gameObject, sre::Sprite *sprite);
    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse);
    void updateFriction();
    void updateDrive(char control);

private:
    std::shared_ptr<GameObject> gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    float maxForwardSpeed;
    float maxBackwardSpeed;
    float maxDriveForce;
    float maxLateralImpulse;
    float currentTraction; // Between 0-1, 0 is slippery and 1 is perfect maneuverablity
    friend class Car;
};

class Car : public Component
{
public:
    explicit Car(GameObject *gameObject);
    void initTires(sre::Sprite *tireSprite);
    void update(float deltaTime) override;
    bool onKey(SDL_Event &event) override;

private:
    GameObject *gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    std::vector<std::shared_ptr<Tire>> tires;
    b2RevoluteJoint *flJoint;
    b2RevoluteJoint *frJoint;
    char control = 0;
};
