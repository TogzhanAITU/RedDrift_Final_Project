#include "Level.hpp"
#include "MarsScene.hpp"
#include "Collision.hpp"
#include <algorithm>
#include <cmath>

using namespace Collision;

float Level::range(float low, float high) { return std::uniform_real_distribution<float>(low, high)(random); }
float Level::progress() const { return std::clamp(distance / definition().length, 0.0f, 1.0f); }
void Level::start(int index) {
    const auto history = completed;
    const auto upgrades = profile;
    const bool dirty = progressDirty;
    *this = Level{};
    completed = history;
    profile = upgrades; progressDirty = dirty;
    player.maxHull = profile.maxHull(); player.health = player.maxHull;
    player.maxEnergy=static_cast<float>(profile.maxEnergy()); player.energy=player.maxEnergy;
    levelIndex = selectedLevel = std::clamp(index, 0, StageCount - 1);
    random.seed(42 + levelIndex);
    state = levelIndex == 0 ? MissionState::Launching : levelIndex == 1 ? MissionState::Encounter : levelIndex == 2 ? MissionState::LiftOff : MissionState::Flying;
}
void Level::reset() { start(levelIndex); }
void Level::openMenu() { selectedLevel = levelIndex; state = MissionState::Ready; workshopNotice = false; beamActive = false; resetMissileLock(); }
void Level::openWorkshop() {
    if (state != MissionState::Ready && state != MissionState::Won && state != MissionState::Lost) return;
    state = MissionState::Workshop; workshopNotice = false; beamActive = false;
    shopMessage = profile.workshopUnlocked ? "Select an upgrade below." : "Land at Selene station in level 2 to unlock.";
}
bool Level::buy(Upgrade item) {
    if (state != MissionState::Workshop) return false;
    if (!profile.workshopUnlocked) { shopMessage="Workshop locked. Complete level 2 first."; return false; }
    const int cost=profile.price(item);
    if (cost==0) { shopMessage="This upgrade is already installed."; return false; }
    if (profile.sol<cost) { shopMessage="Not enough Sol. Destroy enemy ships to earn more."; return false; }
    profile.sol-=cost;
    if(item==Upgrade::Hull) {
        ++profile.hullTier; player.maxHull=profile.maxHull(); player.health=player.maxHull;
        shopMessage="Hull reinforced. Maximum hull increased by 25.";
    } else if(item==Upgrade::Energy) {
        ++profile.energyTier; player.maxEnergy=static_cast<float>(profile.maxEnergy()); player.energy=player.maxEnergy;
        shopMessage="Energy capacity increased by 25. Battery fully charged.";
    } else if(item==Upgrade::Laser) {
        profile.laserTier=profile.laserLevel()+1; profile.laser=true; profile.weapon=PrimaryWeapon::Laser;
        shopMessage="Laser level "+std::to_string(profile.laserLevel())+" installed. Continuous damage increased.";
    } else {
        profile.missileTier=profile.missileLevel()+1; profile.missiles=true;
        shopMessage=profile.missileLevel()==3?"Level 3: two missiles can lock on to two different ships.":profile.missileLevel()==5?"Level 5: faster target lock and faster missiles.":"Missile damage increased. Hold E to lock on.";
    }
    progressDirty=true;
    return true;
}
void Level::equip(PrimaryWeapon weapon) {
    if(weapon==PrimaryWeapon::Laser && !profile.laser) return;
    if(profile.weapon==weapon) return;
    profile.weapon=weapon; progressDirty=true;
}
void Level::skipLaunch() {
    if (state != MissionState::Launching) return;
    launchTime = LaunchDuration;
    state = MissionState::Flying;
}
void Level::finishLevel() {
    if (state == MissionState::Won) return;
    state = MissionState::Won;
    beamActive=false;
    resetMissileLock();
    score += player.health * 10;
    completed[levelIndex] = true;
    if(levelIndex==1 && !profile.workshopUnlocked) {
        profile.workshopUnlocked=true;
        workshopNotice=true;
    }
    progressDirty=true;
}
void Level::skipScene() {
    if (state == MissionState::Launching) skipLaunch();
    else if (state == MissionState::Encounter) { encounterTime = EncounterDuration; state = MissionState::Flying; }
    else if (state == MissionState::Landing) { landingTime = LandingDuration; finishLevel(); }
    else if (state == MissionState::LiftOff) { takeoffTime = TakeoffDuration; state = MissionState::Flying; }
    else if (state == MissionState::MarsFinale) { finaleTime=MarsFinaleDuration; finaleDestroyed.fill(true); finishLevel(); }
}
void Level::pause() {
    if (state != MissionState::Flying && !cinematicActive()) return;
    if (state == MissionState::Paused) return;
    pausedState = state;
    state = MissionState::Paused;
}
void Level::resume() { if (state == MissionState::Paused) state = pausedState; }
void Level::advance() {
    if (state != MissionState::Won) return;
    if (levelIndex + 1 < StageCount) start(levelIndex + 1);
    else openMenu();
}
void Level::burst(glm::vec2 p, glm::vec3 color, int count) {
    for (int i = 0; i < count; ++i) {
        const float angle = range(0.0f, 6.283185f), speed = range(1.0f, 6.0f), life = range(0.25f, 0.8f);
        particles.push_back({p, glm::vec2(std::cos(angle), std::sin(angle)) * speed, color, life, life});
    }
}
void Level::shipExplosion(glm::vec2 position,bool heavy) {
    explosions.push_back({position,0,heavy?1.4f:1.0f});
    burst(position,{1.0f,0.5f,0.18f},heavy?45:30);
    const int fragments=heavy?18:12;
    for(int i=0;i<fragments;++i) {
        const float angle=6.283185f*i/fragments+range(-0.15f,0.15f);
        const float speed=range(2.5f,7.0f), life=range(1.0f,1.8f);
        const glm::vec2 radial(std::cos(angle),std::sin(angle));
        Particle shard{position+glm::vec2(range(-0.7f,0.7f),range(-0.2f,0.2f)),radial*speed+glm::vec2(-1,0),
                       {range(0.32f,0.65f),0.43f,0.43f},life,life};
        shard.shard=true; shard.size={range(0.12f,0.32f),range(0.05f,0.14f)};
        shard.angle=angle; shard.spin=range(-9.0f,9.0f);
        particles.push_back(shard);
    }
}
void Level::update(float dt, const FlightInput& input) {
    if (dt <= 0.0f) return;
    if(state==MissionState::MarsFinale) {
        finaleTime=std::min(MarsFinaleDuration,finaleTime+dt);
        for(int i=0;i<MarsScene::Pursuers;++i) if(!finaleDestroyed[i] && finaleTime>=MarsScene::destructionTime(i)) {
            finaleDestroyed[i]=true;
            const auto p=MarsScene::pursuer(finaleTime,i);
            shipExplosion({cameraX+(p.x-640)/36,(370-p.y)/36},true);
        }
        updateEffects(dt*1.35f);
        finaleSkipHold=input.skipCinematic && finaleTime>2.5f?finaleSkipHold+dt:0;
        if(finaleTime>=MarsFinaleDuration || finaleSkipHold>=1) skipScene();
        return;
    }
    if(state==MissionState::LiftOff) {
        takeoffTime=std::min(TakeoffDuration,takeoffTime+dt);
        if(takeoffTime>=TakeoffDuration) skipScene();
        return;
    }
    if (state == MissionState::Encounter) {
        encounterTime = std::min(EncounterDuration, encounterTime + dt);
        if (encounterTime >= EncounterDuration) skipScene();
        return;
    }
    if (state == MissionState::Landing) {
        landingTime = std::min(LandingDuration, landingTime + dt);
        landingSkipHold = input.skipCinematic && landingTime > 2.5f ? landingSkipHold + dt : 0.0f;
        if (landingTime >= LandingDuration || landingSkipHold >= 1.0f) {
            landingTime = LandingDuration;
            finishLevel();
        }
        return;
    }
    if (state == MissionState::Launching) {
        launchTime = std::min(LaunchDuration, launchTime + dt);
        if (launchTime >= LaunchDuration) skipLaunch();
        return;
    }
    if (state != MissionState::Flying) return;
    time += dt;
    const auto& stage = definition();
    distance = std::min(stage.length, distance + Player::CruiseSpeed * dt);
    cameraX = distance + 4.0f;
    const glm::vec2 oldPlayer = player.position;
    FlightInput poweredInput=input;
    poweredInput.laser=input.laser && profile.laser;
    player.update(dt, poweredInput, cameraX);
    if (input.fire && player.fireCooldown <= 0.0f) {
        const glm::vec2 muzzle = player.position + glm::vec2(1.05f, 0.0f);
        bullets.push_back({muzzle, muzzle, {34.0f, 0.0f}, false, 1.5f});
        player.fireCooldown = 0.16f;
    }
    spawnTimer -= dt; asteroidTimer -= dt; pickupTimer -= dt;
    if (stage.enemyTier > 0 && progress() > stage.enemyStart && spawnTimer <= 0.0f && progress() < 0.90f) {
        const int count = stage.enemyTier >= 2 ? 2 : 1;
        for (int i = 0; i < count; ++i) {
            const float y = range(-5.8f, 5.8f);
            const glm::vec2 p(cameraX + 19.0f + i * 2.5f, y);
            const bool heavy = stage.enemyTier == 3 || (stage.enemyTier == 2 && i == 0);
            enemies.push_back({p, p, y, range(0.0f, 6.28f), range(0.8f, 1.6f), heavy ? (stage.enemyTier == 3 ? 5.0f : 4.0f) : 2.0f, heavy});
        }
        spawnTimer = stage.waveInterval + range(-0.3f, 0.3f);
    }
    if (asteroidTimer <= 0.0f && progress() < 0.90f) {
        asteroids.push_back({{cameraX + 20.0f, range(-5.8f, 5.8f)}, range(0.45f, 1.0f), range(0.0f, 6.28f), range(0,1) < 0.4f});
        asteroidTimer = stage.rockInterval + range(-0.25f, 0.35f);
    }
    if (pickupTimer <= 0.0f && progress() < 0.96f) {
        pickups.push_back({{cameraX + 18.0f, range(-5.0f, 5.0f)}});
        pickupTimer = 9.0f;
    }
    for (auto& enemy : enemies) {
        if(enemy.id==0) enemy.id=nextEnemyId++;
        enemy.previous = enemy.position;
        enemy.position.x -= (enemy.heavy ? 1.0f : 2.3f) * dt;
        enemy.position.y = enemy.baseY + std::sin(time * 1.8f + enemy.phase) * 0.7f;
        enemy.cooldown -= dt;
        if (enemy.cooldown <= 0.0f && enemy.position.x < cameraX + 15.0f && enemy.position.x > player.position.x + 1.0f) {
            const glm::vec2 p = enemy.position - glm::vec2(0.9f, 0.0f);
            bullets.push_back({p, p, glm::normalize(player.position - p) * (enemy.heavy ? 12.0f : 10.0f), true, 4.0f});
            enemy.cooldown = enemy.heavy ? (stage.enemyTier == 3 ? 0.85f : 1.15f) : 2.0f;
        }
    }
    for (auto& rock : asteroids) { rock.position.x -= dt; rock.rotation += dt * 0.4f; rock.heat=std::max(0.0f,rock.heat-dt*2); }
    updateBeam(dt,input);
    updateMissiles(dt,input);
    for (auto& bullet : bullets) {
        bullet.previous = bullet.position;
        if(bullet.homing) {
            const Enemy* target=nullptr; float closest=1000;
            for(const auto& enemy:enemies) if(enemy.id==bullet.targetId && enemy.health>0) target=&enemy;
            if(!target) for(const auto& enemy:enemies) if(enemy.health>0) {
                const float distanceToEnemy=glm::length(enemy.position-bullet.position);
                if(distanceToEnemy<closest) { closest=distanceToEnemy; target=&enemy; }
            }
            if(target && glm::length(target->position-bullet.position)>0.01f) {
                bullet.targetId=target->id;
                const auto aim=glm::normalize(target->position-bullet.position)*bullet.cruiseSpeed;
                bullet.velocity=glm::mix(bullet.velocity,aim,1-std::exp(-5.0f*dt));
            }
        }
        bullet.position += bullet.velocity * dt;
        bullet.life -= dt;
        if (bullet.life <= 0.0f) continue;
        float nearest = 2.0f, contact = 0.0f;
        bool hitRock = false, hitPlayer = false;
        Asteroid* struckRock=nullptr;
        Enemy* hitEnemy = nullptr;
        for (auto& rock : asteroids) {
            if(rock.health<=0) continue;
            if (sweptHit(bullet.previous, bullet.position, rock.position, glm::vec2(rock.radius), &contact) && contact < nearest) {
                nearest = contact; hitRock = true; struckRock=&rock;
            }
        }
        if (bullet.hostile) {
            if (sweptHit(bullet.previous - oldPlayer, bullet.position - player.position, glm::vec2(0), player.size * 0.5f + glm::vec2(0.14f), &contact) && contact < nearest) {
                nearest = contact; hitRock = false; hitPlayer = true;
            }
        } else {
            for (auto& enemy : enemies) {
                if (enemy.health <= 0) continue;
                const glm::vec2 half = enemy.heavy ? glm::vec2(1.1f, 0.65f) : glm::vec2(0.85f, 0.5f);
                if (sweptHit(bullet.previous - enemy.previous, bullet.position - enemy.position, glm::vec2(0), half + glm::vec2(0.12f), &contact) && contact < nearest) {
                    nearest = contact; hitRock = false; hitEnemy = &enemy;
                }
            }
        }
        if (nearest <= 1.0f) {
            bullet.position = glm::mix(bullet.previous, bullet.position, nearest);
            bullet.life = 0.0f;
            if (hitRock) {
                burst(bullet.position, {0.7f, 0.6f, 0.5f}, 5);
                if(bullet.homing && struckRock) {
                    struckRock->health-=bullet.damage;
                    if(struckRock->health<=0) burst(struckRock->position,{1,0.6f,0.2f},20);
                }
            }
            else if (hitPlayer) {
                if (player.damage(12)) burst(player.position, {1.0f, 0.35f, 0.2f}, 16);
            } else if (hitEnemy) {
                this->hitEnemy(*hitEnemy,bullet.damage);
                burst(bullet.position, {0.3f, 1.0f, 0.85f}, 6);
            }
        }
    }
    for (const auto& enemy : enemies) {
        if (enemy.health > 0 && overlap(player.position, player.size * 0.5f, enemy.position, {0.9f, 0.55f})) {
            if (player.damage(25)) burst(player.position, {1.0f, 0.35f, 0.2f}, 20);
        }
    }
    for (const auto& rock : asteroids) {
        if (rock.health>0 && circleHit(player.position, player.size * 0.5f, rock.position,rock.radius)) {
            if (player.damage(20)) burst(player.position, {1.0f, 0.6f, 0.3f}, 18);
        }
    }
    for (auto it = pickups.begin(); it != pickups.end();) {
        if (overlap(player.position, player.size * 0.5f, it->position, {0.5f, 0.5f})) {
            player.health = std::min(player.maxHull, player.health + 25); score += 50;
            burst(it->position, {0.3f, 1.0f, 0.65f}, 20); it = pickups.erase(it);
        } else if (it->position.x < cameraX - 20.0f) it = pickups.erase(it);
        else ++it;
    }
    updateEffects(dt);
    const float behind = cameraX - 22.0f;
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [behind](const Enemy& e) { return e.health <= 0 || e.position.x < behind; }), enemies.end());
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [behind](const Bullet& b) { return b.life <= 0.0f || b.position.x < behind; }), bullets.end());
    asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(), [behind](const Asteroid& a) { return a.health<=0 || a.position.x < behind; }), asteroids.end());
    if (player.health <= 0) { state = MissionState::Lost; beamActive=false; burst(player.position, {1.0f, 0.4f, 0.1f}, 60); }
    else if (distance >= stage.length) {
        if (levelIndex == 1 || levelIndex == 5) {
            state = levelIndex==1?MissionState::Landing:MissionState::MarsFinale;
            beamActive=false;
            resetMissileLock();
            landingTime = 0.0f; finaleTime=0;
            landingStartScreen = {640+(player.position.x-cameraX)*36,370-player.position.y*36};
            landingStartTilt = -player.velocity.y*0.025f;
            finaleStartScreen=landingStartScreen; finaleStartTilt=landingStartTilt;
            enemies.clear(); bullets.clear(); asteroids.clear(); pickups.clear(); particles.clear(); explosions.clear();
        } else finishLevel();
    }
}
void Level::updateEffects(float dt) {
    const float damping=std::exp(-0.25f*dt);
    for (auto& p : particles) {
        p.position += p.velocity * dt; p.life -= dt;
        if(p.shard) { p.angle+=p.spin*dt; p.velocity*=damping; }
    }
    for(auto& blast:explosions) blast.age+=dt;
    explosions.erase(std::remove_if(explosions.begin(),explosions.end(),[](const Explosion& blast){return blast.age>=1.0f;}),explosions.end());
    particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.life <= 0.0f; }), particles.end());
}
