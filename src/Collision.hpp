#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

namespace Collision {
inline bool overlap(glm::vec2 a, glm::vec2 halfA, glm::vec2 b, glm::vec2 halfB) {
    const glm::vec2 d = glm::abs(a - b);
    return d.x <= halfA.x + halfB.x && d.y <= halfA.y + halfB.y;
}
inline bool sweptHit(glm::vec2 from, glm::vec2 to, glm::vec2 center, glm::vec2 half, float* contact = nullptr) {
    float enter = 0.0f, leave = 1.0f;
    for (int axis = 0; axis < 2; ++axis) {
        const float delta = to[axis] - from[axis];
        if (std::abs(delta) < 0.00001f) {
            if (from[axis] < center[axis] - half[axis] || from[axis] > center[axis] + half[axis]) return false;
        } else {
            float a = (center[axis] - half[axis] - from[axis]) / delta;
            float b = (center[axis] + half[axis] - from[axis]) / delta;
            if (a > b) std::swap(a, b);
            enter = std::max(enter, a); leave = std::min(leave, b);
            if (enter > leave) return false;
        }
    }
    if (contact) *contact = enter;
    return true;
}
inline bool circleHit(glm::vec2 p, glm::vec2 half, glm::vec2 center,float radius) {
    const glm::vec2 closest = glm::clamp(center, p - half, p + half);
    const auto delta=closest-center;
    return glm::dot(delta,delta)<=radius*radius;
}
}
