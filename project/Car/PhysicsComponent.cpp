//

#include <Box2D/Box2D.h>
#include <iostream>
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"

PhysicsComponent::PhysicsComponent(GameObject *gameObject)
    : Component(gameObject)
{
    world = CarGame::instance->world;
}

PhysicsComponent::~PhysicsComponent()
{
    CarGame::instance->deregisterPhysicsComponent(this);

    delete polygon;
    delete circle;
    if (body != nullptr && fixture != nullptr)
    {
        body->DestroyFixture(fixture);
        fixture = nullptr;
    }
    if (world != nullptr && joint != nullptr)
    {
        world->DestroyJoint(joint);
        joint = nullptr;
    }
    if (world != nullptr && body != nullptr)
    {
        world->DestroyBody(body);
        body = nullptr;
    }
}

void PhysicsComponent::addImpulse(glm::vec2 impulse)
{
    b2Vec2 iForceV{impulse.x, impulse.y};
    body->ApplyLinearImpulse(iForceV, body->GetWorldCenter(), true);
}

void PhysicsComponent::addAngularImpulse(float impulse)
{
    body->ApplyAngularImpulse(impulse, true);
}

float PhysicsComponent::getAngularVelocity()
{
    return body->GetAngularVelocity();
}

float PhysicsComponent::getAngle()
{
    return body->GetAngle();
}

void PhysicsComponent::addForce(glm::vec2 force)
{
    b2Vec2 forceV{force.x, force.y};
    body->ApplyForce(forceV, body->GetWorldCenter(), true);
}

void PhysicsComponent::addTorque(float torque)
{
    body->ApplyTorque(torque, true);
}

glm::vec2 PhysicsComponent::getLinearVelocity()
{
    b2Vec2 v = body->GetLinearVelocity();
    return {v.x, v.y};
}
glm::vec2 PhysicsComponent::getLinearImpulse()
{
    b2Vec2 v = body->GetMass() * body->GetLinearVelocity();
    return {v.x, v.y};
}

glm::vec2 PhysicsComponent::getDirectionVector()
{
    auto angle = getAngle();
    glm::vec2 forwardTraction(-glm::sin(angle), glm::cos(angle));
    return forwardTraction;
}

void PhysicsComponent::setLinearVelocity(glm::vec2 velocity)
{
    b2Vec2 v{velocity.x, velocity.y};
    if (velocity != glm::vec2(0, 0))
    {
        body->SetAwake(true);
    }
    body->SetLinearVelocity(v);
}

glm::vec2 PhysicsComponent::getLateralImpulse()
{
    auto b2_currentRightNormal = body->GetWorldVector({1, 0});
    glm::vec2 currentRightNormal(b2_currentRightNormal.x, b2_currentRightNormal.y);
    auto lateralVelocity = glm::dot(currentRightNormal, getLinearVelocity()) * currentRightNormal;
    return body->GetMass() * -lateralVelocity;
}

glm::vec2 PhysicsComponent::getForwardVelocity()
{
    auto b2_currentForwardNormal = body->GetWorldVector({0, 1});
    glm::vec2 currentForwardNormal(b2_currentForwardNormal.x, b2_currentForwardNormal.y);
    return glm::dot(currentForwardNormal, getLinearVelocity()) * currentForwardNormal;
}

float PhysicsComponent::getIntertia()
{
    return body->GetInertia();
}

void PhysicsComponent::setAngularDamping(float angularDamping)
{
    body->SetAngularDamping(angularDamping);
}

void PhysicsComponent::initCircle(b2BodyType type, float radius, glm::vec2 center, float density)
{
    assert(body == nullptr);
    // do init
    shapeType = b2Shape::Type::e_circle;
    b2BodyDef bd;
    bd.type = type;
    rbType = type;
    bd.position = b2Vec2(center.x, center.y);
    body = world->CreateBody(&bd);
    circle = new b2CircleShape();
    circle->m_radius = radius;
    b2FixtureDef fxD;
    fxD.shape = circle;
    fxD.density = density;
    fixture = body->CreateFixture(&fxD);

    CarGame::instance->registerPhysicsComponent(this);
}

void PhysicsComponent::initBox(b2BodyType type, glm::vec2 size, glm::vec2 position, float density)
{
    assert(body == nullptr);
    // do init
    shapeType = b2Shape::Type::e_polygon;
    b2BodyDef bd;
    bd.type = type;
    rbType = type;
    bd.position = b2Vec2(position.x, position.y);
    body = world->CreateBody(&bd);
    polygon = new b2PolygonShape();
    polygon->SetAsBox(size.x, size.y, {0, 0}, 0);
    b2FixtureDef fxD;
    fxD.shape = polygon;
    fxD.density = density;
    fixture = body->CreateFixture(&fxD);

    CarGame::instance->registerPhysicsComponent(this);
}

b2Joint *PhysicsComponent::initJoint(std::shared_ptr<PhysicsComponent> other, b2JointDef *jointDef)
{
    assert(other->joint == nullptr);
    jointDef->bodyA = this->body;
    jointDef->bodyB = other->body;
    other->joint = world->CreateJoint(jointDef);
    return other->joint; // We assign the joint to the "other"
}

bool PhysicsComponent::isSensor()
{
    return fixture->IsSensor();
}

void PhysicsComponent::setSensor(bool enabled)
{
    fixture->SetSensor(enabled);
}
