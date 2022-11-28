#include "Car.hpp"
#include "CarGame.hpp"
#include "PhysicsComponent.hpp"
#include "SpriteComponent.hpp"
#include <iostream>

Tire::Tire(std::shared_ptr<GameObject> gameObject, sre::Sprite *sprite)
{
    this->gameObject = gameObject;
    auto s = gameObject->addComponent<SpriteComponent>();
    s->setSprite(*sprite);
    phys = gameObject->addComponent<PhysicsComponent>();
    phys->initBox(b2_dynamicBody, {10 / 10, 20 / 10}, {5 / 10, 10 / 10}, 1);
    phys->setSensor(true);
    currentTraction = 0.5;
}

void Tire::setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse)
{
    this->maxForwardSpeed = maxForwardSpeed;
    this->maxBackwardSpeed = maxBackwardSpeed;
    this->maxDriveForce = maxDriveForce;
    this->maxLateralImpulse = maxLateralImpulse;
}

void Tire::updateFriction()
{
    auto angle = phys->getAngle();
    float ratio = 2;
    glm::vec2 traction(-glm::sin(angle), glm::cos(angle));
    auto impulse = 100 * currentTraction * -phys->getLinearImpulse();
    auto forwardImpulse = (impulse / ratio) * glm::vec2(-glm::sin(angle), glm::cos(angle));
    auto lateralImpulse = (impulse)*glm::vec2(glm::cos(angle), glm::sin(angle));
    phys->addForce(impulse);
    std::cout << "fw imp " << forwardImpulse.x << " " << forwardImpulse.y << "\n";
    std::cout << "lt imp " << lateralImpulse.x << " " << lateralImpulse.y << "\n";
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
    auto forceVec = glm::vec2(force * -glm::sin(rotation), force * glm::cos(rotation));
    phys->addForce(forceVec);
}

Car::Car(GameObject *gameObject) : Component(gameObject)
{
    this->gameObject = gameObject;
    phys = gameObject->addComponent<PhysicsComponent>();
    glm::vec2 size(30 / 10, 80 / 10);
    phys->initBox(b2_dynamicBody, size, {15 / 10, 40 / 10}, 10);
    phys->setAngularDamping(1);
}

void Car::initTires(sre::Sprite *tireSprite)
{
    float maxForwardSpeed = 250;
    float maxBackwardSpeed = -40;
    float backTireMaxDriveForce = 30000;
    float frontTireMaxDriveForce = 50000;
    float backTireMaxLateralImpulse = 8.5;
    float frontTireMaxLateralImpulse = 7.5;

    b2RevoluteJointDef jointDef;
    jointDef.enableLimit = true;
    jointDef.lowerAngle = 0;
    jointDef.upperAngle = 0;
    jointDef.localAnchorB.SetZero(); // center of tire

    // back left tire
    auto tire = std::make_shared<Tire>(CarGame::instance->createGameObject(), tireSprite);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
    jointDef.localAnchorA.Set(-3, -4);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // back right tire
    tire = std::make_shared<Tire>(CarGame::instance->createGameObject(), tireSprite);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
    jointDef.localAnchorA.Set(3, -4);
    phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // front left
    tire = std::make_shared<Tire>(CarGame::instance->createGameObject(), tireSprite);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
    jointDef.localAnchorA.Set(-3, 3);
    flJoint = (b2RevoluteJoint *)phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
    tires.push_back(tire);

    // front right
    tire = std::make_shared<Tire>(CarGame::instance->createGameObject(), tireSprite);
    tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
    jointDef.localAnchorA.Set(3, 3);
    frJoint = (b2RevoluteJoint *)phys->initJoint(tire->gameObject->getComponent<PhysicsComponent>(), &jointDef);
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
    float turnSpeedPerSec = glm::radians(100.0); // from lock to lock in 0.5 sec
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
