#pragma once
#include "LaunchScene.hpp"
#include <glm/glm.hpp>
#include <cmath>

namespace MarsScene {
inline constexpr int Pursuers = 8, Fighters = 5;
inline constexpr float EndCardStart = 25.0f, Duration = 28.0f;
inline float destructionTime(int index) { return index>=3 && index<6?12.8f:index<3?17.2f:18.4f; }
inline glm::vec2 pursuer(float time,int index) {
    const float entry=LaunchScene::ease((time-2.0f-index*0.12f)/3.0f);
    const glm::vec2 formation(155+(index%4)*142.0f,220+(index/4)*120.0f);
    const auto pursuit=glm::mix(glm::vec2(-170-index*70.0f,formation.y-60),formation,entry);
    const float angle=3.14159265f+index*6.2831853f/Pursuers;
    const auto ring=glm::vec2(700,330)+glm::vec2(std::cos(angle)*240,std::sin(angle)*155);
    return glm::mix(pursuit,ring,LaunchScene::ease((time-6.5f)/4.0f));
}
inline glm::vec2 fighter(float time,int index) {
    const float entry=LaunchScene::ease((time-12.0f-index*0.06f)/2.5f);
    const float leave=LaunchScene::ease((time-21.0f-index*0.12f)/2.7f);
    return {glm::mix(1450+index*35.0f,1180+(index%2)*75.0f,entry)-1700*leave,
            190+index*62.0f+std::sin(time*1.3f+index)*9-100*leave};
}
inline glm::vec2 cruiser(float time,glm::vec2 start) {
    auto position=glm::mix(start,glm::vec2(850,405),LaunchScene::ease(time/4.0f));
    position=glm::mix(position,glm::vec2(700,330),LaunchScene::ease((time-6.5f)/4.0f));
    const float evade=LaunchScene::ease((time-3.5f)/0.5f)*(1-LaunchScene::ease((time-7.2f)/1.0f));
    position+=glm::vec2(28*std::sin((time-3.5f)*2.6f),82*std::sin((time-3.5f)*3.3f))*evade;
    return position+glm::vec2(330,85)*LaunchScene::ease((time-20.5f)/4.5f);
}
struct Camera {
    glm::vec2 center;
    float zoom, reveal;
    glm::vec2 project(glm::vec2 p) const { return glm::vec2(640,360)+(p-center)*zoom; }
};
inline Camera camera(float time) {
    const float reveal=LaunchScene::ease((time-11.95f)/0.6f)*(1-LaunchScene::ease((time-14.6f)/1.4f));
    const auto approach=glm::mix(glm::vec2(1280,330),glm::vec2(1210,330),LaunchScene::ease((time-12)/2.5f));
    return {glm::mix(glm::vec2(640,360),approach,reveal),1+0.45f*reveal,reveal};
}
inline float enemyHeading(float time,int index,glm::vec2 start) {
    const auto p=pursuer(time,index);
    const auto initial=cruiser(time,start)-p;
    const auto target=fighter(time,index%Fighters)-p;
    const float from=std::atan2(initial.y,initial.x),to=std::atan2(target.y,target.x);
    const float turn=std::atan2(std::sin(to-from),std::cos(to-from));
    return from+turn*LaunchScene::ease((time-12.4f)/0.8f);
}
struct Shot { glm::vec2 position{0},velocity{0}; bool active=false; };
inline Shot counterShot(float time,int enemy,int cycle,int bullet) {
    const float fired=13.25f+enemy*0.055f+cycle*0.85f+bullet*0.13f;
    const float age=time-fired;
    if(fired>=destructionTime(enemy) || age<0 || age>1.3f) return {};
    const auto p=pursuer(fired,enemy);
    const auto aim=fighter(fired+0.95f,enemy%Fighters)+glm::vec2(0,bullet?30.0f:-30.0f);
    const auto direction=glm::normalize(aim-p);
    const auto muzzle=p+direction*43.0f;
    const auto velocity=(aim-muzzle)/0.95f;
    return {muzzle+velocity*age,velocity,true};
}
inline Shot enemyShot(float time,int enemy,int cycle,int bullet,glm::vec2 start) {
    if(bullet!=0) return {};
    const float fired=4.0f+enemy*0.12f+cycle*1.5f+bullet*0.14f;
    const float age=time-fired;
    if(fired>7.2f || age<0 || age>1.5f) return {};
    const auto muzzle=pursuer(fired,enemy)+glm::vec2(43,bullet?3.0f:-3.0f);
    const float side=(enemy+bullet)%2?1.0f:-1.0f;
    const auto miss=cruiser(fired+0.8f,start)+glm::vec2(0,side*130);
    const auto velocity=(miss-muzzle)/0.8f;
    return {muzzle+velocity*age,velocity,true};
}
}
