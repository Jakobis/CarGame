#include "Car.hpp"
#include "CarGame.hpp"
#include "PhysicsComponent.hpp"
#include "SpriteComponent.hpp"
#include <iostream>

Tire::Tire(std::shared_ptr<GameObject> gameObject)
{
    this->gameObject = gameObject;
    // auto s = gameObject->addComponent<SpriteComponent>();
    phys = gameObject->addComponent<PhysicsComponent>();
    phys->initBox(b2_dynamicBody, {300 / 100, 300 / 100}, {3 / 100, 6 / 100}, 10);
    phys->setSensor(true);
    currentTraction = 1;
}

void Tire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float maxTorque)
{
    this->maxForwardSpeed = maxForwardSpeed;
    this->maxBackwardSpeed = maxBackwardSpeed;
    this->maxDriveForce = maxDriveForce;
    this->maxLateralImpulse = maxLateralImpulse;
    this->maxTorque = maxTorque;
}

void Tire::updateTurn(char control)
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

void Tire::updateFriction()
{
    auto impulse = phys->getLateralImpulse();
    if (impulse.length() > maxLateralImpulse)
        impulse *= maxLateralImpulse / impulse.length();
    phys->addImpulse(currentTraction * impulse);

    phys->addAngularImpulse(currentTraction * 0.1 * phys->getIntertia() * phys->getAngularVelocity());

    // auto currentForwardNormal = phys->getForwardVelocity();
    // auto currentForwardSpeed = currentForwardNormal.g;
    // auto dragForceMagnitude = -2 * currentForwardSpeed;
    // phys->addForce(currentTraction * dragForceMagnitude * currentForwardNormal);
}

void Tire::updateDrive(char control)
{
    // find desired speed
    float desiredSpeed = 0;
    switch (control & (C_UP | C_DOWN))
    {
    case C_UP:
        desiredSpeed = maxForwardSpeed;
        break;
    case C_DOWN:
        desiredSpeed = maxBackwardSpeed;
        break;
    default:
        return; // do nothing
    }

    // find current speed in forward direction
    auto currentForwardNormal = phys->getForwardVelocity(); // m_body->GetWorldVector(b2Vec2(0, 1));
    float currentSpeed = currentForwardNormal.length();
    // float currentSpeed = b2Dot(getForwardVelocity(), currentForwardNormal);

    // apply necessary force
    float force = 0;
    if (desiredSpeed > currentSpeed)
        force = maxDriveForce;
    else if (desiredSpeed < currentSpeed)
        force = -maxDriveForce;
    else
        return;
    std::cout << currentSpeed << " " << force << "\n";
    phys->addForce({0, force * 10});
}

Car::Car(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;
    phys = gameObject->addComponent<PhysicsComponent>();
    glm::vec2 size(200 / 100, 500 / 100);
    phys->initBox(b2_dynamicBody, size, {10 / 100, 25 / 100}, 10);
    phys->setAngularDamping(3);

    float maxForwardSpeed = 250;
    float maxBackwardSpeed = -40;
    float backTireMaxDriveForce = 300;
    float frontTireMaxDriveForce = 500;
    float backTireMaxLateralImpulse = 8.5;
    float frontTireMaxLateralImpulse = 7.5;
    float maxTorque = glm::radians(15.0);

    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0;
    jointDef.upperAngle = 0;
    jointDef.localAnchorB.SetZero(); // center of tire

    // back left tire
    auto tire = std::make_shared<Tire>(CarGame::instance->createGameObject());
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse, maxTorque);
    jointDef.localAnchorA.Set(-3, 0.75);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // back right tire
    tire = std::make_shared<Tire>(CarGame::instance->createGameObject());
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse, maxTorque);
    jointDef.localAnchorA.Set(3, 0.75);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);
}

void Car::update(float deltaTime)
{
    for (auto tire : tires)
    {
        tire->updateFriction();
    }
    for (auto tire : tires)
    {
        tire->updateDrive(control);
    }
    // control steering
    float lockAngle = glm::radians(35.0);
    float turnSpeedPerSec = glm::radians(160.0); // from lock to lock in 0.5 sec
    float turnPerTimeStep = turnSpeedPerSec / 60.0f;
    float desiredAngle = 0;
    switch (control & (C_LEFT | C_RIGHT))
    {
    case C_LEFT:
        desiredAngle = lockAngle;
        break;
    case C_RIGHT:
        desiredAngle = -lockAngle;
        break;
    default:; // nothing
    }
}

bool Car::onKey(SDL_Event &event)
{
    if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
        return false;

    char isDown = event.type == SDL_KEYDOWN ? C_ALL : C_NONE;

    switch (event.key.keysym.sym)
    {
    case SDLK_w:
        control = (control & (C_ALL ^ C_UP)) | (C_UP & isDown);
        break;
    case SDLK_s:
        control = (control & (C_ALL ^ C_DOWN)) | (C_DOWN & isDown);
        break;
    case SDLK_a:
        control = (control & (C_ALL ^ C_LEFT)) | (C_LEFT & isDown);
        break;
    case SDLK_d:
        control = (control & (C_ALL ^ C_RIGHT)) | (C_RIGHT & isDown);
        break;
    default:
        return false;
    }

    return true;
}
