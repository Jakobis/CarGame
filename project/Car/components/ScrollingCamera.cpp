//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include "ScrollingCamera.hpp"
#include "PhysicsComponent.hpp"
#include "../CarGame.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

using namespace glm;

ScrollingCamera::ScrollingCamera(GameObject *gameObject)
    : Component(gameObject)
{
    cameraSize = sre::Renderer::instance->getWindowSize().y;
    camera.setOrthographicProjection(cameraSize, -1, 1);
}

sre::Camera &ScrollingCamera::getCamera()
{
    return camera;
}

void ScrollingCamera::update(float deltaTime)
{
    auto position = followObject->getPosition();

    position.x += offset.x;
    position.y += offset.y;

    // Compromise for our sanity: Same zoom level for all screen sizes, bigger screen == bigger view
    if (cameraSize != sre::Renderer::instance->getWindowSize().y)
    {
        cameraSize = sre::Renderer::instance->getWindowSize().y;
        camera.setOrthographicProjection(cameraSize, -1, 1);
    }

    gameObject->setPosition(position);
    vec3 eye(position, 0);
    vec3 at(position, -1);
    vec3 up(0, 1, 0);
    camera.lookAt(eye, at, up);
}

void ScrollingCamera::setFollowObject(std::shared_ptr<GameObject> followObject, glm::vec2 offset)
{
    this->followObject = followObject;
    this->offset = offset;
}

ImVec2 ScrollingCamera::getScreenPosition(glm::vec2 position)
{
    auto x = (glm::vec2)sre::Renderer::instance->getWindowSize();
    x = (glm::vec2(camera.getPosition()) - position + glm::vec2(-x.x, x.y)) * 0.5f;
    return ImVec2(-x.x, x.y);
}

bool ScrollingCamera::isInView(glm::vec2 position)
{
    // Check position is to right and below of top left corner
    auto ps1 = camera.screenPointToRay({0, 0});
    if (position.x < ps1[0].x || position.y < ps1[0].y)
        return false;
    // Check position is above and left of bottom right corner
    auto ps2 = camera.screenPointToRay((glm::vec2)sre::Renderer::instance->getWindowSize());
    if (position.x > ps2[0].x || position.y > ps2[0].y)
        return false;
    return true;
}
