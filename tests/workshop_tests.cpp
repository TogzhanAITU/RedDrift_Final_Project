#include "Level.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
constexpr float dt=1.0f/120;
void require(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
void ticks(Level& game,int count,FlightInput input={}) { for(int i=0;i<count;++i) game.update(dt,input); }
void enemy(Level& game,glm::vec2 p,float hp=2,bool heavy=false) {
    game.enemies.push_back({p,p,p.y,0,100,hp,heavy});
}
}
int main() {
    try {
        Level economy;
        economy.profile.sol=1000;
        economy.openWorkshop();
        require(!economy.buy(Upgrade::Laser) && economy.profile.sol==1000,"Locked workshop must not take payment");
        economy.start(1); economy.skipScene();
        economy.distance=economy.definition().length-0.02f; economy.player.position.x=economy.distance;
        economy.update(dt,{});
        require(!economy.profile.workshopUnlocked && economy.state==MissionState::Landing,"Workshop must stay locked until the cruiser lands");
        economy.skipScene();
        require(economy.profile.workshopUnlocked && economy.workshopNotice && economy.progressDirty,"First lunar landing must unlock workshop and show a notice");
        economy.openWorkshop();
        require(!economy.workshopNotice && economy.state==MissionState::Workshop,"Workshop button must dismiss unlock notice");
        require(economy.buy(Upgrade::Laser) && economy.profile.sol==900 && economy.profile.weapon==PrimaryWeapon::Laser,"Laser purchase must deduct 100 Sol and equip the cannon");
        require(economy.buy(Upgrade::Laser) && economy.profile.sol==700 && economy.profile.laserLevel()==2 && economy.profile.laserDamage()==6,"Owned laser must upgrade to level two for 200 Sol and gain sustained damage");
        require(economy.buy(Upgrade::Missiles) && economy.profile.sol==550,"Missile launcher must cost 150 Sol");
        require(economy.buy(Upgrade::Hull) && economy.profile.sol==490 && economy.player.maxHull==125,"Hull upgrade must cost 60 Sol and add 25 hull");
        economy.profile.sol=0;
        require(!economy.buy(Upgrade::Hull) && economy.profile.hullTier==1 && economy.profile.sol==0,"Insufficient funds must not change upgrades or wallet");
        economy.profile.sol=5000;
        while(economy.profile.hullTier<5) require(economy.buy(Upgrade::Hull),"Hull tiers must remain purchasable up to the cap");
        while(economy.profile.laserLevel()<5) require(economy.buy(Upgrade::Laser),"Laser must upgrade through level five");
        while(economy.profile.missileLevel()<5) require(economy.buy(Upgrade::Missiles),"Missile launcher must upgrade through level five");
        const int balance=economy.profile.sol;
        require(!economy.buy(Upgrade::Hull) && economy.profile.maxHull()==225 && economy.profile.sol==balance,"Maximum hull must be capped at 225 without repeat charges");
        require(!economy.buy(Upgrade::Laser) && !economy.buy(Upgrade::Missiles) && economy.profile.sol==balance,"Maximum weapon levels must not charge more Sol");
        require(economy.profile.laserDamage()==12 && economy.profile.missileDamage()==7,"Level-five weapons must apply their increased damage");
        economy.start(0); economy.skipScene();
        require(economy.player.health==225 && economy.profile.laser && economy.profile.missiles && economy.profile.sol==balance,"Starting and restarting missions must preserve purchases and wallet");
        require(!economy.buy(Upgrade::Hull),"Purchases must only be allowed inside the workshop");
        economy.player.health=215; economy.pickups.push_back({{0,0}}); economy.update(dt,{});
        require(economy.player.health==225,"Repair pickups must respect the upgraded hull limit");
        Level pulse; pulse.start(0); pulse.skipScene();
        enemy(pulse,{2,0},1);
        for(int i=0;i<2;++i) pulse.bullets.push_back({{0,0},{0,0},{600,0},false,1});
        pulse.update(dt,{});
        require(pulse.kills==1 && pulse.profile.sol==25,"Multiple hits on one destroyed scout must award Sol only once");
        enemy(pulse,{4,0},1,true); pulse.bullets.push_back({{2,0},{2,0},{600,0},false,1}); pulse.update(dt,{});
        require(pulse.profile.sol==85,"Heavy ships must award 60 Sol");
        pulse.player.health=0; pulse.update(dt,{}); pulse.reset();
        require(pulse.profile.sol==85,"Sol earned by kills must survive defeat and retry");
        Level beam; beam.profile.laser=true; beam.profile.weapon=PrimaryWeapon::Laser;
        beam.start(0); beam.skipScene();
        beam.asteroids.push_back({{22,0},0.8f,0}); enemy(beam,{25,0},2);
        FlightInput fire; fire.laser=true;
        ticks(beam,30,fire);
        require(beam.beamActive && beam.bullets.empty() && beam.asteroids.size()==1 && beam.asteroids[0].health<3 && beam.asteroids[0].health>0,"Laser must be continuous and melt rocks gradually rather than instantly");
        require(beam.enemies[0].health==2,"Nearest rock must block laser damage to enemies behind it");
        require(std::abs(beam.asteroids[0].health-1.5f)<0.01f,"Laser must melt meteors at six damage per second");
        const float rockHealth=beam.asteroids[0].health;
        ticks(beam,30,{});
        require(!beam.beamActive && beam.asteroids[0].health==rockHealth,"Releasing fire must stop laser damage immediately");
        ticks(beam,160,fire);
        require(beam.asteroids.empty(),"Sustained laser fire must destroy the rock");
        FlightInput bullets; bullets.fire=true;
        beam.equip(PrimaryWeapon::Laser); ticks(beam,1,bullets);
        require(!beam.beamActive && !beam.bullets.empty(),"Left-button fire must always produce bullets even with laser installed");
        Level laserKill; laserKill.profile.laser=true; laserKill.profile.weapon=PrimaryWeapon::Laser;
        laserKill.start(0); laserKill.skipScene(); enemy(laserKill,{9,0},0.2f);
        ticks(laserKill,20,fire);
        require(laserKill.kills==1 && laserKill.profile.sol==25,"Laser kills must use the same Sol reward logic as pulse shots");
        Level battery; battery.profile.laser=true; battery.start(0); battery.skipScene();
        ticks(battery,120,fire);
        require(battery.beamActive && std::abs(battery.player.energy-70)<0.1f,"One second of RMB laser must consume 30 Energy");
        ticks(battery,480,fire);
        require(!battery.beamActive && battery.player.energy==0 && battery.player.laserExhausted,"Holding RMB must exhaust Energy and stop the laser");
        ticks(battery,120,fire);
        require(!battery.beamActive && battery.player.energy==0,"Keeping RMB held must not recharge into an infinite flickering beam");
        ticks(battery,120,{});
        require(std::abs(battery.player.energy-19)<0.1f && !battery.player.laserExhausted,"Releasing RMB must restore Energy and clear laser lockout");
        battery.update(dt,fire); require(battery.beamActive,"Laser must fire again after release and recharge");
        Level shared; shared.profile.laser=true; shared.start(0); shared.skipScene();
        FlightInput both=fire; both.boost=true;
        ticks(shared,120,both);
        require(std::abs(shared.player.energy-38)<0.15f,"Laser and acceleration must drain the same battery without regeneration");
        Level unowned; unowned.start(0); unowned.skipScene(); ticks(unowned,120,fire);
        require(unowned.player.energy==100 && !unowned.beamActive,"RMB must not consume Energy before buying the laser");
        Level dps; dps.profile.laser=true; dps.start(0); dps.skipScene(); enemy(dps,{14,0},10,true);
        ticks(dps,30,fire);
        require(std::abs(dps.enemies[0].health-9.0f)<0.01f,"Laser damage must be raised to four hull points per second");
        Level missile; missile.profile.missiles=true; missile.start(0); missile.skipScene();
        enemy(missile,{12,4},2);
        FlightInput launch; launch.missile=true;
        ticks(missile,179,launch);
        require(missile.bullets.empty() && missile.missileLockTime>1.4f && missile.player.energy==100,"Missiles must spend 1.5 seconds locking without charging Energy early");
        ticks(missile,3,launch);
        require(missile.bullets.size()==1 && missile.bullets[0].homing && missile.bullets[0].velocity.y>0,"Missiles must launch and steer toward off-axis enemies");
        require(missile.player.energy>=80 && missile.player.energy<80.5f,"Missile launch must deduct exactly 20 Energy");
        missile.update(dt,launch);
        require(missile.bullets.size()==1,"Missile cooldown must prevent launching every frame");
        ticks(missile,240,{});
        require(missile.kills==1 && missile.profile.sol==25,"Homing missile must reach and destroy a moving target");
        Level locked; locked.start(0); locked.skipScene(); enemy(locked,{10,2}); locked.update(dt,launch);
        require(locked.bullets.empty(),"Missiles cannot fire before purchasing the launcher");
        Level noTarget; noTarget.profile.missiles=true; noTarget.start(0); noTarget.skipScene(); noTarget.update(dt,launch);
        require(noTarget.bullets.empty() && noTarget.player.missileCooldown==0,"No-target firing must not waste the missile cooldown");
        Level power; power.profile.workshopUnlocked=true; power.profile.sol=5000; power.openWorkshop();
        for(int i=0;i<5;++i) require(power.buy(Upgrade::Energy),"All five Energy upgrade tiers must be purchasable");
        require(power.profile.sol==4250 && power.profile.maxEnergy()==225 && power.player.maxEnergy==225,"Energy tiers must cost 50/100/150/200/250 Sol and increase capacity to 225");
        require(!power.buy(Upgrade::Energy) && power.profile.sol==4250,"Maximum Energy tier must not take another payment");
        power.start(0); power.skipScene();
        require(power.player.energy==225 && power.player.maxHull==100,"Energy upgrade must persist without altering Hull");
        power.player.energy=220; ticks(power,120,{});
        require(power.player.energy==225,"Battery regeneration must clamp to upgraded capacity");
        power.profile.laser=true; ticks(power,600,fire);
        require(std::abs(power.player.energy-75)<0.1f,"Expanded battery must power the laser beyond the original capacity");
        Level debris; debris.start(0); debris.skipScene(); enemy(debris,{2,0},1);
        debris.bullets.push_back({{0,0},{0,0},{600,0},false,1}); debris.update(dt,{});
        int shards=0; bool left=false,right=false,up=false,down=false;
        for(const auto& piece:debris.particles) if(piece.shard) {
            ++shards; left=left||piece.velocity.x<0; right=right||piece.velocity.x>0;
            up=up||piece.velocity.y>0; down=down||piece.velocity.y<0;
        }
        require(shards==12 && debris.explosions.size()==1 && left && right && up && down,"Enemy destruction must create one flash and armor fragments flying to all sides");
        const auto frozenPieces=debris.particles; const float blastAge=debris.explosions[0].age;
        debris.pause(); debris.update(1,{});
        require(debris.explosions[0].age==blastAge && debris.particles[0].position==frozenPieces[0].position,"Pause must freeze explosions and fragments");
        debris.resume(); ticks(debris,240,{});
        require(debris.explosions.empty() && debris.particles.empty(),"Explosion effects and fragments must expire without accumulating");
        Level cancel; cancel.profile.missiles=true; cancel.start(0); cancel.skipScene(); enemy(cancel,{25,4});
        ticks(cancel,120,launch); cancel.update(dt,{});
        require(cancel.missileTargetId==0 && cancel.missileLockTime==0 && cancel.player.energy==100,"Releasing E must cancel the lock without charging Energy");
        cancel.enemies.front().position.x=cancel.player.position.x+20;
        ticks(cancel,120,launch); require(cancel.bullets.empty(),"A new hold must acquire a fresh full-duration lock");
        Level retarget; retarget.profile.missiles=true; retarget.start(0); retarget.skipScene(); enemy(retarget,{20,4}); enemy(retarget,{25,-4});
        ticks(retarget,120,launch); const auto firstTarget=retarget.missileTargetId;
        for(auto& target:retarget.enemies) if(target.id==firstTarget) target.health=0;
        retarget.update(dt,launch);
        require(retarget.missileTargetId!=0 && retarget.missileTargetId!=firstTarget && retarget.missileLockTime<0.1f,"Losing a target must restart the lock on another ship");
        Level hungry; hungry.profile.missiles=true; hungry.profile.laser=true; hungry.start(0); hungry.skipScene(); enemy(hungry,{25,4});
        hungry.player.energy=25; FlightInput drainAndLock=launch; drainAndLock.laser=true;
        ticks(hungry,250,drainAndLock);
        require(hungry.bullets.empty() && hungry.missileLowEnergy && hungry.player.energy==0,"A completed lock must wait when fewer than 20 Energy remain");
        hungry.player.energy=20; hungry.update(dt,launch);
        require(!hungry.bullets.empty() && hungry.player.energy<1,"Energy is charged only when a locked missile actually launches");
        const auto directory=std::filesystem::temp_directory_path()/("red-drift-tests-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        const auto path=directory/"progress.txt";
        Profile saved=economy.profile; saved.sol=423; saved.weapon=PrimaryWeapon::Laser; saved.energyTier=3;
        std::array<bool,6> completed{true,true,false,false,false,false};
        require(ProgressStore::save(path,saved,completed),"Progress save must create a new file");
        Profile loaded; std::array<bool,6> levels{};
        require(ProgressStore::load(path,loaded,levels) && loaded.sol==423 && loaded.hullTier==5 && loaded.energyTier==3 && loaded.laserLevel()==5 && loaded.missileLevel()==5 && loaded.workshopUnlocked && levels==completed,"Saving and loading must preserve wallet, weapon levels, upgrades, unlock and completed levels");
        saved.sol=398;
        require(ProgressStore::save(path,saved,completed) && ProgressStore::load(path,loaded,levels) && loaded.sol==398,"Save replacement must work when the progress file already exists");
        { std::ofstream corrupt(path); corrupt<<"RED_DRIFT_SAVE 1\n-1 99 1 1 1 1\n"; }
        require(!ProgressStore::load(path,loaded,levels) && loaded.sol==398,"Invalid saves must be rejected without partially replacing the profile");
        { std::ofstream legacy(path); legacy<<"RED_DRIFT_SAVE 1\n321 2 1 1 1 1\n1 1 0 0 0 0\n"; }
        require(ProgressStore::load(path,loaded,levels) && loaded.sol==321 && loaded.hullTier==2 && loaded.energyTier==0 && loaded.laserLevel()==1 && loaded.missileLevel()==1 && levels[1],"Old version-one saves must preserve Sol and convert installed weapons to level one");
        { std::ofstream legacy(path); legacy<<"RED_DRIFT_SAVE 2\n654 3 2 1 1 1 1\n1 1 1 0 0 0\n"; }
        require(ProgressStore::load(path,loaded,levels) && loaded.sol==654 && loaded.energyTier==2 && loaded.laserLevel()==1 && loaded.missileLevel()==1,"Version-two saves must retain ship tiers and map purchased weapons to level one");
        std::filesystem::remove(path); std::filesystem::remove(directory);
        std::cout<<"PASS: workshop unlock, purchases, hull, Sol rewards, laser, homing missiles, save/load\n";
        return 0;
    } catch(const std::exception& error) { std::cerr<<"FAIL: "<<error.what()<<'\n'; return 1; }
}
