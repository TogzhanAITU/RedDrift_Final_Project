#include "Level.hpp"
#include "Collision.hpp"
#include <algorithm>
#include <cmath>

using namespace Collision;

void Level::hitEnemy(Enemy& enemy, float damage) {
    if(enemy.health<=0) return;
    enemy.health-=damage;
    if(enemy.health>0.0001f) return;
    enemy.health=0;
    ++kills; score+=enemy.heavy?250:100;
    profile.sol=std::min(1000000000,profile.sol+(enemy.heavy?60:25));
    progressDirty=true;
    shipExplosion(enemy.position,enemy.heavy);
}

void Level::updateBeam(float dt, const FlightInput& input) {
    beamActive=input.laser && profile.laser && player.laserFiring;
    if(!beamActive) return;
    beamStart=player.position+glm::vec2(1.05f,0);
    const glm::vec2 farEnd=beamStart+glm::vec2(26,0);
    float nearest=1,contact=0;
    Enemy* target=nullptr; Asteroid* rockTarget=nullptr;
    for(auto& enemy:enemies) {
        if(enemy.health<=0) continue;
        if(sweptHit(beamStart,farEnd,enemy.position,enemy.heavy?glm::vec2(1.1f,0.65f):glm::vec2(0.85f,0.5f),&contact) && contact<nearest) {
            nearest=contact; target=&enemy;
        }
    }
    for(auto& rock:asteroids) {
        if(rock.health<=0) continue;
        if(sweptHit(beamStart,farEnd,rock.position,glm::vec2(rock.radius),&contact) && contact<nearest) {
            nearest=contact; target=nullptr; rockTarget=&rock;
        }
    }
    beamEnd=glm::mix(beamStart,farEnd,nearest);
    if(target) hitEnemy(*target,profile.laserDamage()*dt);
    if(rockTarget) {
        rockTarget->health-=profile.rockDamage()*dt;
        rockTarget->heat=1;
        if(rockTarget->health<=0.0001f) {
            rockTarget->health=0; score+=20;
            burst(rockTarget->position,{1.0f,0.58f,0.24f},25);
        }
    }
    if((target||rockTarget) && static_cast<int>(time*30)!=static_cast<int>((time-dt)*30)) burst(beamEnd,{0.4f,0.95f,1},2);
}

void Level::resetMissileLock() { missileTargetId=0; missileSecondTargetId=0; missileLockTime=0; }

void Level::updateMissiles(float dt, const FlightInput& input) {
    missileLaunchFlash=std::max(0.0f,missileLaunchFlash-dt);
    missileLowEnergy=false;
    if(!input.missile || !profile.missiles || player.missileCooldown>0) {
        resetMissileLock();
        return;
    }
    Enemy* target=nullptr;
    for(auto& enemy:enemies) if(enemy.id==missileTargetId && enemy.health>0 && glm::length(enemy.position-player.position)<=30) target=&enemy;
    if(!target) {
        resetMissileLock();
        float closest=26;
        for(auto& enemy:enemies) {
            const float targetDistance=glm::length(enemy.position-player.position);
            if(enemy.health>0 && enemy.position.x>player.position.x && targetDistance<closest) { target=&enemy; closest=targetDistance; }
        }
        if(target) missileTargetId=target->id;
    }
    if(!target) return;
    missileLockPosition=target->position;
    Enemy* second=nullptr;
    if(profile.missileCount()==2) {
        for(auto& enemy:enemies) if(enemy.id==missileSecondTargetId && enemy.id!=target->id && enemy.health>0 && glm::length(enemy.position-player.position)<=30) second=&enemy;
        if(!second) {
            float closest=26;
            for(auto& enemy:enemies) {
                const float rangeToEnemy=glm::length(enemy.position-player.position);
                if(enemy.id!=target->id && enemy.health>0 && enemy.position.x>player.position.x && rangeToEnemy<closest) { second=&enemy; closest=rangeToEnemy; }
            }
        }
    }
    const unsigned int secondaryId=second?second->id:0;
    if(secondaryId!=missileSecondTargetId) { missileLockTime=0; missileSecondTargetId=secondaryId; }
    if(second) missileSecondLockPosition=second->position;
    missileEnergyRequired=MissileEnergyCost*(second?2:1);
    const float lock=profile.lockDuration();
    missileLockTime=std::min(lock,missileLockTime+dt);
    if(missileLockTime<lock) return;
    if(player.energy<missileEnergyRequired) { missileLowEnergy=true; return; }
    auto launch=[&](Enemy& destination,float offset) {
        const auto muzzle=player.position+glm::vec2(0.8f,offset);
        const auto direction=glm::normalize(destination.position-muzzle);
        const float speed=profile.missileSpeed();
        bullets.push_back({muzzle,muzzle,direction*(speed*0.82f),false,4,true,profile.missileDamage(),destination.id,speed});
    };
    launch(*target,-0.3f);
    if(second) launch(*second,0.3f);
    missileLastSalvo=second?2:1;
    player.energy-=missileEnergyRequired;
    player.missileCooldown=1.4f;
    resetMissileLock(); missileLaunchFlash=0.4f;
}
