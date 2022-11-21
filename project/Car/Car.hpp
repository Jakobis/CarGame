#pragma once
#include "GameObject.hpp"

enum Control
{
    C_UP = 0b0001,
    C_LEFT = 0b0010,
    C_DOWN = 0b0100,
    C_RIGHT = 0b1000,
};

class Tire
{
public:
    explicit Tire(GameObject *gameObject);
    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float maxTorque);
    void updateFriction();
    void updateDrive(Control control);
    void updateTurn(Control control);

private:
    GameObject *gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    float maxForwardSpeed;
    float maxBackwardSpeed;
    float maxDriveForce;
    float maxLateralImpulse;
    float currentTraction;
    float maxTorque;
    friend class Car;
};

class Car
{
public:
    explicit Car(GameObject *gameObject, GameObject tireObjects[4]);

private:
    GameObject *gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    std::vector<std::shared_ptr<Tire>> tires;
    std::shared_ptr<b2RevoluteJoint> flJoint;
    std::shared_ptr<b2RevoluteJoint> frJoint;
};
