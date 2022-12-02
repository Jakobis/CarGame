#pragma once

#include "Box2D/Common/b2Math.h"
#include "Box2D/Collision/Shapes/b2PolygonShape.h"
#include "Box2D/Collision/Shapes/b2CircleShape.h"
#include "Box2D/Dynamics/b2Body.h"
#include "Box2D/Dynamics/Joints/b2RevoluteJoint.h"
#include "Component.hpp"

class PhysicsComponent : public Component
{
public:
    explicit PhysicsComponent(GameObject *gameObject);
    virtual ~PhysicsComponent();
    void initCircle(b2BodyType type, float radius, glm::vec2 center, float density);
    void initBox(b2BodyType type, glm::vec2 size, glm::vec2 center, float density);
    b2Joint *initJoint(std::shared_ptr<PhysicsComponent> other, b2JointDef *jointDef);

    void addForce(glm::vec2 force); // Force gradually affects the velocity over time

    void addImpulse(glm::vec2 impulse); // Instantly affects velocity

    void addAngularImpulse(float impulse);

    float getAngularVelocity();

    float getAngle();

    float getIntertia();

    void setLinearVelocity(glm::vec2 velocity);

    void setAngularDamping(float anglularDamping);

    void addTorque(float torque);

    glm::vec2 getLinearVelocity();
    glm::vec2 getLinearImpulse();

    glm::vec2 getForwardVelocity();

    glm::vec2 getLateralImpulse();
    glm::vec2 getDirectionVector();

    bool isSensor();

    void setSensor(bool enabled);

    b2Body *body = nullptr;

private:
    b2PolygonShape *polygon = nullptr;
    b2CircleShape *circle = nullptr;
    b2Shape::Type shapeType;
    b2Fixture *fixture = nullptr;
    b2Joint *joint = nullptr;
    b2BodyType rbType;
    std::vector<PhysicsComponent *> collidingBodies;
    b2World *world = nullptr;
    friend class CarGame;
};
