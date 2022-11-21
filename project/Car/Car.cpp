#include "Car.hpp"
#include "PhysicsComponent.hpp"
#include "SpriteComponent.hpp"

Tire::Tire(GameObject *gameObject)
{
    this->gameObject = gameObject;
    auto s = gameObject->addComponent<SpriteComponent>();
    phys = gameObject->addComponent<PhysicsComponent>();
    phys->initBox(b2_dynamicBody, {6, 12}, {3, 6}, 1);
    phys->setSensor(true);
}

void Tire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float maxTorque)
{
    this->maxForwardSpeed = maxForwardSpeed;
    this->maxBackwardSpeed = maxBackwardSpeed;
    this->maxDriveForce = maxDriveForce;
    this->maxLateralImpulse = maxLateralImpulse;
    this->maxTorque = maxTorque;
}

void Tire::updateTurn(Control control)
{
    float desiredTorque = 0;
    switch (control & (C_LEFT | C_RIGHT))
    {
    case C_LEFT:
        desiredTorque = maxTorque;
        break;
    case C_RIGHT:
        desiredTorque = -maxTorque;
        break;
    default:; // nothing
    }
    phys->addTorque(desiredTorque);
}

void Tire::updateDrive(Control control)
{
}

Car::Car(GameObject *gameObject, GameObject tireObjects[4])
{
    this->gameObject = gameObject;
    phys = gameObject->addComponent<PhysicsComponent>();
    phys->initBox(b2_dynamicBody, {20, 50}, {10, 25}, 1);
    phys->setAngularDamping(3);

    float maxForwardSpeed = 250;
    float maxBackwardSpeed = -40;
    float backTireMaxDriveForce = 300;
    float frontTireMaxDriveForce = 500;
    float backTireMaxLateralImpulse = 8.5;
    float frontTireMaxLateralImpulse = 7.5;
    float maxTorque = 15;

    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0;
    jointDef.upperAngle = 0;
    jointDef.localAnchorB.SetZero(); // center of tire

    // back left tire
    auto tire = std::make_shared<Tire>(tireObjects[0]);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse, maxTorque);
    jointDef.localAnchorA.Set(-3, 0.75);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);

    // back right tire
    auto tire = std::make_shared<Tire>(tireObjects[1]);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse, maxTorque);
    jointDef.localAnchorA.Set(3, 0.75);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
}
