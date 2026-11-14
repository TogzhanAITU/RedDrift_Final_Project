#include "Level.hpp"
#include "Menu.hpp"
#include "LaunchScene.hpp"
#include "MoonScene.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool condition,const char* message) { if(!condition) throw std::runtime_error(message); }
int main() {
    try {
        constexpr float dt=1.0f/120.0f;
        Level intro;
        intro.start(0);
        require(intro.state==MissionState::Launching,"First stage must begin with station departure");
        FlightInput cinematicInput; cinematicInput.fire=true; cinematicInput.boost=true; cinematicInput.direction={1,1};
        for(int i=0;i<120;++i) intro.update(dt,cinematicInput);
        require(intro.state==MissionState::Launching && intro.distance==0 && intro.time==0 && intro.player.position==glm::vec2(0),"Intro must not advance the flight or accept movement");
        require(intro.bullets.empty() && intro.asteroids.empty() && intro.enemies.empty() && intro.player.energy==100,"Combat and encounters must stay inactive during departure");
        const float frozen=intro.launchTime;
        intro.pause();
        for(int i=0;i<120;++i) intro.update(dt,{});
        require(intro.launchActive() && intro.launchTime==frozen,"Pausing must freeze and retain the departure scene");
        intro.resume();
        require(intro.state==MissionState::Launching,"Resuming intro must not skip it");
        for(int i=0;i<1000 && intro.state==MissionState::Launching;++i) intro.update(dt,{});
        require(intro.state==MissionState::Flying && intro.distance==0 && intro.launchTime==Level::LaunchDuration,"Departure must hand off to fresh gameplay automatically");
        intro.update(dt,{});
        require(intro.distance>0,"Flight must advance after departure");
        intro.reset();
        require(intro.state==MissionState::Launching && intro.launchTime==0,"Restarting stage one must replay departure");
        intro.skipLaunch();
        require(intro.state==MissionState::Flying && intro.distance==0 && intro.bullets.empty(),"Skipping intro must start a clean flight");
        intro.start(0); intro.openMenu();
        require(intro.state==MissionState::Ready && !intro.launchActive(),"Leaving intro must return to level selection");
        intro.start(3);
        require(intro.state==MissionState::Flying && !intro.launchActive(),"Other stages must start normally");
        Level ascent;
        ascent.start(2);
        require(ascent.state==MissionState::LiftOff && ascent.definition().environment==Environment::Moon,"Third level must start from the lunar station");
        for(int i=0;i<180;++i) ascent.update(dt,cinematicInput);
        require(ascent.distance==0 && ascent.time==0 && ascent.bullets.empty() && ascent.enemies.empty(),"Lift-off must not advance gameplay or accept weapon input");
        const float liftFrozen=ascent.takeoffTime;
        ascent.pause(); ascent.update(1,{});
        require(ascent.sceneState()==MissionState::LiftOff && ascent.takeoffTime==liftFrozen,"Pausing must retain lift-off camera position");
        ascent.resume();
        for(int i=0;i<1200 && ascent.state==MissionState::LiftOff;++i) ascent.update(dt,{});
        require(ascent.state==MissionState::Flying && ascent.distance==0,"Lift-off must hand over to fresh gameplay");
        const auto liftStart=MoonScene::takeoff(0),liftMid=MoonScene::takeoff(3.5f),liftEnd=MoonScene::takeoff(Level::TakeoffDuration);
        require(liftStart.player==glm::vec2(800,421) && liftStart.legs==1 && liftStart.engine==0,"Lift-off must begin parked on Selene's landing deck");
        require(liftMid.player.y<liftStart.player.y && liftMid.engine>0,"Cruiser must rise from the station using its engines");
        require(liftEnd.player==glm::vec2(496,370) && liftEnd.scale==36 && liftEnd.legs==0,"Takeoff camera must match the flight view and retract landing gear");
        ascent.reset(); ascent.skipScene(); require(ascent.state==MissionState::Flying,"Lift-off must support restart and skipping");
        Level lunar;
        lunar.start(1);
        require(lunar.state==MissionState::Encounter,"Second stage must introduce the approaching enemy");
        for(int i=0;i<180;++i) lunar.update(dt,cinematicInput);
        require(lunar.distance==0 && lunar.time==0 && lunar.enemies.empty() && lunar.bullets.empty(),"Enemy flyby must not advance combat or route");
        const float encounterFrozen=lunar.encounterTime;
        lunar.pause(); lunar.update(1,{});
        require(lunar.cinematicActive() && lunar.encounterTime==encounterFrozen,"Pausing must freeze the enemy camera shot");
        lunar.resume();
        for(int i=0;i<1000 && lunar.state==MissionState::Encounter;++i) lunar.update(dt,{});
        require(lunar.state==MissionState::Flying && lunar.distance==0,"Enemy introduction must hand over to fresh gameplay");
        const auto distant=MoonScene::encounter(0),close=MoonScene::encounter(2.8f),handoff=MoonScene::encounter(Level::EncounterDuration);
        require(close.enemyScale>distant.enemyScale*10 && handoff.enemy.x+handoff.enemyScale<0,"Hostile must approach from the distance and fly out of view");
        require(handoff.player==glm::vec2(496,370) && handoff.playerScale==36 && handoff.bars==0,"Lunar intro must match gameplay camera at handoff");
        lunar.distance=lunar.definition().length-0.02f; lunar.player.position={lunar.distance,1};
        lunar.update(dt,{});
        require(lunar.state==MissionState::Landing && lunar.progress()==1 && !lunar.completed[1],"Reaching lunar finish must begin landing before marking victory");
        const int beforeLanding=lunar.score;
        lunar.advance(); require(lunar.state==MissionState::Landing,"Next stage must wait for landing");
        const auto landingStart=MoonScene::landing(0,lunar.landingStartScreen,lunar.landingStartTilt);
        require(landingStart.player==lunar.landingStartScreen && landingStart.scale==36,"Landing must start at the actual cruiser position");
        lunar.pause(); lunar.update(1,{});
        require(lunar.sceneState()==MissionState::Landing && lunar.landingTime==0,"Landing must pause without advancing");
        lunar.resume();
        for(int i=0;i<2000 && lunar.state==MissionState::Landing;++i) lunar.update(dt,cinematicInput);
        require(lunar.state==MissionState::Won && lunar.completed[1] && lunar.score==beforeLanding+lunar.player.health*10,"Touchdown must finish level and award the bonus once");
        lunar.update(dt,{}); lunar.skipScene();
        require(lunar.score==beforeLanding+lunar.player.health*10,"Landing cannot award the finish bonus twice");
        require(MoonScene::caption(0).empty() && MoonScene::caption(1).size()<MoonScene::ArrivalText.size() && MoonScene::caption(2.5f)==MoonScene::ArrivalText,"Arrival text must type out verbatim before descent");
        const auto touchdown=MoonScene::landing(Level::LandingDuration,{496,370},0);
        const float surface=touchdown.moon.y-std::sqrt(touchdown.radius*touchdown.radius-10000);
        require(std::abs(touchdown.player.y+touchdown.scale*0.5f-(surface-MoonScene::DeckHeight))<1 && touchdown.engine==0 && touchdown.legs==1,"Cruiser must settle on the station deck with engines off and landing gear down");
        for(int i=150;i<=720;++i) {
            const auto frame=MoonScene::landing(i/60.0f,{496,370},0);
            const float dx=frame.player.x-frame.moon.x;
            if(std::abs(dx)<frame.radius) {
                const float ground=frame.moon.y-std::sqrt(frame.radius*frame.radius-dx*dx);
                require(frame.player.y+frame.scale*0.5f<=ground+1,"Descent must stay above lunar terrain until touchdown");
            }
        }
        lunar.start(1); lunar.skipScene();
        require(lunar.state==MissionState::Flying && lunar.distance==0,"Enemy intro must support skipping");
        lunar.distance=lunar.definition().length-0.02f; lunar.player.position.x=lunar.distance; lunar.update(dt,{}); lunar.skipScene();
        require(lunar.state==MissionState::Won,"Landing must support skipping to the result");
        lunar.start(1); lunar.skipScene();
        lunar.distance=lunar.definition().length-0.02f; lunar.player.position.x=lunar.distance; lunar.update(dt,{});
        for(int i=0;i<420;++i) lunar.update(dt,cinematicInput);
        require(lunar.state==MissionState::Landing && lunar.landingTime>3,"Holding fire must not skip descent");
        FlightInput deliberateSkip; deliberateSkip.skipCinematic=true;
        lunar.update(dt,deliberateSkip); lunar.update(dt,{});
        require(lunar.state==MissionState::Landing && lunar.landingSkipHold==0,"A single Enter press must not skip landing");
        for(int i=0;i<125;++i) lunar.update(dt,deliberateSkip);
        require(lunar.state==MissionState::Won,"Holding Enter deliberately must allow skipping landing");
        Level threshold;
        threshold.start(1); threshold.skipScene();
        bool sawLateScouts=false;
        for(int i=0;i<2600;++i) {
            threshold.player.invulnerability=2;
            threshold.update(dt,{});
            if(threshold.progress()<=0.5f) require(threshold.enemies.empty(),"Level two enemies must not spawn before or at 50 percent");
            else for(const auto& enemy:threshold.enemies) {
                sawLateScouts=true;
                require(!enemy.heavy && enemy.health==2,"Second half of level two must contain only light enemies");
            }
        }
        require(sawLateScouts,"Light enemies must spawn after 50 percent in level two");
        const auto closed=LaunchScene::sample(0),opened=LaunchScene::sample(1.4f),departed=LaunchScene::sample(Level::LaunchDuration);
        require(closed.door==0 && closed.engine==0 && opened.door>0.999f && opened.shipX==closed.shipX,"Hangar must open before the ship starts moving");
        require(departed.zoom==0.5f && departed.screenX==496 && departed.y(370)==370 && departed.x(441)<0 && departed.bars==0,"Cutscene endpoint must match gameplay framing with station offscreen");
        Player p;
        Player jets;
        FlightInput thrust; thrust.direction={0,1};
        for(int i=0;i<30;++i) jets.update(dt,thrust,4+jets.position.x);
        require(jets.maneuverJets.x>0.99f && jets.maneuverJets.y==0,"Up input must ignite lower maneuvering jets");
        thrust.direction.y=-1;
        for(int i=0;i<30;++i) jets.update(dt,thrust,4+jets.position.x);
        require(jets.maneuverJets.y>0.99f && jets.maneuverJets.x<0.01f,"Down input must ignite upper jets and extinguish lower jets");
        for(int i=0;i<60;++i) jets.update(dt,{},4+jets.position.x);
        require(glm::length(jets.maneuverJets)<0.001f,"Releasing movement must fade maneuvering jets");
        for(int i=0;i<120;++i) p.update(dt,{},i*dt*8+4);
        require(std::abs(p.position.x-8)<0.01f && p.position.y==0,"Cruise must move horizontally without falling");
        FlightInput up; up.direction={0,1};
        for(int i=0;i<240;++i) p.update(dt,up,4+p.position.x);
        require(p.position.y<=Player::Corridor-p.size.y/2+0.001f,"Rocket must stay inside corridor");
        up.boost=true;
        for(int i=0;i<2000;++i) p.update(dt,up,4+p.position.x);
        require(p.energy>=0 && p.energy<=100,"Boost energy must remain bounded");
        Player coarse,fine;
        for(int i=0;i<60;++i) coarse.update(1.0f/60,up,4+coarse.position.x);
        for(int i=0;i<120;++i) fine.update(dt,up,4+fine.position.x);
        require(glm::length(coarse.position-fine.position)<0.08f,"Flight should be stable at different update rates");
        require(p.damage(20)&&!p.damage(20)&&p.health==80,"Repeated hits must respect invulnerability");
        Level game; game.reset(); game.skipLaunch(); game.state=MissionState::Paused;
        game.update(dt,up); require(game.time==0 && game.distance==0,"Pause must freeze simulation");
        game.reset(); game.skipLaunch();
        const glm::vec2 target(2,0);
        game.enemies.push_back({target,target,0,0,100,1,false});
        game.bullets.push_back({{0,0},{0,0},{600,0},false,1});
        game.update(dt,{});
        require(game.kills==1 && game.score==100,"Swept laser must hit a ship even when crossing it in one tick");
        game.reset(); game.skipLaunch();
        game.enemies.push_back({{4,0},{4,0},0,0,100,1,false});
        game.asteroids.push_back({{2,0},0.5f,0});
        game.bullets.push_back({{0,0},{0,0},{800,0},false,1});
        game.update(dt,{});
        require(game.kills==0 && game.enemies[0].health==1 && game.bullets.empty(),"Asteroid must block laser before it reaches enemy");
        game.reset(); game.skipLaunch(); game.player.health=45; game.pickups.push_back({{0,0}});
        game.update(dt,{}); require(game.player.health==70 && game.pickups.empty(),"Repair must restore hull and be consumed");
        game.reset(); game.skipLaunch(); game.player.health=10;
        game.asteroids.push_back({{0,0},1,0}); game.update(dt,{});
        require(game.state==MissionState::Lost && game.player.health==0,"Lethal collision must end mission");
        game.reset(); game.skipLaunch(); game.distance=game.definition().length-0.02f; game.player.position.x=game.distance;
        game.update(dt,{}); require(game.state==MissionState::Won && game.progress()==1,"Reaching Mars must win");
        const int finalScore=game.score; game.update(dt,{}); require(game.score==finalScore,"Victory bonus must only apply once");
        game.reset(); game.skipLaunch(); size_t maxEntities=0;
        for(int i=0;i<12500 && game.state==MissionState::Flying;++i) {
            game.player.invulnerability=2;
            FlightInput input; input.fire=true; input.direction.y=std::sin(i*0.008f);
            game.update(dt,input);
            maxEntities=std::max(maxEntities,game.bullets.size()+game.enemies.size()+game.asteroids.size()+game.particles.size());
        }
        require(game.state==MissionState::Won,"Full mission must reach Mars");
        require(maxEntities<1000,"Offscreen objects must be removed");
        game.reset(); game.skipLaunch(); require(game.score==0 && game.player.health==100 && game.enemies.empty() && game.distance==0,"Restart must clear all mission state");
        for(int index=0;index<Level::StageCount;++index) {
            game.start(index); game.skipScene();
            bool sawEnemy=false, sawHeavy=false, sawHostileBullet=false, sawRock=false, sawDebris=false;
            for(int i=0;i<6000 && game.state==MissionState::Flying;++i) {
                game.player.invulnerability=2;
                game.update(dt,{});
                for(const auto& enemy:game.enemies) { sawEnemy=true; sawHeavy=sawHeavy||enemy.heavy; }
                for(const auto& bullet:game.bullets) sawHostileBullet=sawHostileBullet||bullet.hostile;
                for(const auto& rock:game.asteroids) { sawRock=true; sawDebris=sawDebris||rock.debris; }
            }
            for(int i=0;i<static_cast<int>((Level::MarsFinaleDuration+1)/dt) && (game.state==MissionState::Landing || game.state==MissionState::MarsFinale);++i) game.update(dt,{});
            require(game.state==MissionState::Won && game.completed[index],"Every stage must finish and record completion");
            require(sawRock && sawDebris,"Every stage must generate rocks and space debris");
            require(sawEnemy==(index>=1),"Enemy ships must appear from stage two onward");
            require(sawHostileBullet==(index>=1),"Enemy shooting must begin at stage two");
            require(sawHeavy==(index>=4),"Heavy enemies must begin at stage five");
            const auto saved=game.completed;
            game.advance();
            require(game.completed==saved,"Advancing must preserve completed stages");
            require(index==5?game.state==MissionState::Ready:((game.state==MissionState::Flying || game.state==MissionState::Encounter || game.state==MissionState::LiftOff) && game.levelIndex==index+1),"Victory must advance to next stage or return to selection");
        }
        for(int i=0;i<Level::StageCount;++i) {
            const auto card=Menu::card(i);
            require(Menu::cardAt({card.x+10,card.y+10})==i,"Each card must have its own clickable area");
            const auto scaled=Menu::cursor((card.x+10)*1.5+40,(card.y+10)*1.5,2000,1080);
            require(Menu::cardAt(scaled)==i,"Card hit testing must respect letterboxing and window size");
        }
        require(Menu::cardAt({10,10})==-1,"Clicking outside the cards must not select a level");
        std::cout<<"PASS: flight, boundaries, boost, damage, pause, swept hits, repair, defeat, victory, full mission, restart\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; return 1; }
}
