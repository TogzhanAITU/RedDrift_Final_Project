#include "Level.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

bool Level::loadFromFile(const std::string& filepath) {
    platforms.clear();
    std::string fullPath = std::string(PROJECT_SOURCE_DIR) + "/" + filepath;

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file: " << fullPath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.rfind("level height:", 0) == 0 || line.find("level height") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                height = std::stof(line.substr(colon + 1));
            }
            continue;
        }

        Platform p;
        p.size = glm::vec3(3.0f, 0.5f, 1.0f);

        size_t posStart = line.find("positions:(");
        if (posStart != std::string::npos) {
            size_t posEnd = line.find(')', posStart);
            std::string coords = line.substr(posStart + 11, posEnd - (posStart + 11));
            std::stringstream ss(coords);
            char comma;
            ss >> p.position.x >> comma >> p.position.y >> comma >> p.position.z;
        }

        size_t sizeStart = line.find("size:(");
        if (sizeStart != std::string::npos) {
            size_t sizeEnd = line.find(')', sizeStart);
            std::string szCoords = line.substr(sizeStart + 6, sizeEnd - (sizeStart + 6));
            std::stringstream ssSz(szCoords);
            char comma;
            ssSz >> p.size.x >> comma >> p.size.y >> comma >> p.size.z;
        }

        size_t typeStart = line.find("type:(");
        if (typeStart != std::string::npos) {
            size_t typeEnd = line.find(')', typeStart);
            p.type = line.substr(typeStart + 6, typeEnd - (typeStart + 6));
        }

        size_t matStart = line.find("material:(");
        if (matStart != std::string::npos) {
            size_t matEnd = line.find(')', matStart);
            p.material = line.substr(matStart + 10, matEnd - (matStart + 10));
        }

        size_t assetStart = line.find("assetname:(");
        if (assetStart != std::string::npos) {
            size_t assetEnd = line.find(')', assetStart);
            p.assetname = line.substr(assetStart + 11, assetEnd - (assetStart + 11));
        }

        platforms.push_back(p);
    }
    file.close();
    return true;
}