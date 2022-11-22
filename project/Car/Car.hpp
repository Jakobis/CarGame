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
    explicit Tire(std::shared_ptr<GameObject> gameObject);
    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float maxTorque);
    void updateFriction();
    void updateDrive(char control);
    void updateTurn(char control);

private:
    std::shared_ptr<GameObject> gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    float maxForwardSpeed;
    float maxBackwardSpeed;
    float maxDriveForce;
    float maxLateralImpulse;
    float currentTraction;
    float maxTorque;
    friend class Car;
};

class Car : public Component
{
public:
    explicit Car(GameObject *gameObject);
    void update(float deltaTime) override;
    bool onKey(SDL_Event &event) override;

private:
    GameObject *gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    std::vector<std::shared_ptr<Tire>> tires;
    std::shared_ptr<b2RevoluteJoint> flJoint;
    std::shared_ptr<b2RevoluteJoint> frJoint;
    char control = 0;
};
