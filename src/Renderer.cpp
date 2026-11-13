#include "Renderer.hpp"
#include "Level.hpp"
#include "RenderStyle.hpp"
#include <algorithm>
#include <cmath>

#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace RenderStyle;

namespace {
std::string read(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open shader: " + path.string());
    std::ostringstream out; out << file.rdbuf(); return out.str();
}
GLuint compile(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* data = source.c_str(); glShaderSource(shader, 1, &data, nullptr); glCompileShader(shader);
    GLint ok; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]{}; glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader); throw std::runtime_error(log);
    }
    return shader;
}
}

Renderer::Renderer(const std::filesystem::path& shaderDirectory) {
    const auto vertexSource = read(shaderDirectory / "vertex.glsl");
    const auto fragmentSource = read(shaderDirectory / "fragment.glsl");
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource), fs = 0;
    try { fs = compile(GL_FRAGMENT_SHADER, fragmentSource); }
    catch (...) { glDeleteShader(vs); throw; }
    program = glCreateProgram(); glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
    glDeleteShader(vs); glDeleteShader(fs);
    GLint ok; glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]{}; glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        glDeleteProgram(program); throw std::runtime_error(log);
    }
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec2))); glEnableVertexAttribArray(1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    vertices.reserve(100000);
}

Renderer::~Renderer() { glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao); glDeleteProgram(program); }

void Renderer::draw(const Level& game, float animationTime, int width, int height, glm::vec2 cursor) {
    glViewport(0, 0, width, height);
    glClearColor(0.015f, 0.023f, 0.045f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    if (width <= 0 || height <= 0) return;

    const float fit = std::min(width / 1280.0f, height / 720.0f);
    glViewport(static_cast<int>((width - 1280 * fit) / 2),
               static_cast<int>((height - 720 * fit) / 2),
               static_cast<int>(1280 * fit), static_cast<int>(720 * fit));
    vertices.clear();
    rect(0, 0, 1280, 720, {0.025f, 0.035f, 0.055f, 1});

    const auto world = [&](glm::vec2 position) {
        return glm::vec2(640 + (position.x - game.cameraX) * 36, 370 - position.y * 36);
    };
    if (!game.cinematicActive()) {
        line({0, 104}, {1280, 104}, 1, Muted);
        line({0, 636}, {1280, 636}, 1, Muted);
        for (const auto& rock : game.asteroids) {
            const auto position = world(rock.position);
            disc(position, rock.radius * 36, rock.radius * 36, {0.45f, 0.4f, 0.35f, 1}, 8);
        }
        for (const auto& pickup : game.pickups) {
            const auto position = world(pickup.position);
            rect(position.x - 9, position.y - 9, 18, 18, Cyan);
        }
        for (const auto& enemy : game.enemies) {
            const auto position = world(enemy.position);
            ship(position, enemy.heavy ? 42.0f : 33.0f, 0, true, enemy.heavy, animationTime, false);
            rect(position.x - 20, position.y - 30, 40, 3, Muted);
            const float maxHealth = enemy.heavy ? (game.definition().enemyTier == 3 ? 5.0f : 4.0f) : 2.0f;
            rect(position.x - 20, position.y - 30, 40 * std::clamp(enemy.health / maxHealth, 0.0f, 1.0f), 3, Orange);
        }
        for (const auto& bullet : game.bullets) {
            const auto position = world(bullet.position);
            const auto color = bullet.hostile ? Orange : bullet.homing ? White : Cyan;
            rect(position.x - 4, position.y - 2, bullet.homing ? 12.0f : 8.0f, 4, color);
        }
        if (game.beamActive) line(world(game.beamStart), world(game.beamEnd), 3, Cyan);
        if (game.missileTargetId != 0) {
            const auto position = world(game.missileLockPosition);
            line(position - glm::vec2(16, 0), position + glm::vec2(16, 0), 2, Orange);
            line(position - glm::vec2(0, 16), position + glm::vec2(0, 16), 2, Orange);
            text("LOCK " + std::to_string(static_cast<int>(100 * game.missileLockTime / game.profile.lockDuration())) + "%",
                 position.x - 30, position.y + 22, 1.3f, White);
        }
        if (game.state != MissionState::Ready && game.state != MissionState::Lost &&
            (game.player.invulnerability <= 0 || static_cast<int>(animationTime * 14) % 2 == 0)) {
            ship(world(game.player.position), 36, -game.player.velocity.y * 0.025f,
                 false, false, animationTime, game.player.boosting);
        }
        effects(game);
        if (game.state != MissionState::Ready) hud(game);
    }

    const auto scene = game.sceneState();
    if (scene == MissionState::Launching) departure(game);
    if (scene == MissionState::Encounter) moonEncounter(game);
    if (scene == MissionState::Landing) moonLanding(game);
    if (scene == MissionState::LiftOff) moonTakeoff(game);
    const bool finale = scene == MissionState::MarsFinale || (game.state == MissionState::Won && game.levelIndex == 5);
    if (finale) {
        marsFinale(game);
        endCard(game);
    }
    if (game.state != MissionState::Flying &&
        (!game.cinematicActive() || game.state == MissionState::Paused) &&
        !(finale && game.state == MissionState::Won)) {
        menu(game, cursor);
    }
    text("SIMPLIFIED VISUAL BASELINE", 16, 704, 1, Muted);
    submit();
}

void Renderer::submit() {
    glUseProgram(program);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));
}
