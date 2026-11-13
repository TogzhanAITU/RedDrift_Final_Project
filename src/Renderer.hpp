#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <cstddef>
#include <string>
#include <vector>

class Level;

class Renderer {
public:
    explicit Renderer(const std::filesystem::path& shaderDirectory);
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    void draw(const Level& game, float animationTime, int width, int height, glm::vec2 cursor = {-1,-1});
private:
    struct Vertex { glm::vec2 position; glm::vec4 color; };
    GLuint program = 0, vao = 0, vbo = 0;
    std::vector<Vertex> vertices;
    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color);
    void rect(float x, float y, float w, float h, glm::vec4 color);
    void disc(glm::vec2 center, float rx, float ry, glm::vec4 color, int sides = 48);
    void line(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color);
    void text(const std::string& value, float x, float y, float scale, glm::vec4 color);
    void centeredText(const std::string& value,float y,float scale,glm::vec4 color);
    void hud(const Level& game);
    void menu(const Level& game,glm::vec2 cursor);
    void endCard(const Level& game);
    void submit();
    void ship(glm::vec2 p, float scale, float tilt, bool enemy, bool heavy, float time, bool boost, float engine = 1.0f, glm::vec2 maneuverJets = {0,0}, bool reverseFacing = false);
    void martianFighter(glm::vec2 position,float scale,float tilt,float time);
    void marsFinale(const Level& game);
    void applyMarsCamera(std::size_t first,const Level& game);
    void effects(const Level& game);
    void departure(const Level& game);
    void moon(glm::vec2 center, float radius);
    void moonEncounter(const Level& game);
    void moonLanding(const Level& game);
    void moonTakeoff(const Level& game);
    void station(glm::vec2 moonCenter, float radius, float ground, float alpha);
    void workshop(const Level& game, glm::vec2 cursor);
};
