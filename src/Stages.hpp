#pragma once
#include <array>

enum class Environment { Earth, Moon, Space, Mars };
struct StageDefinition {
    const char* title;
    const char* description;
    Environment environment;
    float length;
    int enemyTier;
    float waveInterval, rockInterval;
    float enemyStart = 0.0f;
};
inline constexpr std::array<StageDefinition, 6> Stages{{
    {"EARTH STRATOSPHERE", "DEBRIS AND ROCKS", Environment::Earth, 200, 0, 0, 2.0f},
    {"NEAR THE MOON", "LIGHT ENEMIES AFTER 50%", Environment::Moon, 240, 1, 3.2f, 1.6f, 0.5f},
    {"MOON ATMOSPHERE", "SELENE DEPARTURE / SCOUTS", Environment::Moon, 280, 1, 3.4f, 3.0f},
    {"ROAD TO MARS", "ENEMY SHIPS AND ROCKS", Environment::Space, 280, 1, 2.8f, 2.2f},
    {"ROAD TO MARS II", "HEAVY ENEMY PATROLS", Environment::Space, 320, 2, 2.4f, 2.5f},
    {"MARS STRATOSPHERE", "ELITE FLEET AND DEBRIS", Environment::Mars, 360, 3, 2.0f, 1.9f}
}};
