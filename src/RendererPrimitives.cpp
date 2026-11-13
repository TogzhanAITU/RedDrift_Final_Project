#include "Renderer.hpp"
#include "RenderStyle.hpp"
#include <algorithm>
#include <array>
#include <cmath>

using namespace RenderStyle;

namespace {
const auto& circlePoints(int sides) {
    static const auto circles=[] {
        std::array<std::array<glm::vec2,49>,49> result{};
        for(int count=3;count<=48;++count) for(int i=0;i<=count;++i) {
            const float angle=2*Pi*i/count;
            result[count][i]={std::cos(angle),std::sin(angle)};
        }
        return result;
    }();
    return circles[sides];
}
std::array<unsigned char, 7> glyph(char c) {
    switch (c) {
    case 'a': return {0,0,14,1,15,17,15}; case 'b': return {16,16,30,17,17,17,30};
    case 'c': return {0,0,14,17,16,17,14}; case 'd': return {1,1,15,17,17,17,15};
    case 'e': return {0,0,14,17,31,16,14}; case 'f': return {6,9,8,28,8,8,8};
    case 'g': return {0,15,17,17,15,1,14}; case 'h': return {16,16,30,17,17,17,17};
    case 'i': return {4,0,12,4,4,4,14}; case 'j': return {2,0,6,2,2,18,12};
    case 'k': return {16,16,18,20,24,20,18}; case 'l': return {12,4,4,4,4,4,14};
    case 'm': return {0,0,26,21,21,21,21}; case 'n': return {0,0,30,17,17,17,17};
    case 'o': return {0,0,14,17,17,17,14}; case 'p': return {0,30,17,17,30,16,16};
    case 'q': return {0,15,17,17,15,1,1}; case 'r': return {0,0,22,25,16,16,16};
    case 's': return {0,0,15,16,14,1,30}; case 't': return {8,8,28,8,8,9,6};
    case 'u': return {0,0,17,17,17,19,13}; case 'v': return {0,0,17,17,17,10,4};
    case 'w': return {0,0,17,17,21,21,10}; case 'x': return {0,0,17,10,4,10,17};
    case 'y': return {0,17,17,17,15,1,14}; case 'z': return {0,0,31,2,4,8,31};
    case '!': return {4,4,4,4,4,0,4}; case 39: return {4,4,8,0,0,0,0};
    case '>': return {16,8,4,2,4,8,16};
    case 'A': return {14,17,17,31,17,17,17}; case 'B': return {30,17,17,30,17,17,30};
    case 'C': return {14,17,16,16,16,17,14}; case 'D': return {30,17,17,17,17,17,30};
    case 'E': return {31,16,16,30,16,16,31}; case 'F': return {31,16,16,30,16,16,16};
    case 'G': return {14,17,16,23,17,17,15}; case 'H': return {17,17,17,31,17,17,17};
    case 'I': return {31,4,4,4,4,4,31}; case 'J': return {7,2,2,2,18,18,12};
    case 'K': return {17,18,20,24,20,18,17}; case 'L': return {16,16,16,16,16,16,31};
    case 'M': return {17,27,21,21,17,17,17}; case 'N': return {17,25,21,19,17,17,17};
    case 'O': return {14,17,17,17,17,17,14}; case 'P': return {30,17,17,30,16,16,16};
    case 'Q': return {14,17,17,17,21,18,13}; case 'R': return {30,17,17,30,20,18,17};
    case 'S': return {15,16,16,14,1,1,30}; case 'T': return {31,4,4,4,4,4,4};
    case 'U': return {17,17,17,17,17,17,14}; case 'V': return {17,17,17,17,17,10,4};
    case 'W': return {17,17,17,21,21,21,10}; case 'X': return {17,17,10,4,10,17,17};
    case 'Y': return {17,17,10,4,4,4,4}; case 'Z': return {31,1,2,4,8,16,31};
    case '0': return {14,17,19,21,25,17,14}; case '1': return {4,12,4,4,4,4,14};
    case '2': return {14,17,1,2,4,8,31}; case '3': return {30,1,1,14,1,1,30};
    case '4': return {2,6,10,18,31,2,2}; case '5': return {31,16,16,30,1,1,30};
    case '6': return {14,16,16,30,17,17,14}; case '7': return {31,1,2,4,8,8,8};
    case '8': return {14,17,17,14,17,17,14}; case '9': return {14,17,17,15,1,1,14};
    case ':': return {0,4,4,0,4,4,0}; case '.': return {0,0,0,0,0,6,6};
    case '/': return {1,2,2,4,8,8,16}; case '-': return {0,0,0,31,0,0,0};
    case '+': return {0,4,4,31,4,4,0}; case '%': return {17,2,4,4,8,16,17};
    case '[': return {14,8,8,8,8,8,14}; case ']': return {14,2,2,2,2,2,14};
    default: return {};
    }
}
}

void Renderer::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color) {
    vertices.push_back({a,color}); vertices.push_back({b,color}); vertices.push_back({c,color});
}

void Renderer::rect(float x, float y, float w, float h, glm::vec4 color) {
    triangle({x,y},{x+w,y},{x+w,y+h},color); triangle({x,y},{x+w,y+h},{x,y+h},color);
}

void Renderer::disc(glm::vec2 p, float rx, float ry, glm::vec4 color, int sides) {
    if(color.a<=0) return;
    sides=std::clamp(sides,3,48);
    const auto& points=circlePoints(sides);
    const glm::vec2 radius(rx,ry);
    for (int i = 0; i < sides; ++i) {
        triangle(p,p+points[i]*radius,p+points[i+1]*radius,color);
    }
}

void Renderer::line(glm::vec2 a, glm::vec2 b, float width, glm::vec4 color) {
    if (color.a<=0 || glm::length(b-a) < 0.001f) return;
    const glm::vec2 n = glm::normalize(glm::vec2(-(b-a).y, (b-a).x)) * width * 0.5f;
    triangle(a+n,a-n,b-n,color); triangle(a+n,b-n,b+n,color);
}

void Renderer::text(const std::string& value, float x, float y, float scale, glm::vec4 color) {
    if(color.a<=0) return;
    for (char c : value) {
        const auto rows = glyph(c);
        for (int r = 0; r < 7; ++r) for (int col = 0; col < 5; ++col)
            if (rows[r] & (1 << (4-col))) rect(x+col*scale,y+r*scale,scale,scale,color);
        x += 6.0f*scale;
    }
}

void Renderer::ship(glm::vec2 position, float scale, float tilt, bool enemy, bool heavy,
                    float, bool, float, glm::vec2, bool reverseFacing) {
    const float direction = (enemy ? -1.0f : 1.0f) * (reverseFacing ? -1.0f : 1.0f);
    const float cosine = std::cos(tilt), sine = std::sin(tilt);
    const auto point = [&](float x, float y) {
        x *= direction;
        return position + glm::vec2(x * cosine - y * sine, x * sine + y * cosine) * scale;
    };
    const glm::vec4 color = enemy ? (heavy ? Orange : glm::vec4(0.7f, 0.45f, 0.8f, 1)) : Cyan;
    triangle(point(0.85f, 0), point(-0.85f, -0.425f), point(-0.85f, 0.425f), color);
}

void Renderer::moon(glm::vec2 position, float radius) {
    disc(position, radius, radius, {0.45f, 0.48f, 0.52f, 1}, 16);
}

void Renderer::martianFighter(glm::vec2 position, float scale, float tilt, float time) {
    ship(position, scale, tilt, false, false, time, false);
}
