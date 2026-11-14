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

void Renderer::ship(glm::vec2 p, float s, float tilt, bool enemy, bool heavy, float time, bool boost, float engine, glm::vec2 maneuverJets, bool reverseFacing) {
    const float direction = (enemy ? -1.0f : 1.0f)*(reverseFacing?-1.0f:1.0f);
    const float cosine=std::cos(tilt),sine=std::sin(tilt);
    auto point = [&](float x, float y) {
        x *= direction;
        return p + glm::vec2(x*cosine-y*sine,x*sine+y*cosine)*s;
    };
    auto tri = [&](glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec4 color) {
        triangle(point(a.x,a.y),point(b.x,b.y),point(c.x,c.y),color);
    };
    auto quad = [&](glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, glm::vec4 color) {
        tri(a,b,c,color); tri(a,c,d,color);
    };
    auto box = [&](float x, float y, float w, float h, glm::vec4 color) {
        quad({x,y},{x+w,y},{x+w,y+h},{x,y+h},color);
    };
    auto beam = [&](glm::vec2 a, glm::vec2 b, float width, glm::vec4 color) {
        line(point(a.x,a.y),point(b.x,b.y),width*s,color);
    };

    const glm::vec4 accent = enemy ? (heavy ? Orange : glm::vec4(0.75f,0.48f,0.78f,1)) : Cyan;
    const glm::vec4 armor = enemy ? glm::vec4(0.37f,0.39f,0.40f,1) : glm::vec4(0.49f,0.54f,0.52f,1);
    const glm::vec4 top = enemy ? glm::vec4(0.55f,0.56f,0.53f,1) : glm::vec4(0.70f,0.73f,0.67f,1);
    const glm::vec4 side = enemy ? glm::vec4(0.24f,0.27f,0.29f,1) : glm::vec4(0.30f,0.36f,0.36f,1);
    const glm::vec4 dark(0.08f,0.12f,0.14f,1), edge(0.18f,0.23f,0.24f,1);
    const glm::vec4 exhaust = enemy ? glm::vec4(1,0.46f,0.24f,1) : glm::vec4(0.34f,0.77f,1,1);

    const int engines = heavy ? 3 : 2;
    for (int i=0;i<engines;++i) {
        const float y = heavy ? -0.19f+i*0.19f : -0.15f+i*0.30f;
        const float flame = ((boost ? 0.8f : 0.34f) + 0.055f*std::sin(time*43+i*2.0f))*engine;
        if (engine > 0.01f) {
        disc(point(-0.97f,y),s*(boost?0.48f:0.23f),s*0.13f,{exhaust.r,exhaust.g,exhaust.b,0.10f*engine},16);
        quad({-0.93f,y-0.065f},{-0.93f,y+0.065f},{-1.0f-flame,y+0.022f},{-1.08f-flame,y},exhaust);
        tri({-0.96f,y-0.032f},{-0.96f,y+0.032f},{-1.0f-flame*0.68f,y},White);
        }
        box(-0.95f,y-0.10f,0.25f,0.20f,edge);
        box(-0.94f,y-0.07f,0.06f,0.14f,dark);
        beam({-0.93f,y-0.05f},{-0.93f,y+0.05f},0.025f,exhaust);
        box(-0.85f,y-0.09f,0.14f,0.035f,top);
    }

    quad({-0.80f,-0.29f},{0.52f,-0.29f},{0.94f,0.10f},{-0.80f,0.25f},dark);
    quad({-0.77f,-0.25f},{0.48f,-0.25f},{0.87f,0.08f},{-0.77f,0.13f},armor);
    quad({-0.77f,-0.25f},{0.48f,-0.25f},{0.62f,-0.13f},{-0.77f,-0.13f},top);
    quad({-0.78f,0.03f},{0.84f,0.03f},{0.92f,0.19f},{-0.69f,0.27f},side);
    beam({-0.75f,0.04f},{0.78f,0.04f},0.028f,dark);
    beam({-0.67f,0.23f},{0.86f,0.16f},0.022f,armor);

    for (int i=0;i<3;++i) {
        const float x=-0.32f+i*0.25f;
        box(x,-0.11f,0.21f,0.13f,side);
        box(x+0.025f,-0.095f,0.15f,0.085f,armor);
        beam({x+0.02f,-0.09f},{x+0.17f,-0.09f},0.015f,top);
    }
    quad({0.49f,-0.23f},{0.61f,-0.15f},{0.88f,0.07f},{0.63f,0.07f},top);
    beam({0.54f,-0.19f},{0.77f,0.04f},0.025f,side);
    box(0.82f,-0.045f,0.16f,0.09f,dark);
    beam({0.96f,-0.027f},{0.96f,0.027f},0.026f,accent);

    box(-0.67f,-0.39f,0.35f,0.18f,side);
    quad({-0.64f,-0.40f},{-0.39f,-0.40f},{-0.31f,-0.32f},{-0.67f,-0.32f},top);
    box(-0.60f,-0.31f,0.22f,0.045f,dark);
    box(-0.57f,-0.30f,0.15f,0.022f,accent);
    beam({-0.51f,-0.40f},{-0.51f,-0.49f},0.023f,edge);
    beam({-0.56f,-0.47f},{-0.43f,-0.47f},0.021f,top);

    auto turret = [&](float x, float y, bool underside) {
        const float sign = underside ? 1.0f : -1.0f;
        box(x-0.11f,y-0.045f,0.23f,0.09f,edge);
        box(x-0.075f,y+(underside?0.0f:-0.10f),0.16f,0.10f,armor);
        beam({x-0.06f,y+sign*0.09f},{x+0.07f,y+sign*0.09f},0.026f,top);
        beam({x+0.06f,y+sign*0.065f},{x+0.29f,y+sign*0.065f},0.042f,side);
        beam({x+0.24f,y+sign*0.065f},{x+0.31f,y+sign*0.065f},0.045f,top);
    };
    turret(0.10f,-0.25f,false);
    if (heavy) {
        turret(0.43f,-0.24f,false);
        turret(-0.12f,0.28f,true);
        box(-0.60f,0.20f,0.26f,0.16f,side);
        box(-0.56f,0.22f,0.18f,0.09f,edge);
    }
    for (int i=0;i<3;++i) box(-0.70f+i*0.065f,-0.12f,0.028f,0.18f,dark);
    box(0.44f,-0.10f,0.045f,0.12f,accent);
    box(-0.36f,0.15f,0.07f,0.025f,accent);
    if(!enemy) for(int jetSign=-1;jetSign<=1;jetSign+=2) {
        const float power=jetSign>0?maneuverJets.x:maneuverJets.y;
        const float y=jetSign>0?0.25f:-0.26f;
        for(float x : {-0.64f,0.43f}) {
            box(x-0.055f,y-0.025f,0.11f,0.05f,edge);
            if(power<0.01f) continue;
            const float reach=(0.10f+0.35f*power)*(1+0.12f*std::sin(time*55+x*8));
            disc(point(x,y+jetSign*reach*0.35f),s*0.18f,s*0.18f,{0.2f,0.75f,1,0.12f*power},16);
            quad({x-0.05f,y},{x+0.05f,y},{x+0.022f,y+jetSign*reach*0.65f},{x-0.022f,y+jetSign*reach*0.65f},{0.25f,0.8f,1,power});
            tri({x-0.025f,y},{x+0.025f,y},{x,y+jetSign*reach*0.8f},{0.85f,1,1,power});
        }
    }
}

void Renderer::moon(glm::vec2 p, float radius) {
    const float s=radius/156.0f;
    disc(p,radius,radius,{0.5f,0.53f,0.58f,1});
    disc(p+glm::vec2(-25,-24)*s,130*s,131*s,{0.62f,0.65f,0.69f,1});
    disc(p+glm::vec2(-75,-50)*s,28*s,32*s,{0.41f,0.44f,0.49f,1});
    disc(p+glm::vec2(24,45)*s,38*s,25*s,{0.45f,0.48f,0.53f,1});
    disc(p+glm::vec2(20,-90)*s,16*s,19*s,{0.46f,0.49f,0.54f,1});
}

void Renderer::martianFighter(glm::vec2 position,float scale,float tilt,float time) {
    const float cosine=std::cos(tilt),sine=std::sin(tilt);
    auto point=[&](float x,float y) { return position+glm::vec2(x*cosine-y*sine,x*sine+y*cosine)*scale; };
    auto tri=[&](glm::vec2 a,glm::vec2 b,glm::vec2 c,glm::vec4 color) { triangle(point(a.x,a.y),point(b.x,b.y),point(c.x,c.y),color); };
    const glm::vec4 armor(0.30f,0.31f,0.28f,1),light(0.56f,0.54f,0.44f,1),shadow(0.11f,0.15f,0.16f,1);
    for(int side=-1;side<=1;side+=2) {
        const float sign=static_cast<float>(side);
        tri({-0.1f,sign*0.1f},{-1.12f,sign*1.08f},{0.62f,sign*0.72f},armor);
        tri({-0.1f,sign*0.1f},{-1.12f,sign*1.08f},{-0.48f,sign*0.4f},light);
        tri({0.15f,sign*0.21f},{0.62f,sign*0.72f},{0.64f,sign*0.24f},shadow);
        line(point(-0.85f,sign*0.88f),point(0.25f,sign*0.62f),1.4f,{0.7f,0.64f,0.46f,1});
        line(point(-0.55f,sign*0.52f),point(0.1f,sign*0.42f),2,shadow);
        tri({0.38f,sign*0.18f},{0.77f,sign*0.63f},{0.92f,sign*0.52f},light);
        disc(point(-0.91f,sign*0.94f),2,2,Cyan,12);
    }
    tri({-1.0f,0},{0.54f,-0.32f},{0.74f,0.29f},shadow);
    tri({-1.0f,0},{-0.32f,-0.23f},{0.54f,-0.32f},light);
    tri({-1.0f,0},{0.74f,0.29f},{-0.26f,0.19f},armor);
    disc(point(-0.28f,-0.02f),scale*0.24f,scale*0.14f,{0.025f,0.09f,0.12f,1},20);
    line(point(-0.46f,-0.06f),point(-0.16f,-0.12f),2,{0.42f,0.75f,0.79f,1});
    for(int i=0;i<4;++i) {
        const float y=-0.27f+i*0.18f;
        const float flame=0.35f+0.07f*std::sin(time*40+i);
        tri({0.65f,y-0.075f},{0.65f,y+0.075f},{1.0f+flame,y},{0.25f,0.7f,1,0.65f});
        disc(point(0.57f,y),scale*0.19f,scale*0.13f,armor,20);
        disc(point(0.7f,y),scale*0.11f,scale*0.11f,shadow,20);
        disc(point(0.72f,y),scale*0.048f,scale*0.065f,Cyan,16);
    }
    line(point(-0.65f,0.09f),point(-0.35f,0.12f),2,Cyan);
}
