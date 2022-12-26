#pragma once
#include <glm/glm.hpp>

struct FrameTiming {
    float event;
    float update;
    float render;

    FrameTiming(glm::vec3 frame) {
        event = frame[0];
        update = frame[1];
        render = frame[2];
    }

    float delta() {
        return (event + update + render) / 1000;
    }
};