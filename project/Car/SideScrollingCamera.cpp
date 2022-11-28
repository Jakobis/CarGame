//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include "SideScrollingCamera.hpp"
#include "CarGame.hpp"

using namespace glm;

SideScrollingCamera::SideScrollingCamera(GameObject *gameObject)
    : Component(gameObject)
{
    camera.setOrthographicProjection(CarGame::windowSize.y, -1, 1);
}

sre::Camera &SideScrollingCamera::getCamera()
{
    return camera;
}

void SideScrollingCamera::update(float deltaTime)
{
    auto position = followObject->getPosition();

    position.x += offset.x;
    position.y += offset.y;

    gameObject->setPosition(position);
    vec3 eye(position, 0);
    vec3 at(position, -1);
    vec3 up(0, 1, 0);
    camera.lookAt(eye, at, up);
}

void SideScrollingCamera::setFollowObject(std::shared_ptr<GameObject> followObject, glm::vec2 offset)
{
    this->followObject = followObject;
    this->offset = offset;
}
