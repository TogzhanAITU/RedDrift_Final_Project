#include "Player.hpp"
#include <algorithm>
#include <cmath>

void Player::update(float dt, const FlightInput& input, float cameraX) {
    invulnerability = std::max(0.0f, invulnerability - dt);
    fireCooldown = std::max(0.0f, fireCooldown - dt);
    missileCooldown = std::max(0.0f, missileCooldown - dt);
    glm::vec2 direction = input.direction;
    if (glm::length(direction) > 1.0f) direction = glm::normalize(direction);
    const glm::vec2 jets(std::max(0.0f,direction.y),std::max(0.0f,-direction.y));
    maneuverJets=glm::mix(maneuverJets,jets,1.0f-std::exp(-20.0f*dt));
    if(!input.laser) laserExhausted=false;
    laserFiring=input.laser && !laserExhausted && energy>=LaserEnergyPerSecond*dt;
    if(input.laser && !laserFiring) { laserExhausted=true; energy=0; }
    boosting = input.boost && energy > (boosting ? 0.0f : 15.0f);
    const float regeneration=boosting?0.0f:input.laser?0.0f:19.0f;
    const float drain=(boosting?32.0f:0.0f)+(laserFiring?LaserEnergyPerSecond:0.0f);
    energy = std::clamp(energy + (regeneration-drain)*dt, 0.0f, maxEnergy);
    if(input.laser && energy<=0) laserExhausted=true;
    const float thrust = boosting ? 12.0f : 7.0f;
    const glm::vec2 target(CruiseSpeed + direction.x * thrust, direction.y * thrust);
    velocity += (target - velocity) * (1.0f - std::exp(-6.0f * dt));
    position += velocity * dt;
    const float minX = cameraX - 11.0f + size.x * 0.5f;
    const float maxX = cameraX + 10.0f - size.x * 0.5f;
    const float maxY = Corridor - size.y * 0.5f;
    if (position.x < minX) { position.x = minX; velocity.x = CruiseSpeed; }
    if (position.x > maxX) { position.x = maxX; velocity.x = CruiseSpeed; }
    if (position.y < -maxY) { position.y = -maxY; velocity.y = 0.0f; }
    if (position.y > maxY) { position.y = maxY; velocity.y = 0.0f; }
}
bool Player::damage(int amount) {
    if (invulnerability > 0.0f || health <= 0) return false;
    health = std::max(0, health - amount);
    invulnerability = 1.1f;
    return true;
}
