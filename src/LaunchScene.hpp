#pragma once
#include <algorithm>

namespace LaunchScene {
inline float ease(float value) {
    const float t = std::clamp(value, 0.0f, 1.0f);
    return t*t*(3.0f-2.0f*t);
}
struct Frame {
    float door, shipX, screenX, zoom, engine, bars;
    float x(float worldX) const { return screenX+(worldX-shipX)*zoom; }
    float y(float worldY) const { return 370.0f+(worldY-370.0f)*zoom; }
};
inline Frame sample(float seconds) {
    return {
        ease((seconds-0.2f)/1.2f),
        220.0f+180.0f*ease((seconds-1.4f)/2.0f)+1200.0f*ease((seconds-2.5f)/3.0f),
        350.0f+146.0f*ease(seconds/5.5f),
        1.0f-0.5f*ease((seconds-2.8f)/2.7f),
        ease((seconds-1.1f)/0.9f),
        1.0f-ease((seconds-4.7f)/0.8f)
    };
}
}
