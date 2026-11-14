#pragma once
#include "Player.hpp"
#include "Progress.hpp"
#include "MarsScene.hpp"
#include "Stages.hpp"
#include <random>
#include <array>
#include <vector>
#include <string>

enum class MissionState { Ready, Workshop, Launching, Encounter, Landing, LiftOff, MarsFinale, Flying, Paused, Won, Lost };
struct Enemy {
    glm::vec2 position, previous;
    float baseY = 0.0f, phase = 0.0f, cooldown = 1.0f;
    float health = 2;
    bool heavy = false;
    unsigned int id = 0;
};
struct Bullet {
    glm::vec2 position, previous, velocity;
    bool hostile = false;
    float life = 3.0f;
    bool homing = false;
    float damage = 1.0f;
    unsigned int targetId = 0;
    float cruiseSpeed = 22.0f;
};
struct Asteroid { glm::vec2 position; float radius, rotation; bool debris = false; float health = 3.0f, heat = 0.0f; };
struct Pickup { glm::vec2 position; };
struct Particle {
    glm::vec2 position, velocity; glm::vec3 color; float life, total;
    bool shard = false;
    glm::vec2 size{0.1f};
    float angle = 0.0f, spin = 0.0f;
};
struct Explosion { glm::vec2 position; float age = 0.0f, scale = 1.0f; };

class Level {
public:
    static constexpr int StageCount = static_cast<int>(Stages.size());
    static constexpr float LaunchDuration = 5.5f;
    static constexpr float EncounterDuration = 6.0f, LandingDuration = 12.0f;
    static constexpr float TakeoffDuration = 7.0f, MissileEnergyCost = 20.0f;
    static constexpr float MarsFinaleDuration = MarsScene::Duration;
    int levelIndex = 0, selectedLevel = 0;
    std::array<bool, StageCount> completed{};
    Profile profile;
    bool progressDirty = false, workshopNotice = false;
    std::string shopMessage;
    int workshopTab = 0;
    MissionState state = MissionState::Ready;
    Player player;
    float cameraX = 4.0f, distance = 0.0f, time = 0.0f;
    float launchTime = 0.0f;
    float encounterTime = 0.0f, landingTime = 0.0f;
    float takeoffTime = 0.0f;
    float finaleTime = 0.0f, finaleSkipHold = 0.0f;
    glm::vec2 finaleStartScreen{496,370};
    float finaleStartTilt = 0.0f;
    std::array<bool,MarsScene::Pursuers> finaleDestroyed{};
    float landingSkipHold = 0.0f;
    glm::vec2 landingStartScreen{496.0f,370.0f};
    float landingStartTilt = 0.0f;
    int score = 0, kills = 0;
    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    std::vector<Asteroid> asteroids;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    std::vector<Explosion> explosions;
    bool beamActive = false;
    glm::vec2 beamStart{0}, beamEnd{0};
    float missileLockTime = 0.0f, missileLaunchFlash = 0.0f;
    unsigned int missileTargetId = 0;
    unsigned int missileSecondTargetId = 0;
    glm::vec2 missileLockPosition{0};
    glm::vec2 missileSecondLockPosition{0};
    int missileLastSalvo = 1;
    float missileEnergyRequired = MissileEnergyCost;
    bool missileLowEnergy = false;
    void reset();
    void start(int index);
    void openMenu();
    void openWorkshop();
    bool buy(Upgrade item);
    void equip(PrimaryWeapon weapon);
    void advance();
    void skipLaunch();
    void skipScene();
    void pause();
    void resume();
    bool launchActive() const { return state == MissionState::Launching || (state == MissionState::Paused && pausedState == MissionState::Launching); }
    MissionState sceneState() const { return state == MissionState::Paused ? pausedState : state; }
    bool cinematicActive() const {
        const auto scene = sceneState();
        return scene == MissionState::Launching || scene == MissionState::Encounter || scene == MissionState::Landing || scene == MissionState::LiftOff || scene == MissionState::MarsFinale;
    }
    const StageDefinition& definition() const { return Stages[levelIndex]; }
    void update(float dt, const FlightInput& input);
    float progress() const;
    void burst(glm::vec2 position, glm::vec3 color, int count);
private:
    void hitEnemy(Enemy& enemy, float damage);
    void shipExplosion(glm::vec2 position,bool heavy);
    void updateEffects(float dt);
    void updateBeam(float dt, const FlightInput& input);
    void updateMissiles(float dt, const FlightInput& input);
    void resetMissileLock();
    void finishLevel();
    MissionState pausedState = MissionState::Flying;
    std::mt19937 random{42};
    unsigned int nextEnemyId = 1;
    float spawnTimer = 1.5f, asteroidTimer = 4.0f, pickupTimer = 10.0f;
    float range(float low, float high);
};
