#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/glm.hpp>
#include <vector>

struct Platform;

class Player {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 size;

    bool enableGravity = true;
    int gravityDirection = 1;
    bool enableInertia = true;
    bool isGrounded = false;

    Player(glm::vec3 startPos);
    void update(float dt, const std::vector<Platform>& platforms, float levelHeight);
    void jump();
};

#endif