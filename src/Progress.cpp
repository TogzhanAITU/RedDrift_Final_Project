#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "Progress.hpp"
#include <fstream>
#include <string>

bool ProgressStore::load(const std::filesystem::path& path, Profile& profile, std::array<bool,6>& completed) {
    std::ifstream file(path);
    std::string magic; int version=0,laser=0,missiles=0,unlocked=0,weapon=0;
    Profile loaded;
    std::array<bool,6> levels{};
    if(!(file>>magic>>version) || magic!="RED_DRIFT_SAVE" || version<1 || version>3) return false;
    if(!(file>>loaded.sol>>loaded.hullTier)) return false;
    if(version>=2 && !(file>>loaded.energyTier)) return false;
    if(!(file>>laser>>missiles>>unlocked>>weapon)) return false;
    if(version>=3) {
        if(!(file>>loaded.laserTier>>loaded.missileTier)) return false;
        if(loaded.laserTier<0 || loaded.laserTier>5 || loaded.missileTier<0 || loaded.missileTier>5 ||
           (laser==0)!=(loaded.laserTier==0) || (missiles==0)!=(loaded.missileTier==0)) return false;
    } else { loaded.laserTier=laser; loaded.missileTier=missiles; }
    if(loaded.sol<0 || loaded.sol>1000000000 || loaded.hullTier<0 || loaded.hullTier>5 ||
       loaded.energyTier<0 || loaded.energyTier>5 || laser<0 || laser>1 || missiles<0 || missiles>1 || unlocked<0 || unlocked>1 || weapon<0 || weapon>1) return false;
    for(auto& level:levels) { int value; if(!(file>>value) || value<0 || value>1) return false; level=value!=0; }
    loaded.laser=laser!=0; loaded.missiles=missiles!=0; loaded.workshopUnlocked=unlocked!=0;
    if(!loaded.workshopUnlocked && (loaded.laser || loaded.missiles || loaded.hullTier>0 || loaded.energyTier>0)) return false;
    loaded.weapon=weapon==1 && loaded.laser ? PrimaryWeapon::Laser : PrimaryWeapon::Pulse;
    profile=loaded; completed=levels;
    return true;
}

bool ProgressStore::save(const std::filesystem::path& path, const Profile& profile, const std::array<bool,6>& completed) {
    std::error_code error;
    if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(),error);
    if(error) return false;
    auto temporary=path; temporary+=".tmp";
    {
        std::ofstream file(temporary,std::ios::trunc);
        if(!file) return false;
        file<<"RED_DRIFT_SAVE 3\n"<<profile.sol<<' '<<profile.hullTier<<' '<<profile.energyTier<<' '<<profile.laser<<' '
            <<profile.missiles<<' '<<profile.workshopUnlocked<<' '<<static_cast<int>(profile.weapon)<<' '
            <<profile.laserLevel()<<' '<<profile.missileLevel()<<'\n';
        for(bool level:completed) file<<level<<' ';
        file<<'\n'; file.flush();
        if(!file) return false;
        file.close(); if(!file) return false;
    }
#ifdef _WIN32
    return MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
    std::filesystem::rename(temporary,path,error);
    return !error;
#endif
}
