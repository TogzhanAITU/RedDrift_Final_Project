#pragma once
#include <glm/glm.hpp>

struct FlightInput {
    glm::vec2 direction{0.0f};
    bool fire = false;
    bool boost = false;
    bool skipCinematic = false;
    bool missile = false;
    bool laser = false;
};

class Player {
public:
    static constexpr float CruiseSpeed = 8.0f;
    static constexpr float Corridor = 7.4f;
    static constexpr float LaserEnergyPerSecond = 30.0f;
    glm::vec2 position{0.0f};
    glm::vec2 velocity{CruiseSpeed, 0.0f};
    glm::vec2 size{1.7f, 0.85f};
    int health = 100;
    int maxHull = 100;
    float energy = 100.0f;
    float maxEnergy = 100.0f;
    float invulnerability = 0.0f;
    float fireCooldown = 0.0f;
    float missileCooldown = 0.0f;
    bool boosting = false;
    bool laserFiring = false, laserExhausted = false;
    glm::vec2 maneuverJets{0.0f};
    void update(float dt, const FlightInput& input, float cameraX);
    bool damage(int amount);
};
