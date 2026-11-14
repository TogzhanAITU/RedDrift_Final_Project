#include "Renderer.hpp"
#include "Level.hpp"
#include "RenderStyle.hpp"
#include <algorithm>
#include <cmath>

#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace RenderStyle;

namespace {
std::string read(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open shader: " + path.string());
    std::ostringstream out; out << file.rdbuf(); return out.str();
}
GLuint compile(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* data = source.c_str(); glShaderSource(shader, 1, &data, nullptr); glCompileShader(shader);
    GLint ok; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]{}; glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader); throw std::runtime_error(log);
    }
    return shader;
}
}

Renderer::Renderer(const std::filesystem::path& shaderDirectory) {
    const auto vertexSource = read(shaderDirectory / "vertex.glsl");
    const auto fragmentSource = read(shaderDirectory / "fragment.glsl");
    GLuint vs = compile(GL_VERTEX_SHADER, vertexSource), fs = 0;
    try { fs = compile(GL_FRAGMENT_SHADER, fragmentSource); }
    catch (...) { glDeleteShader(vs); throw; }
    program = glCreateProgram(); glAttachShader(program, vs); glAttachShader(program, fs); glLinkProgram(program);
    glDeleteShader(vs); glDeleteShader(fs);
    GLint ok; glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]{}; glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        glDeleteProgram(program); throw std::runtime_error(log);
    }
    glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo);
    glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(sizeof(glm::vec2))); glEnableVertexAttribArray(1);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    vertices.reserve(100000);
}

Renderer::~Renderer() { glDeleteBuffers(1, &vbo); glDeleteVertexArrays(1, &vao); glDeleteProgram(program); }

void Renderer::draw(const Level& game, float t, int width, int height, glm::vec2 cursor) {
    glViewport(0,0,width,height); glClearColor(0.015f,0.023f,0.045f,1); glClear(GL_COLOR_BUFFER_BIT);
    if (width <= 0 || height <= 0) return;
    const float fit = std::min(width/1280.0f, height/720.0f);
    glViewport(static_cast<int>((width-1280*fit)/2),static_cast<int>((height-720*fit)/2),static_cast<int>(1280*fit),static_cast<int>(720*fit));
    vertices.clear();
    const bool ending=(game.sceneState()==MissionState::MarsFinale || (game.state==MissionState::Won && game.levelIndex==5)) && game.finaleTime>=MarsScene::EndCardStart;
    if(ending && game.state!=MissionState::Paused) {
        rect(0,0,1280,720,{0,0,0,1});
        endCard(game);
        submit();
        return;
    }
    rect(0,0,1280,720,{0.018f,0.03f,0.065f,1});
    for (int i=10;i>0;--i) {
        disc({820,300},i*54.0f,i*24.0f,{0.19f,0.12f,0.36f,0.016f});
        disc({240,540},i*40.0f,i*24.0f,{0.02f,0.32f,0.35f,0.014f});
    }
    for (int i=0;i<180;++i) {
        const float depth = 0.15f + (i%3)*0.19f;
        float x = std::fmod(i*127.71f - game.cameraX*36*depth,1280.0f); if(x<0) x+=1280;
        const float y = std::fmod(i*93.37f,720.0f);
        const float alpha = 0.25f+(i%5)*0.12f;
        rect(x,y,(i%3==0)?2.0f:1.0f,(i%3==0)?2.0f:1.0f,{0.65f,0.81f,1,alpha});
        if (game.player.boosting && game.state==MissionState::Flying) line({x,y},{x-12,y},1,{0.6f,0.8f,1,0.2f});
    }
    const bool landingBackdrop=game.sceneState()==MissionState::Landing || (game.state==MissionState::Won && game.levelIndex==1);
    const bool finaleBackdrop=game.sceneState()==MissionState::MarsFinale || (game.state==MissionState::Won && game.levelIndex==5);
    const auto& stage = Stages[game.state == MissionState::Ready ? game.selectedLevel : game.levelIndex];
    if (stage.environment == Environment::Earth) {
        for (int i=5;i>0;--i) disc({520,1580},1330.0f+i*15,1000.0f+i*15,{0.08f,0.35f,0.66f,0.06f});
        disc({520,1580},1330,1000,{0.04f,0.18f,0.31f,1});
        disc({260,670},150,14,{0.4f,0.67f,0.82f,0.16f});
        disc({960,625},200,10,{0.4f,0.67f,0.82f,0.12f});
    } else if (stage.environment == Environment::Moon) {
        if (!landingBackdrop && game.sceneState()!=MissionState::LiftOff) {
            moon({1090,290},156);
            text("MOON",1067,470,1.5f,Muted);
        }
        if(game.levelIndex==2 && !game.cinematicActive() && game.state!=MissionState::Ready) {
            for(int i=0;i<4;++i) disc({i*350.0f,610.0f+i*12},280,25,{0.46f,0.58f,0.7f,0.035f});
        }
    } else if(stage.environment==Environment::Space) {
        const float marsRadius = 42.0f+game.progress()*130.0f;
        const glm::vec2 mars(1190.0f-game.progress()*95.0f,250.0f);
        disc(mars,marsRadius+12,marsRadius+12,{1,0.37f,0.2f,0.035f});
        disc(mars,marsRadius,marsRadius,{0.55f,0.21f,0.15f,1});
        disc(mars+glm::vec2(-marsRadius*0.16f,-marsRadius*0.13f),marsRadius*0.78f,marsRadius*0.8f,{0.73f,0.32f,0.22f,1});
        disc(mars+glm::vec2(-marsRadius*0.3f,marsRadius*0.16f),marsRadius*0.23f,marsRadius*0.13f,{0.59f,0.24f,0.18f,1});
        disc(mars+glm::vec2(marsRadius*0.18f,-marsRadius*0.4f),marsRadius*0.17f,marsRadius*0.11f,{0.62f,0.26f,0.19f,1});
        text("MARS",mars.x-24,mars.y+marsRadius+21,1.5f,Orange);
    } else if(stage.environment==Environment::Mars && !finaleBackdrop) {
        for (int i=5;i>0;--i) disc({640,1600},1400.0f+i*12,1020.0f+i*12,{0.85f,0.27f,0.12f,0.06f});
        disc({640,1600},1400,1020,{0.32f,0.12f,0.09f,1});
    }
    auto world = [&](glm::vec2 p) { return glm::vec2(640+(p.x-game.cameraX)*36,370-p.y*36); };
    line({0,104},{1280,104},1,{0.25f,0.58f,0.65f,0.15f});
    line({0,636},{1280,636},1,{0.25f,0.58f,0.65f,0.15f});
    for (const auto& rock : game.asteroids) {
        const glm::vec2 p=world(rock.position); const float r=rock.radius*36;
        if(rock.heat>0) {
            disc(p,r+5,r+5,{1,0.35f,0.12f,rock.heat*0.18f});
            rect(p.x-16,p.y+r+6,32,3,{0.12f,0.18f,0.22f,rock.heat});
            rect(p.x-16,p.y+r+6,32*std::clamp(rock.health/3.0f,0.0f,1.0f),3,{Orange.r,Orange.g,Orange.b,rock.heat});
        }
        if (rock.debris) {
            const glm::vec2 axis(std::cos(rock.rotation),std::sin(rock.rotation));
            line(p-axis*r,p+axis*r,r*0.5f,{0.18f,0.34f,0.48f,1});
            line(p-axis*r*0.2f,p+axis*r*0.2f,r*0.9f,{0.65f,0.69f,0.7f,1});
            line(p,p+glm::vec2(-axis.y,axis.x)*r,2,{0.7f,0.75f,0.8f,1});
            continue;
        }
        for(int i=0;i<9;++i) {
            float a=i*2*Pi/9+rock.rotation,b=(i+1)*2*Pi/9+rock.rotation;
            triangle(p,p+glm::vec2(std::cos(a),std::sin(a))*r*(i%2?0.86f:1.0f),p+glm::vec2(std::cos(b),std::sin(b))*r*((i+1)%2?0.86f:1.0f),{0.31f+i*0.008f,0.27f+i*0.006f,0.29f+i*0.006f,1});
        }
        disc(p+glm::vec2(-r*0.25f,-r*0.2f),r*0.25f,r*0.22f,{0.23f,0.20f,0.23f,1},12);
    }
    for (const auto& pickup : game.pickups) {
        auto p=world(pickup.position); disc(p,24,24,{0.2f,1,0.65f,0.08f});
        rect(p.x-12,p.y-12,24,24,{0.05f,0.25f,0.23f,1});
        rect(p.x-3,p.y-8,6,16,Cyan); rect(p.x-8,p.y-3,16,6,Cyan);
    }
    for(const auto& e : game.enemies) {
        const auto p=world(e.position); ship(p,e.heavy?42.0f:33.0f,0,true,e.heavy,t,false);
        rect(p.x-19,p.y-34,38,3,{0.1f,0.12f,0.2f,1});
        rect(p.x-19,p.y-34,38.0f*e.health/(e.heavy?(game.definition().enemyTier==3?5:4):2),3,e.heavy?Orange:glm::vec4(0.7f,0.5f,1,1));
    }
    for (const auto& b : game.bullets) {
        const auto p=world(b.position); const glm::vec4 color=b.hostile?Orange:Cyan;
        if(b.homing) {
            const auto axis=glm::length(b.velocity)>0.001f?glm::normalize(glm::vec2(b.velocity.x,-b.velocity.y)):glm::vec2(1,0);
            const glm::vec2 normal(-axis.y,axis.x);
            const float flicker=1+0.15f*std::sin(game.time*65);
            for(int i=5;i>0;--i) disc(p-axis*(14+i*6.0f),4+i*1.4f,3+i*0.8f,{0.6f,0.65f,0.7f,0.028f*(6-i)},12);
            line(p-axis*(37*flicker),p-axis*12.0f,8,{1,0.45f,0.12f,0.12f});
            triangle(p-axis*11.0f+normal*3.5f,p-axis*11.0f-normal*3.5f,p-axis*(32*flicker),{1,0.45f,0.14f,0.9f});
            triangle(p-axis*12.0f+normal*1.5f,p-axis*12.0f-normal*1.5f,p-axis*(23*flicker),{1,0.95f,0.7f,1});
            triangle(p+axis*13.0f,p+axis*4.0f+normal*4.2f,p+axis*4.0f-normal*4.2f,Orange);
            triangle(p+axis*4.0f+normal*4.0f,p-axis*11.0f+normal*4.0f,p-axis*11.0f-normal*4.0f,White);
            triangle(p+axis*4.0f+normal*4.0f,p-axis*11.0f-normal*4.0f,p+axis*4.0f-normal*4.0f,{0.5f,0.64f,0.72f,1});
            triangle(p-axis*6.0f+normal*3.0f,p-axis*13.0f+normal*8.0f,p-axis*10.0f,Muted);
            triangle(p-axis*6.0f-normal*3.0f,p-axis*13.0f-normal*8.0f,p-axis*10.0f,Muted);
            continue;
        }
        disc(p,14,7,{color.r,color.g,color.b,0.12f});
        line(p,p-glm::normalize(glm::vec2(b.velocity.x,-b.velocity.y))*(b.hostile?10.0f:20.0f),b.hostile?4.0f:3.0f,color);
    }
    if(game.beamActive && !game.cinematicActive()) {
        const auto a=world(game.beamStart),b=world(game.beamEnd);
        const float pulse=1+0.08f*std::sin(game.time*35);
        line(a,b,34*pulse,{0.12f,0.45f,1,0.06f});
        line(a,b,23*pulse,{0.18f,0.65f,1,0.11f});
        line(a,b,13*pulse,{0.25f,0.8f,1,0.4f});
        line(a,b,6*pulse,{0.55f,0.97f,1,0.85f});
        line(a,b,2.4f,{0.95f,1,1,1});
        const float length=glm::length(b-a);
        const glm::vec2 direction=length>0.01f?(b-a)/length:glm::vec2(1,0);
        const glm::vec2 normal(-direction.y,direction.x);
        for(int strand=-1;strand<=1;strand+=2) {
            glm::vec2 previous=a;
            for(int i=1;i<=64;++i) {
                const float distance=length*i/64.0f;
                const float offset=std::sin(distance*0.065f-game.time*22)*4.5f*strand*std::sin(Pi*i/64);
                const auto next=a+direction*distance+normal*offset;
                line(previous,next,1.3f,{0.4f,0.85f,1,0.55f}); previous=next;
            }
        }
        for(float distance=std::fmod(game.time*320,95.0f);distance<length;distance+=95)
            disc(a+direction*distance,14,6,{0.65f,0.95f,1,0.13f},20);
        disc(a,20,16,{0.3f,0.8f,1,0.13f}); disc(a,7,6,{0.8f,1,1,0.8f});
        disc(b,17,17,{0.3f,0.8f,1,0.15f}); disc(b,6,6,{0.8f,1,1,0.9f});
    }
    if(!game.cinematicActive() && (game.missileTargetId!=0 || game.missileLaunchFlash>0)) {
        const float progress=game.missileLaunchFlash>0?1.0f:game.missileLockTime/game.profile.lockDuration();
        auto targetingRing=[&](glm::vec2 position) {
        const auto target=world(position);
        const float radius=46-18*progress;
        const auto color=game.missileLowEnergy?Orange:progress>=1?Cyan:glm::vec4(1,0.68f,0.24f,1);
        for(int i=0;i<48;++i) {
            const float angle=i*2*Pi/48-Pi/2, next=(i+0.7f)*2*Pi/48-Pi/2;
            line(target+glm::vec2(std::cos(angle),std::sin(angle))*radius,target+glm::vec2(std::cos(next),std::sin(next))*radius,2,
                 {color.r,color.g,color.b,i/48.0f<progress?0.9f:0.15f});
        }
        for(int side=-1;side<=1;side+=2) for(int vertical=-1;vertical<=1;vertical+=2) {
            const auto corner=target+glm::vec2(side,vertical)*(radius+5);
            line(corner,corner-glm::vec2(side*11,0),2,color);
            line(corner,corner-glm::vec2(0,vertical*11),2,color);
        }
        text(game.missileLowEnergy?"NEED "+std::to_string(static_cast<int>(game.missileEnergyRequired))+" ENERGY":game.missileLaunchFlash>0?"MISSILE AWAY":"LOCK "+std::to_string(static_cast<int>(progress*100))+"%",target.x-50,target.y+radius+15,1.3f,color);
        };
        targetingRing(game.missileLockPosition);
        if(game.missileSecondTargetId!=0 || (game.missileLaunchFlash>0 && game.missileLastSalvo==2)) targetingRing(game.missileSecondLockPosition);
    }
    if(game.state!=MissionState::Ready && game.state!=MissionState::Lost && !game.cinematicActive() && !landingBackdrop && !finaleBackdrop && (game.player.invulnerability<=0 || static_cast<int>(t*14)%2==0))
        ship(world(game.player.position),36,-game.player.velocity.y*0.025f,false,false,t,game.player.boosting,1,game.player.maneuverJets);
    if (game.launchActive()) departure(game);
    if (game.sceneState()==MissionState::Encounter) moonEncounter(game);
    if (landingBackdrop) moonLanding(game);
    if (game.sceneState()==MissionState::LiftOff) moonTakeoff(game);
    if (finaleBackdrop) marsFinale(game);
    effects(game);

    if(finaleBackdrop) endCard(game);

    if(game.state!=MissionState::Ready && !game.cinematicActive() && !landingBackdrop && !finaleBackdrop) hud(game);

    if(game.state!=MissionState::Flying && (!game.cinematicActive() || game.state==MissionState::Paused) && !(finaleBackdrop && game.state==MissionState::Won)) menu(game,cursor);

    submit();
}

void Renderer::submit() {
    glUseProgram(program); glBindVertexArray(vao); glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));
}
