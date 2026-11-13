#pragma once
#include <glm/glm.hpp>
#include "Progress.hpp"

namespace Menu {
struct Rect {
    float x, y, w, h;
    bool contains(glm::vec2 p) const { return p.x >= x && p.x <= x+w && p.y >= y && p.y <= y+h; }
};
inline Rect card(int index) { return {100.0f+(index%3)*366.0f, 176.0f+(index/3)*172.0f, 348, 150}; }
inline constexpr Rect Start{480,554,320,48};
inline constexpr Rect Primary{480,456,320,46};
inline constexpr Rect Back{540,524,200,32};
inline constexpr Rect Workshop{982,96,198,40};
inline constexpr Rect ShopBack{480,614,320,44};
inline Rect shopTab(int index) { return {200.0f+index*460.0f,150,420,44}; }
inline Rect shopCard(int index) { return {200.0f+index*460.0f,222,420,310}; }
inline Rect buyButton(int index) { const auto card=shopCard(index); return {card.x+20,card.y+244,380,44}; }
inline Upgrade shopUpgrade(int tab,int index) { return tab==0?(index==0?Upgrade::Hull:Upgrade::Energy):(index==0?Upgrade::Laser:Upgrade::Missiles); }
inline int cardAt(glm::vec2 p) {
    for(int i=0;i<6;++i) if(card(i).contains(p)) return i;
    return -1;
}
inline glm::vec2 cursor(double x, double y, int width, int height) {
    const float fit = glm::min(width/1280.0f, height/720.0f);
    if(fit <= 0) return {-1,-1};
    return {(static_cast<float>(x)-(width-1280*fit)*0.5f)/fit,
            (static_cast<float>(y)-(height-720*fit)*0.5f)/fit};
}
}
