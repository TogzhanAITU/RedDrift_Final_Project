#include "Level.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
constexpr float dt=1.0f/120;
void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
void ticks(Level& level,int count,const FlightInput& input) { for(int i=0;i<count;++i) level.update(dt,input); }
void enemy(Level& level,float x,float y) { const glm::vec2 p(x,y); level.enemies.push_back({p,p,y,0,100,40,false}); }
Level missiles(int tier) { Level game; game.profile.missiles=true; game.profile.missileTier=tier; game.start(0); game.skipScene(); return game; }
}
int main() {
    try {
        FlightInput lock; lock.missile=true;
        auto salvo=missiles(3); enemy(salvo,22,4); enemy(salvo,24,-4);
        ticks(salvo,90,lock);
        require(salvo.missileTargetId!=0 && salvo.missileSecondTargetId!=0 && salvo.missileTargetId!=salvo.missileSecondTargetId && salvo.bullets.empty(),"Level-three launcher must lock two distinct ships before firing");
        ticks(salvo,92,lock);
        require(salvo.bullets.size()==2 && salvo.bullets[0].targetId!=salvo.bullets[1].targetId,"Level-three salvo must launch one guided missile per target");
        require(salvo.player.energy>=60 && salvo.player.energy<60.5f && salvo.bullets[0].damage==5 && salvo.bullets[1].damage==5,"Dual salvo must cost 40 Energy and apply level-three damage to both missiles");
        ticks(salvo,180,{});
        require(salvo.enemies.size()==2 && std::abs(salvo.enemies[0].health-35)<0.01f && std::abs(salvo.enemies[1].health-35)<0.01f,"Both missiles must damage their separately locked targets");
        auto single=missiles(3); enemy(single,22,4); ticks(single,182,lock);
        require(single.bullets.size()==1 && single.player.energy>=80 && single.player.energy<80.5f,"One available target must receive only one missile for 20 Energy");
        auto low=missiles(3); low.profile.laser=true; low.player.energy=25; enemy(low,22,4); enemy(low,24,-4);
        FlightInput depleted=lock; depleted.laser=true;
        ticks(low,182,depleted);
        require(low.bullets.empty() && low.missileLowEnergy && low.missileEnergyRequired==40,"Dual lock must wait for enough energy to pay for both missiles");
        low.player.energy=40; low.update(dt,lock);
        require(low.bullets.size()==2 && low.player.energy<1,"A funded dual salvo must launch both missiles with one resource transaction");
        auto fast=missiles(5); enemy(fast,22,4); ticks(fast,119,lock);
        require(fast.bullets.empty() && fast.missileLockTime>0.9f,"Level-five launcher still needs its one-second lock");
        ticks(fast,3,lock);
        require(fast.bullets.size()==1 && fast.bullets[0].damage==7 && fast.bullets[0].cruiseSpeed==30 && glm::length(fast.bullets[0].velocity)>24,"Level five must launch earlier, fly faster and hit harder");
        Level laser; laser.profile.laser=true; laser.profile.laserTier=4; laser.start(0); laser.skipScene(); enemy(laser,18,0);
        FlightInput beam; beam.laser=true; ticks(laser,30,beam);
        require(std::abs(laser.enemies[0].health-37.5f)<0.01f,"Laser level four must apply ten sustained damage per second");
        Profile preview; preview.missiles=true; preview.missileTier=2;
        require(preview.nextUpgrade(Upgrade::Missiles)[1].find("2 MISSILES")!=std::string::npos,"Workshop must preview the dual-target milestone before buying level three");
        preview.missileTier=4;
        require(preview.nextUpgrade(Upgrade::Missiles)[1].find("1.0S")!=std::string::npos && preview.nextUpgrade(Upgrade::Missiles)[2].find("30")!=std::string::npos,"Workshop must preview both level-five speed improvements");
        preview.missileTier=5;
        require(preview.price(Upgrade::Missiles)==0 && preview.nextUpgrade(Upgrade::Missiles)[0].find("MAXIMUM")!=std::string::npos,"Max-level card must clearly stop further upgrades");
        std::cout<<"PASS: weapon levels, distinct-target salvo, energy costs, speed, damage and previews\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; return 1; }
}
