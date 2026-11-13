#include "Level.hpp"
#include "MarsScene.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {
constexpr float dt=1.0f/120;
void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
void enterFinale(Level& game) {
    game.start(5); game.distance=game.definition().length-0.02f;
    game.player.position={game.distance,0}; game.update(dt,{});
}
}
int main() {
    try {
        Level game; game.profile.sol=73; enterFinale(game);
        require(game.state==MissionState::MarsFinale && !game.completed[5] && game.finaleTime==0,"Mars route must enter its rescue cinematic before recording victory");
        const auto original=MarsScene::cruiser(0,game.finaleStartScreen);
        require(original==game.finaleStartScreen,"Mars descent must start at the actual cruiser position");
        const int health=game.player.health,score=game.score,kills=game.kills;
        for(int i=0;i<MarsScene::Pursuers;++i) {
            require(MarsScene::pursuer(0,i).x<0,"Heavy pursuers must enter from behind");
            require(MarsScene::pursuer(6,i).x<MarsScene::cruiser(6,original).x,"Pursuing fleet must remain behind the cruiser");
        }
        bool left=false,right=false,above=false,below=false;
        const auto surrounded=MarsScene::cruiser(10.5f,original);
        for(int i=0;i<MarsScene::Pursuers;++i) {
            const auto p=MarsScene::pursuer(10.5f,i);
            left=left||p.x<surrounded.x-100; right=right||p.x>surrounded.x+100;
            above=above||p.y<surrounded.y-80; below=below||p.y>surrounded.y+80;
            require(glm::length(p-surrounded)>150,"Encircling enemies must leave visible space around the cruiser");
        }
        require(left && right && above && below,"Enemy fleet must surround the cruiser from all four sides before rescue");
        float minY=1000,maxY=-1000; int reversals=0; float previousDelta=0;
        for(int i=41;i<=65;++i) {
            const float time=i/10.0f;
            const auto p=MarsScene::cruiser(time,original),previous=MarsScene::cruiser(time-0.1f,original);
            minY=std::min(minY,p.y); maxY=std::max(maxY,p.y);
            const float delta=p.y-previous.y;
            if(delta*previousDelta<0) ++reversals;
            previousDelta=delta;
        }
        require(maxY-minY>110 && reversals>=2 && reversals<=3,"Cruiser must weave with slower, clearly separated evasive turns");
        require(MarsScene::camera(12.6f).reveal>0.99f,"Camera must reveal the approaching guard within a quick cut before the first kills");
        for(int i=0;i<MarsScene::Pursuers;++i) if(MarsScene::destructionTime(i)==12.8f) {
            const auto impact=MarsScene::camera(12.8f).project(MarsScene::pursuer(12.8f,i));
            require(impact.x>40 && impact.x<1240 && impact.y>76 && impact.y<644,"First enemy explosions must stay visible alongside the approaching guards");
        }
        const auto reveal=MarsScene::camera(13.8f);
        require(reveal.center.x>1100 && reveal.zoom>1.4f,"Camera must pan and zoom right to reveal the guards");
        for(int i=0;i<MarsScene::Fighters;++i) {
            const auto visible=reveal.project(MarsScene::fighter(13.8f,i));
            require(visible.x>100 && visible.x<1200 && visible.y>76 && visible.y<644,"All five guards must be framed in the reveal shot");
            require(MarsScene::fighter(24.5f,i).x< -100,"Guards must depart beyond the left edge after clearing the enemy ring");
            require(MarsScene::fighter(13.0f,i).x>MarsScene::fighter(14.3f,i).x,"Guard approach must keep moving during the camera reveal and first kills");
        }
        int counterShots=0;
        for(int enemy=0;enemy<MarsScene::Pursuers;++enemy) {
            if(MarsScene::destructionTime(enemy)<=14) continue;
            const auto direction=MarsScene::fighter(14,enemy%MarsScene::Fighters)-MarsScene::pursuer(14,enemy);
            const float heading=MarsScene::enemyHeading(14,enemy,original);
            require(glm::dot(glm::normalize(direction),glm::vec2(std::cos(heading),std::sin(heading)))>0.99f,"Surviving enemies must turn to face the incoming guards");
            for(int cycle=0;cycle<9;++cycle) for(int bullet=0;bullet<2;++bullet) {
                const auto shot=MarsScene::counterShot(14,enemy,cycle,bullet);
                if(!shot.active) continue;
                ++counterShots;
                require(shot.velocity.x>0,"Enemy counterfire must travel toward the guards on the right");
            }
        }
        require(counterShots>0,"Surviving enemies must fire back during the guard reveal");
        const auto wide=MarsScene::camera(19);
        require(wide.zoom==1 && wide.center==glm::vec2(640,360),"Camera must return to the battlefield for the final volleys");
        float closestMiss=10000; int maximumShots=0;
        for(int sample=400;sample<1060;sample+=2) {
            const float time=sample/100.0f; int active=0;
            for(int enemy=0;enemy<MarsScene::Pursuers;++enemy) for(int cycle=0;cycle<3;++cycle) for(int bullet=0;bullet<2;++bullet) {
                const auto shot=MarsScene::enemyShot(time,enemy,cycle,bullet,original);
                if(shot.active) { ++active; closestMiss=std::min(closestMiss,glm::length(shot.position-MarsScene::cruiser(time,original))); }
                if(time>=8.8f) require(!shot.active,"Enemy fire must cease before the ring is fully closed");
            }
            maximumShots=std::max(maximumShots,active);
        }
        require(maximumShots>0 && maximumShots<=16,"Enemy bursts must be sparse rather than filling the whole screen");
        if(closestMiss<=42) throw std::runtime_error("Enemy near-miss clearance too small: "+std::to_string(closestMiss));
        FlightInput heldFire; heldFire.fire=true; heldFire.laser=true; heldFire.missile=true;
        for(int i=0;i<1560;++i) game.update(dt,heldFire);
        require(game.state==MissionState::MarsFinale && std::count(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),true)==3 && game.explosions.size()==3,"First guard volley must destroy three enemies simultaneously");
        const float frozen=game.finaleTime;
        game.pause(); game.update(1,{});
        require(game.sceneState()==MissionState::MarsFinale && game.finaleTime==frozen,"Pause must freeze the Mars rescue");
        game.resume();
        for(int i=0;i<800 && game.finaleTime<17.3f;++i) game.update(dt,heldFire);
        require(std::count(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),true)==6,"Second volley must destroy another three enemies");
        for(int i=0;i<300 && game.finaleTime<18.5f;++i) game.update(dt,heldFire);
        require(std::count(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),true)==8 && game.state==MissionState::MarsFinale,"Last volley must destroy the remaining pair before the final result");
        while(game.finaleTime<MarsScene::EndCardStart+1) game.update(dt,{});
        require(game.state==MissionState::MarsFinale && !game.completed[5],"The black end card must play before recording final victory");
        for(int i=0;i<2500 && game.state==MissionState::MarsFinale;++i) {
            game.update(dt,heldFire);
            for(int ship=0;ship<MarsScene::Pursuers;++ship) require(game.finaleDestroyed[ship]==(game.finaleTime>=MarsScene::destructionTime(ship)),"Pursuers must be destroyed in synchronized groups at the guard's volley impacts");
        }
        require(game.state==MissionState::Won && game.completed[5] && std::all_of(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),[](bool value){return value;}),"All eight heavy ships must be destroyed before final victory");
        require(game.player.health==health && game.profile.sol==73 && game.kills==kills && game.score==score+health*10,"Cinematic rescue must award the finish bonus once without charging the player or inventing player kills");
        const int finalScore=game.score; game.update(dt,{}); require(game.score==finalScore,"Final victory cannot award its bonus twice");
        game.advance(); require(game.state==MissionState::Ready,"Finishing Mars must return to level selection");
        enterFinale(game); game.update(dt,heldFire);
        require(game.state==MissionState::MarsFinale,"Fire inputs must not skip the Mars descent");
        FlightInput skip; skip.skipCinematic=true;
        for(int i=0;i<500 && game.state==MissionState::MarsFinale;++i) game.update(dt,skip);
        require(game.state==MissionState::Won && game.finaleTime==Level::MarsFinaleDuration,"Holding Enter must deliberately skip to the final result");
        enterFinale(game); game.openMenu(); require(!game.cinematicActive(),"Returning to the menu must cancel the cinematic");
        std::cout<<"PASS: sparse near misses, encirclement, fast approach reveal, enemy turns and counterfire, 3/3/2 volleys and leftward departure\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; return 1; }
}
