#pragma once
#include "LaunchScene.hpp"
#include <glm/glm.hpp>
#include <cmath>
#include <string>
#include <string_view>

namespace MoonScene {
inline constexpr float DeckHeight = 14.0f;
inline constexpr std::string_view ArrivalText = "Look! It's Moon! We Finally here!";
inline std::string caption(float time) {
    const auto count = static_cast<size_t>(std::max(0.0f,time-0.35f)*18.0f);
    return std::string(ArrivalText.substr(0,std::min(count,ArrivalText.size())));
}
struct EncounterFrame {
    glm::vec2 enemy, player;
    float enemyScale, playerScale, playerReveal, bars;
};
inline EncounterFrame encounter(float time) {
    const float approach = LaunchScene::ease(time/2.8f);
    const float pass = LaunchScene::ease((time-2.8f)/1.7f);
    const float handoff = LaunchScene::ease((time-3.0f)/3.0f);
    return {
        {1120-400*approach-1150*pass,250+65*approach},
        glm::mix(glm::vec2(-160,420),glm::vec2(496,370),handoff),
        5+110*approach,90-54*handoff,
        LaunchScene::ease((time-3.0f)/0.4f),
        1-LaunchScene::ease((time-5.2f)/0.8f)
    };
}
struct LandingFrame {
    glm::vec2 moon, player;
    float radius, scale, tilt, engine, legs, dust;
};
inline LandingFrame landing(float time, glm::vec2 start, float startTilt) {
    const float approach=LaunchScene::ease((time-2.5f)/7.7f);
    const float coast=LaunchScene::ease((time-2.5f)/3.0f);
    const float descent=LaunchScene::ease((time-5.5f)/4.7f);
    const float settle=LaunchScene::ease((time-10.2f)/0.8f);
    const glm::vec2 upper=glm::mix(start,glm::vec2(650,205),coast);
    return {
        {1090-190*approach,290+1450*std::sqrt(approach)},
        glm::mix(upper,glm::vec2(800,421),descent),
        156+1124*approach,36+22*approach,
        startTilt*(1-coast)+0.17f*std::sin(descent*3.14159265f),
        (1-settle)*(1-0.6f*approach),
        LaunchScene::ease((time-8.7f)/1.1f),
        LaunchScene::ease((time-9.8f)/0.5f)*(1-LaunchScene::ease((time-11.0f)/1.0f))
    };
}
inline LandingFrame takeoff(float time) {
    auto frame=landing(12.0f-9.5f*std::clamp(time/7.0f,0.0f,1.0f),{496,370},0);
    frame.tilt=-frame.tilt;
    return frame;
}
}
