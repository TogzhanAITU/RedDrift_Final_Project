#pragma once
#include <array>
#include <filesystem>
#include <algorithm>
#include <string>

enum class PrimaryWeapon { Pulse, Laser };
enum class Upgrade { Hull, Laser, Missiles, Energy };
struct Profile {
    int sol = 0, hullTier = 0, energyTier = 0;
    int laserTier = 0, missileTier = 0;
    bool laser = false, missiles = false, workshopUnlocked = false;
    PrimaryWeapon weapon = PrimaryWeapon::Pulse;
    int maxHull() const { return 100 + 25*hullTier; }
    int maxEnergy() const { return 100 + 25*energyTier; }
    int laserLevel() const { return laser?std::max(1,laserTier):0; }
    int missileLevel() const { return missiles?std::max(1,missileTier):0; }
    float laserDamage() const { return laser?4.0f+2.0f*(laserLevel()-1):0.0f; }
    float rockDamage() const { return laser?6.0f+1.5f*(laserLevel()-1):0.0f; }
    float missileDamage() const { return missiles?2.0f+missileLevel():0.0f; }
    int missileCount() const { return missileLevel()>=3?2:1; }
    float lockDuration() const { return missileLevel()>=5?1.0f:1.5f; }
    float missileSpeed() const { return missileLevel()>=5?30.0f:22.0f; }
    int price(Upgrade item) const {
        if(item==Upgrade::Hull) return hullTier<5 ? 60*(hullTier+1) : 0;
        if(item==Upgrade::Energy) return energyTier<5 ? 50*(energyTier+1) : 0;
        if(item==Upgrade::Laser) return laserLevel()<5?100*(laserLevel()+1):0;
        return missileLevel()<5?150*(missileLevel()+1):0;
    }
    std::array<std::string,3> nextUpgrade(Upgrade item) const {
        if(price(item)==0) return {"MAXIMUM LEVEL REACHED","",""};
        if(item==Upgrade::Hull) return {"NEXT: HULL "+std::to_string(maxHull())+" > "+std::to_string(maxHull()+25),"+25 MAXIMUM HULL",""};
        if(item==Upgrade::Energy) return {"NEXT: ENERGY "+std::to_string(maxEnergy())+" > "+std::to_string(maxEnergy()+25),"+25 BATTERY CAPACITY",""};
        if(item==Upgrade::Laser) return {"NEXT: DAMAGE "+std::to_string(static_cast<int>(laserDamage()))+" > "+std::to_string(4+2*laserLevel())+" / SEC","STRONGER CONTINUOUS BEAM","FASTER ROCK DESTRUCTION"};
        const int next=missileLevel()+1;
        const std::string damage="NEXT: DAMAGE "+std::to_string(static_cast<int>(missileDamage()))+" > "+std::to_string(next+2);
        if(next==3) return {damage,"NEW: 2 MISSILES / 2 TARGETS","20 ENERGY EACH / 40 PER SALVO"};
        if(next==5) return {damage,"NEW: LOCK 1.5S > 1.0S","SPEED 22 > 30 / SALVO 2"};
        return {damage,next>=3?"SALVO: 2 DISTINCT TARGETS":"SALVO: 1 TARGET",next>=3?"40 ENERGY FOR TWO MISSILES":"20 ENERGY PER MISSILE"};
    }
};
namespace ProgressStore {
bool load(const std::filesystem::path& path, Profile& profile, std::array<bool,6>& completed);
bool save(const std::filesystem::path& path, const Profile& profile, const std::array<bool,6>& completed);
}
