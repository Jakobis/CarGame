//
// Created by Morten Nobel-Jørgensen on 10/10/2017.
//

#include "SideScrollingCamera.hpp"
#include "PhysicsComponent.hpp"
#include "CarGame.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

using namespace glm;

SideScrollingCamera::SideScrollingCamera(GameObject *gameObject)
    : Component(gameObject)
{
    cameraSize = sre::Renderer::instance->getWindowSize().y;
    camera.setOrthographicProjection(cameraSize, -1, 1);
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

    // Compromise for our sanity: Same zoom level for all screen sizes, bigger screen == bigger win
    if (cameraSize != sre::Renderer::instance->getWindowSize().y)
    {
        cameraSize = sre::Renderer::instance->getWindowSize().y;
        camera.setOrthographicProjection(cameraSize, -1, 1);
    }

    // auto phys = followObject->getComponent<PhysicsComponent>();
    // if (phys != nullptr)
    // {
    //     auto lv = phys->getLinearVelocity();
    //     auto len = lv.x * lv.x + lv.y * lv.y;
    //     camera.setOrthographicProjection(CarGame::windowSize.y * ((len * 0.00001) + 1), -1, 1);
    // }

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

ImVec2 SideScrollingCamera::getScreenPosition(glm::vec2 position)
{
    auto x = (glm::vec2)sre::Renderer::instance->getWindowSize();
    x = (glm::vec2(camera.getPosition()) - position + glm::vec2(-x.x, x.y)) * 0.5f;
    return ImVec2(-x.x, x.y);
}

bool SideScrollingCamera::isInView(glm::vec2 position)
{
    auto ps1 = camera.screenPointToRay({0, 0});
    if (position.x < ps1[0].x || position.y < ps1[0].y)
        return false;
    auto ps2 = camera.screenPointToRay((glm::vec2)sre::Renderer::instance->getWindowSize());
    if (position.x > ps2[0].x || position.y > ps2[0].y)
        return false;
    // std::array<glm::vec2, 2> pos({glm::vec2(ps1[0].x, ps1[0].y), glm::vec2(ps2[0].x, ps2[0].y)});
    // std::cout << pos[0].x << " " << pos[0].y << ", " << pos[1].x << " " << pos[1].y << "\n";
    return true;
}
