#include "Car.hpp"
#include "../CarGame.hpp"
#include "PhysicsComponent.hpp"
#include "SpriteComponent.hpp"
#include <iostream>
#include "math.h"

Tire::Tire(const std::shared_ptr<GameObject> &gameObject, sre::Sprite *sprite, bool isFrontTire)
{
    auto s = gameObject->addComponent<SpriteComponent>();
    s->setSprite(*sprite);
    phys = gameObject->addComponent<PhysicsComponent>();
    phys->initBox(b2_dynamicBody, {10 / 10, 20 / 10}, {0, 0}, 1);
    phys->setSensor(true);
    currentEngineSpeed = 0;
    this->isFrontTire = isFrontTire;
}

void Tire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float currentTraction, float dragRatio)
{
    this->maxForwardSpeed = maxForwardSpeed;
    this->maxBackwardSpeed = maxBackwardSpeed;
    this->maxDriveForce = maxDriveForce;
    this->maxLateralImpulse = maxLateralImpulse;
    this->currentTraction = currentTraction;
    this->dragRatio = dragRatio;
}

void Tire::updateFriction()
{
    auto angle = phys->getAngle();

    // We get components rotated to align with the car
    glm::vec2 forwardTraction(-glm::sin(angle), glm::cos(angle));
    glm::vec2 lateralTraction(forwardTraction.y, -forwardTraction.x);
    // We create traction as an opposite impulse...
    auto impulse = 100 * currentTraction * -phys->getLinearImpulse();
    // ...and split it into sideways and straight traction, where forward traction is lower due to tires spinning
    auto forwardImpulse = glm::dot(forwardTraction, impulse) * forwardTraction * dragRatio;
    auto lateralImpulse = glm::dot(lateralTraction, impulse) * lateralTraction;
    phys->addForce(forwardImpulse + lateralImpulse);
}

void Tire::updateDrive(char control)
{
    float accel = 0.2;
    // find desired speed, simulating an enigne revving up/down/backwards
    float desiredSpeed = 0;
    switch (control & (C_UP | C_DOWN))
    {
    case C_UP:
        desiredSpeed = maxForwardSpeed;
        if (currentEngineSpeed < -1e-2)
            currentEngineSpeed -= currentEngineSpeed * accel;
        else
            currentEngineSpeed += (1 - currentEngineSpeed) * accel;
        break;
    case C_DOWN:
        desiredSpeed = maxBackwardSpeed;
        if (currentEngineSpeed > 1e-2)
            currentEngineSpeed -= currentEngineSpeed * accel;
        else
            currentEngineSpeed += (-1 - currentEngineSpeed) * accel;
        break;
    default:
        if (std::abs(currentEngineSpeed) < 1e-4)
            currentEngineSpeed = 0;
        else
            currentEngineSpeed -= currentEngineSpeed * accel;
        return; // do nothing beyond revving down if movement not desired
    }

    auto vel = phys->getLinearVelocity();
    float currentSpeed = vel.length();
    auto rotation = phys->getAngle();

    // apply necessary force
    float force = 0;
    if (desiredSpeed > currentSpeed)
        force = maxDriveForce;
    else if (desiredSpeed < currentSpeed)
        force = -maxDriveForce;
    else
        return;
    auto forceVec = glm::vec2(force * -glm::sin(rotation), force * glm::cos(rotation)) * std::abs(currentEngineSpeed);
    phys->addForce(forceVec);
}

Car::Car(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;
    phys = gameObject->addComponent<PhysicsComponent>();
    glm::vec2 size(30 / 10, 80 / 10);
    phys->initBox(b2_dynamicBody, size, {0, 0}, 10);
    phys->setAngularDamping(1);
}

void Car::initTires(sre::Sprite *tireSprite)
{
    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0;
    jointDef.upperAngle = 0;
    jointDef.localAnchorB.SetZero(); // center of car

    // back left tire
    auto tireGameObject = CarGame::instance->createGameObject();
    auto tire = std::make_shared<Tire>(tireGameObject, tireSprite, false);
    jointDef.localAnchorA.Set(-3, -4); // Center the tire to back left of car
    phys->initJoint(tireGameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // back right tire
    tireGameObject = CarGame::instance->createGameObject();
    tire = std::make_shared<Tire>(tireGameObject, tireSprite, false);
    jointDef.localAnchorA.Set(3, -4);
    phys->initJoint(tireGameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // front left
    tireGameObject = CarGame::instance->createGameObject();
    tire = std::make_shared<Tire>(tireGameObject, tireSprite, true);
    jointDef.localAnchorA.Set(-3, 3);
    // We save the joint on the car, s.t. we can turn the front tires
    flJoint = (b2RevoluteJoint *)phys->initJoint(tireGameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // front right
    tireGameObject = CarGame::instance->createGameObject();
    tire = std::make_shared<Tire>(tireGameObject, tireSprite, true);
    jointDef.localAnchorA.Set(3, 3);
    frJoint = (b2RevoluteJoint *)phys->initJoint(tireGameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);
}

void Car::update(float deltaTime)
{
    // Update characteristics from debug view
    for (auto tire : tires)
    {
        if (tire->isFrontTire)
        {
            tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse, currentTraction, dragRatio);
        }
        else
        {
            tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse, currentTraction, dragRatio);
        }
        tire->updateFriction();
    }
    for (auto tire : tires)
    {
        tire->updateDrive(control);
    }
    // control steering
    float lockAngle = glm::radians(35.0);
    float turnSpeedPerSec = glm::radians(100.0);
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
    // We add partial rotation based on control and rotation speed
    float angleNow = flJoint->GetJointAngle();
    float angleToTurn = desiredAngle - angleNow;
    angleToTurn = b2Clamp(angleToTurn, -turnPerTimeStep, turnPerTimeStep);
    float newAngle = angleNow + angleToTurn;
    flJoint->SetLimits(newAngle, newAngle);
    frJoint->SetLimits(newAngle, newAngle);
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

float angleBetweenInDegrees(glm::vec2 a, glm::vec2 b)
{
    auto da = glm::normalize(a);
    auto db = glm::normalize(b);
    return glm::acos(glm::dot(da, db)) * 180 / M_PI;
}

void Car::onCollisionStart(PhysicsComponent *comp)
{
    // Get collision speed and angle
    auto phys = gameObject->getComponent<PhysicsComponent>();
    auto vector2Enemy = comp->getGameObject()->getPosition() - gameObject->getPosition();
    auto collisionAngle = angleBetweenInDegrees(vector2Enemy, phys->getDirectionVector());
    auto collisionspeed = glm::length(comp->getLinearImpulse() - phys->getLinearImpulse()) / 1000; // the 1000 is to have more managable numbers
    // Only deal damage to car if high enough speed and outside safe angle
    if (collisionspeed >= damageSpeedThreshold)
    {
        if (collisionAngle <= safeangle)
        {
            return;
        }
        health -= collisionspeed;
        std::cout << "Car crashed" << std::endl;
        if (health <= 0)
        {
            std::cout << "Car has died" << std::endl;
            endGame();
        }
    }
    // If collided with power up, apply power up effect
    auto pow = comp->getGameObject()->getComponent<PowerupComponent>();
    if (pow != nullptr)
    {
        switch (pow->getType())
        {
        case PowerupType::Heal:
            health = maxHealth;
            break;

        default:
            break;
        }
    }
}