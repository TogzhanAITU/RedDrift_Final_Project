#include "Renderer.hpp"
#include "Level.hpp"
#include "RenderStyle.hpp"
#include "LaunchScene.hpp"
#include "MoonScene.hpp"
#include "MarsScene.hpp"

using namespace RenderStyle;

void Renderer::departure(const Level& game) {
    const auto frame = LaunchScene::sample(game.launchTime);
    centeredText("DEPARTURE", 140, 3, White);
    rect(120, 300, 210, 150, Muted);
    ship({frame.screenX, 370}, 72 * frame.zoom, 0, false, false, game.launchTime, false);
    centeredText("ENTER / CLICK  SKIP", 570, 1.5f, Muted);
}

void Renderer::moonEncounter(const Level& game) {
    const auto frame = MoonScene::encounter(game.encounterTime);
    centeredText("MOON APPROACH", 140, 3, White);
    ship(frame.enemy, frame.enemyScale, 0, true, true, game.encounterTime, false);
    if (frame.playerReveal > 0) ship(frame.player, frame.playerScale, 0, false, false, game.encounterTime, false);
    centeredText("ENTER / CLICK  SKIP", 570, 1.5f, Muted);
}

void Renderer::station(glm::vec2, float, float ground, float alpha) {
    rect(650, ground - 22, 300, 22, {Muted.r, Muted.g, Muted.b, alpha});
}

void Renderer::moonLanding(const Level& game) {
    const auto frame = MoonScene::landing(game.landingTime, game.landingStartScreen, game.landingStartTilt);
    centeredText("SELENE LANDING", 140, 3, White);
    station(frame.moon, frame.radius, 445, 1);
    ship(frame.player, frame.scale, frame.tilt, false, false, game.landingTime, false);
    centeredText("HOLD ENTER  SKIP", 570, 1.5f, Muted);
}

void Renderer::moonTakeoff(const Level& game) {
    const auto frame = MoonScene::takeoff(game.takeoffTime);
    centeredText("SELENE TAKEOFF", 140, 3, White);
    station(frame.moon, frame.radius, 445, 1);
    ship(frame.player, frame.scale, frame.tilt, false, false, game.takeoffTime, false);
    centeredText("ENTER / CLICK  SKIP", 570, 1.5f, Muted);
}

void Renderer::applyMarsCamera(std::size_t first, const Level& game) {
    const auto camera = MarsScene::camera(game.finaleTime);
    for (std::size_t index = first; index < vertices.size(); ++index) {
        vertices[index].position = camera.project(vertices[index].position);
    }
}

void Renderer::marsFinale(const Level& game) {
    centeredText("MARS ARRIVAL", 140, 3, White);
    const std::size_t first = vertices.size();
    for (int index = 0; index < MarsScene::Pursuers; ++index) {
        if (!game.finaleDestroyed[index]) {
            ship(MarsScene::pursuer(game.finaleTime, index), 42, 0, true, true, game.finaleTime, false);
        }
    }
    ship(MarsScene::cruiser(game.finaleTime, game.finaleStartScreen), 36, 0, false, false, game.finaleTime, false);
    for (int index = 0; index < MarsScene::Fighters; ++index) {
        martianFighter(MarsScene::fighter(game.finaleTime, index), 24, 0, game.finaleTime);
    }
    applyMarsCamera(first, game);
    centeredText("HOLD ENTER  SKIP", 570, 1.5f, Muted);
}

void Renderer::effects(const Level& game) {
    for (const auto& particle : game.particles) {
        const auto position = glm::vec2(640 + (particle.position.x - game.cameraX) * 36, 370 - particle.position.y * 36);
        rect(position.x - 1, position.y - 1, 2, 2,
             {particle.color, particle.life / particle.total});
    }
}
