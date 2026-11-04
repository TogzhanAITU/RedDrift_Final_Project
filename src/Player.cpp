#include "Player.hpp"
#include "Level.hpp"
#include <algorithm>
#include <iostream>

Player::Player(glm::vec3 startPos) {
    position = startPos;
    velocity = glm::vec3(0.0f);
    size = glm::vec3(0.6f, 1.2f, 0.6f);
}

void Player::jump() {
    if (isGrounded) {
        float jumpForce = 8.0f;
        velocity.y = (gravityDirection >= 0) ? jumpForce : -jumpForce;
        isGrounded = false;
    }
}

void Player::update(float dt, const std::vector<Platform>& platforms, float levelHeight) {
    if (enableGravity) {
        float gAccel = 20.0f * gravityDirection;
        velocity.y -= gAccel * dt;
    }

    position.x += velocity.x * dt;

    for (const auto& plat : platforms) {
        glm::vec3 pMin = position - size * 0.5f;
        glm::vec3 pMax = position + size * 0.5f;
        glm::vec3 platMin = plat.position - plat.size * 0.5f;
        glm::vec3 platMax = plat.position + plat.size * 0.5f;

        bool collisionX = pMax.x >= platMin.x && pMin.x <= platMax.x;
        bool isHittingSide = pMax.y > platMin.y + 0.1f && pMin.y < platMax.y - 0.1f;
        bool collisionZ = pMax.z >= platMin.z && pMin.z <= platMax.z;

        if (collisionX && isHittingSide && collisionZ) {
            if (velocity.x > 0.0f) {
                position.x = platMin.x - size.x * 0.5f;
            } else if (velocity.x < 0.0f) {
                position.x = platMax.x + size.x * 0.5f;
            }
            velocity.x = 0.0f;
        }
    }

    float oldY = position.y;
    position.y += velocity.y * dt;
    isGrounded = false;

    for (const auto& plat : platforms) {
        glm::vec3 pMin = position - size * 0.5f;
        glm::vec3 pMax = position + size * 0.5f;
        glm::vec3 platMin = plat.position - plat.size * 0.5f;
        glm::vec3 platMax = plat.position + plat.size * 0.5f;

        bool collisionX = pMax.x >= platMin.x && pMin.x <= platMax.x;
        bool collisionY = pMax.y >= platMin.y && pMin.y <= platMax.y;
        bool collisionZ = pMax.z >= platMin.z && pMin.z <= platMax.z;

        if (collisionX && collisionY && collisionZ) {
            float oldBottom = oldY - size.y * 0.5f;
            float oldTop = oldY + size.y * 0.5f;

            if (gravityDirection > 0) {
                if (oldBottom >= platMax.y - 0.1f && velocity.y <= 0.0f) {
                    position.y = platMax.y + size.y * 0.5f;
                    velocity.y = 0.0f;
                    isGrounded = true;
                } else if (oldTop <= platMin.y + 0.1f && velocity.y > 0.0f) {
                    position.y = platMin.y - size.y * 0.5f;
                    velocity.y = 0.0f;
                }
            } else {
                if (oldTop <= platMin.y + 0.1f && velocity.y >= 0.0f) {
                    position.y = platMin.y - size.y * 0.5f;
                    velocity.y = 0.0f;
                    isGrounded = true;
                } else if (oldBottom >= platMax.y - 0.1f && velocity.y < 0.0f) {
                    position.y = platMax.y + size.y * 0.5f;
                    velocity.y = 0.0f;
                }
            }
        }
    }

    float floorLevel = -10.0f; 
    if (position.y - size.y * 0.5f <= floorLevel && gravityDirection > 0) {
        position.y = floorLevel + size.y * 0.5f;
        velocity.y = 0.0f;
        isGrounded = true;
    }
}