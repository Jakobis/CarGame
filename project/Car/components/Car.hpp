#pragma once
#include "../GameObject.hpp"
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
    Tire(const std::shared_ptr<GameObject> &gameObject, sre::Sprite *sprite, bool isFrontTire);
    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse, float currentTraction, float dragRatio);
    void updateFriction();
    void updateDrive(char control);

private:
    std::shared_ptr<PhysicsComponent> phys;
    float maxForwardSpeed;
    float maxBackwardSpeed;
    float maxDriveForce;
    float maxLateralImpulse;
    float currentTraction;
    float dragRatio;
    float currentEngineSpeed; // Between 0-1, describes how fast the engine is revving
    bool isFrontTire;
    friend class Car;
};

class Car : public Component
{
public:
    explicit Car(GameObject *gameObject);
    void initTires(sre::Sprite *tireSprite);
    void update(float deltaTime) override;
    bool onKey(SDL_Event &event) override;
    void onCollisionStart(PhysicsComponent *comp) override;
    std::function<void()> endGame;

private:
    GameObject *gameObject;
    std::shared_ptr<PhysicsComponent> phys;
    std::vector<std::shared_ptr<Tire>> tires;
    b2RevoluteJoint *flJoint;
    b2RevoluteJoint *frJoint;
    char control = 0;
    float safeangle = 90;
    float maxHealth = 1000;
    float health = maxHealth;
    float damageSpeedThreshold = 100;
    float maxForwardSpeed = 250;
    float maxBackwardSpeed = -40;
    float backTireMaxDriveForce = 30000;
    float frontTireMaxDriveForce = 50000;
    float backTireMaxLateralImpulse = 8.5;
    float frontTireMaxLateralImpulse = 7.5;
    float currentTraction = 1; // Between 0-1, 0 is slippery and 1 is perfect maneuverablity
    float dragRatio = 0.25;    // Between 0-1, describes how much of the traction translates to drag
    friend class CarGame;
};