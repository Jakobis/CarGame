#pragma once

#include "sre/Camera.hpp"
#include "glm/glm.hpp"
#include "Component.hpp"

class SideScrollingCamera : public Component
{
public:
    explicit SideScrollingCamera(GameObject *gameObject);

    void update(float deltaTime) override;

    void setFollowObject(std::shared_ptr<GameObject> followObject, glm::vec2 offset);

    sre::Camera &getCamera();

    bool isInView(glm::vec2 position);

    ImVec2 getScreenPosition(glm::vec2 position);

private:
    sre::Camera camera;
    std::shared_ptr<GameObject> followObject;
    glm::vec2 offset;
    float cameraSize;
};
