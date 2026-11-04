#ifndef LEVEL_HPP
#define LEVEL_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Platform {
    glm::vec3 position;
    glm::vec3 size;
    std::string type;
    std::string material;
    std::string assetname;
};

class Level {
public:
    float height = 20.0f;
    std::vector<Platform> platforms;

    bool loadFromFile(const std::string& filepath);
};

#endif