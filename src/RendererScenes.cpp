#include "Renderer.hpp"
#include "Level.hpp"
#include "RenderStyle.hpp"
#include <algorithm>
#include <cmath>

#include "LaunchScene.hpp"
#include "MoonScene.hpp"
#include "MarsScene.hpp"

using namespace RenderStyle;

void Renderer::departure(const Level& game) {
    const float t = game.launchTime;
    const auto frame = LaunchScene::sample(t);
    const glm::vec4 steel(0.23f,0.30f,0.34f,1), lit(0.40f,0.49f,0.52f,1);
    const glm::vec4 shadow(0.10f,0.16f,0.20f,1), dark(0.025f,0.06f,0.085f,1);
    auto box = [&](float x, float y, float w, float h, glm::vec4 color) {
        rect(frame.x(x),frame.y(y),w*frame.zoom,h*frame.zoom,color);
    };
    auto rail = [&](glm::vec2 a, glm::vec2 b, float width, glm::vec4 color) {
        line({frame.x(a.x),frame.y(a.y)},{frame.x(b.x),frame.y(b.y)},width*frame.zoom,color);
    };
    auto label = [&](const std::string& value, float x, float y, float scale, glm::vec4 color) {
        text(value,frame.x(x),frame.y(y),scale*frame.zoom,color);
    };

    rail({-40,165},{-40,565},14,steel);
    rail({-190,185},{60,185},12,shadow);
    rail({-190,550},{60,550},12,shadow);
    for (int row=0;row<2;++row) {
        const float y=row==0?110.0f:520.0f;
        box(-215,y,260,100,shadow);
        for (int col=0;col<7;++col) {
            box(-207+col*35.0f,y+7,29,86,{0.08f,0.22f,0.32f,1});
            rail({-205+col*35.0f,y+49},{-180+col*35.0f,y+49},1.5f,lit);
        }
    }
    box(-95,235,170,272,shadow);
    box(-82,250,144,242,steel);
    for(int i=0;i<4;++i) {
        box(-65,270+i*51.0f,92,35,shadow);
        box(-60,274+i*51.0f,4,25,lit);
    }
    box(45,268,389,204,shadow);
    box(62,286,356,168,steel);
    box(77,301,342,138,dark);
    box(82,307,326,22,{0.045f,0.10f,0.13f,1});
    for (int i=0;i<6;++i) {
        const float x=91+i*52.0f;
        box(x,308,7,125,shadow);
        box(x+15,315,21,3,{0.33f,0.67f,0.73f,1});
        box(x+12,428,27,3,{0.38f,0.48f,0.49f,1});
    }
    rail({82,416},{418,416},2,{0.32f,0.7f,0.75f,1});
    label("ORBITAL STATION",88,271,1.6f,White);
    label("DOCK 01",89,459,1.5f,Cyan);

    ship({frame.screenX,370},72.0f*frame.zoom,0,false,false,t,t>2.5f,frame.engine);

    box(63,286,354,14,lit);
    box(62,440,354,14,steel);
    box(409,284,29,26,lit);
    box(409,433,29,25,steel);
    const float doorHeight=67.0f*(1.0f-frame.door);
    box(410,303,22,doorHeight,lit);
    box(410,437-doorHeight,22,doorHeight,steel);
    box(436,292,5,154,shadow);
    for (int i=0;i<5;++i) {
        box(420,277+i*3.5f,12,2.0f,{0.78f,0.56f,0.25f,1});
        box(420,450+i*3.5f,12,2.0f,{0.78f,0.56f,0.25f,1});
    }
    const glm::vec4 signal=frame.door>=0.99f?Cyan:Orange;
    disc({frame.x(448),frame.y(290)},3*frame.zoom,3*frame.zoom,signal,12);
    disc({frame.x(448),frame.y(450)},3*frame.zoom,3*frame.zoom,signal,12);

    const float alpha=frame.bars;
    rect(0,0,1280,76,{0.01f,0.02f,0.035f,alpha});
    rect(0,644,1280,76,{0.01f,0.02f,0.035f,alpha});
    text("LEVEL 01 / EARTH DEPARTURE",36,29,1.8f,{White.r,White.g,White.b,alpha});
    const char* status=t<1.4f?"OPENING HANGAR":t<2.8f?"UNDOCKING":t<4.2f?"MAIN ENGINES ONLINE":"ENTERING STRATOSPHERE";
    text(status,36,674,1.6f,{Cyan.r,Cyan.g,Cyan.b,alpha});
    text("ENTER / LMB  SKIP",1055,674,1.5f,{Muted.r,Muted.g,Muted.b,alpha});
}

void Renderer::moonEncounter(const Level& game) {
    const float t=game.encounterTime;
    const auto frame=MoonScene::encounter(t);
    ship(frame.enemy,frame.enemyScale,0,true,true,t,true);
    if(t<3.0f) {
        const float r=frame.enemyScale*1.15f;
        const glm::vec4 alert(Orange.r,Orange.g,Orange.b,0.65f);
        for(int side=-1;side<=1;side+=2) {
            const float x=frame.enemy.x+side*r;
            line({x,frame.enemy.y-38},{x,frame.enemy.y-22},1.5f,alert);
            line({x,frame.enemy.y-38},{x-side*13,frame.enemy.y-38},1.5f,alert);
        }
        text("UNKNOWN VESSEL",frame.enemy.x-72,frame.enemy.y+r*0.45f+18,1.3f,alert);
    }
    if(frame.playerReveal>0) ship(frame.player,frame.playerScale,0,false,false,t,false);
    const float alpha=frame.bars;
    rect(0,0,1280,76,{0.01f,0.02f,0.035f,alpha});
    rect(0,644,1280,76,{0.01f,0.02f,0.035f,alpha});
    text("LEVEL 02 / LUNAR APPROACH",36,29,1.8f,{White.r,White.g,White.b,alpha});
    text(t<2.8f?"HOSTILE CONTACT APPROACHING":t<4.7f?"RETURNING TO OUR CRUISER":"TAKE THE HELM",36,674,1.5f,{Cyan.r,Cyan.g,Cyan.b,alpha});
    text("ENTER / LMB  SKIP",1055,674,1.5f,{Muted.r,Muted.g,Muted.b,alpha});
}

void Renderer::station(glm::vec2 moonCenter, float radius, float ground, float stationAlpha) {
    const float deck=ground-MoonScene::DeckHeight;
    auto structure = [&](float x,float y,float w,float h,glm::vec3 color) {
        rect(x,deck+y,w,h,{color,stationAlpha});
    };
    for (float x : {470.0f,580.0f,690.0f,890.0f}) {
        const float dx=x-moonCenter.x;
        const float soil=moonCenter.y-std::sqrt(std::max(0.0f,radius*radius-dx*dx));
        if(soil>deck+16) {
            line({x,deck+16},{x,soil+2},8,{0.23f,0.29f,0.32f,stationAlpha});
            rect(x-13,soil-2,26,6,{0.33f,0.39f,0.42f,stationAlpha});
        }
    }
    structure(446,4,466,15,{0.20f,0.25f,0.29f});
    structure(458,-32,145,36,{0.32f,0.39f,0.43f});
    disc({530,deck-32},72,32,{0.48f,0.56f,0.60f,stationAlpha});
    structure(466,-24,128,10,{0.07f,0.18f,0.25f});
    for(int i=0;i<7;++i) structure(473+i*17.0f,-22,9,6,{0.35f,0.76f,0.81f});
    structure(603,-26,77,30,{0.23f,0.31f,0.36f});
    structure(624,-81,41,59,{0.37f,0.45f,0.49f});
    structure(613,-91,64,19,{0.52f,0.58f,0.60f});
    structure(620,-87,50,8,{0.08f,0.24f,0.31f});
    line({644,deck-91},{644,deck-120},2,{0.61f,0.69f,0.72f,stationAlpha});
    disc({644,deck-121},3,3,{Orange.r,Orange.g,Orange.b,stationAlpha},12);
    structure(680,-16,40,20,{0.24f,0.32f,0.37f});
    structure(715,0,181,9,{0.43f,0.49f,0.52f});
    structure(728,-3,153,3,{0.72f,0.77f,0.77f});
    structure(747,9,10,14,{0.28f,0.33f,0.36f});
    structure(851,9,10,14,{0.28f,0.33f,0.36f});
    for(int i=0;i<7;++i) structure(737+i*22.0f,4,10,2,{0.91f,0.65f,0.28f});
    for(int side=-1;side<=1;side+=2) {
        const float x=800+side*89.0f;
        structure(x,-17,3,18,{0.41f,0.49f,0.52f});
        disc({x+1.5f,deck-19},4,4,{Cyan.r,Cyan.g,Cyan.b,stationAlpha},12);
        disc({x+1.5f,deck-19},10,10,{Cyan.r,Cyan.g,Cyan.b,stationAlpha*0.08f},16);
    }
    text("SELENE STATION",462,deck-3,1.1f,{White.r,White.g,White.b,stationAlpha});
}

void Renderer::moonLanding(const Level& game) {
    const float t=game.landingTime;
    const auto frame=MoonScene::landing(t,game.landingStartScreen,game.landingStartTilt);
    moon(frame.moon,frame.radius);
    const float ground=frame.moon.y-std::sqrt(std::max(0.0f,frame.radius*frame.radius-(800-frame.moon.x)*(800-frame.moon.x)));
    const float deck=ground-MoonScene::DeckHeight;
    const float haze=LaunchScene::ease((t-2.5f)/1.2f)*(1-LaunchScene::ease((t-7.3f)/1.9f));
    rect(0,76,1280,546,{0.34f,0.42f,0.53f,haze*0.23f});
    for(int i=0;i<9;++i) {
        const float x=std::fmod(1450.0f-i*177.0f-t*150.0f,1600.0f);
        disc({x<0?x+1600:x,140.0f+i*56.0f},260,18,{0.64f,0.71f,0.80f,haze*0.075f});
    }
    station(frame.moon,frame.radius,ground,LaunchScene::ease((t-5.8f)/1.2f));
    const auto p=frame.player;
    for(int side=-1;side<=1;side+=2) {
        const float x=p.x+side*frame.scale*0.44f;
        const float y=p.y+frame.scale*0.22f;
        const float foot=y+frame.scale*0.28f*frame.legs;
        line({x,y},{x+side*4*frame.legs,foot},2,{0.40f,0.47f,0.48f,1});
        line({x-5,foot},{x+5,foot},2,{0.70f,0.74f,0.71f,frame.legs});
        if(t>4.0f && frame.engine>0) {
            const float flame=8.0f+4.0f*std::sin(t*35.0f+side);
            triangle({x-3,y},{x+3,y},{x,y+flame*frame.engine},{0.4f,0.85f,1,frame.engine*0.6f});
        }
    }
    for(int i=0;i<9;++i) {
        const float drift=std::fmod(t*18+i*13.0f,105.0f)-52.5f;
        disc({p.x+drift,deck-4-std::abs(drift)*0.08f},10+std::abs(drift)*0.1f,3,{0.75f,0.76f,0.73f,frame.dust*0.12f});
    }
    ship(p,frame.scale,frame.tilt,false,false,t,false,frame.engine);
    rect(0,0,1280,76,{0.01f,0.02f,0.035f,1});
    text("LEVEL 02 / SELENE STATION",36,22,1.8f,White);
    text(t<2.5f?"LUNAR ARRIVAL":t<6.5f?"ENTERING LUNAR HAZE":t<9.5f?"DESCENDING TO STATION":t<10.7f?"LANDING CLEARANCE CONFIRMED":"TOUCHDOWN / DOCK 01",36,52,1.3f,Cyan);
    rect(0,622,1280,98,{0.01f,0.02f,0.035f,0.97f});
    text("BRIDGE",36,638,1.3f,Cyan);
    const std::string caption=MoonScene::caption(t);
    text(caption,36,666,2.1f,White);
    if(caption.size()<MoonScene::ArrivalText.size() && static_cast<int>(t*4)%2==0)
        rect(36+static_cast<float>(caption.size())*12.6f,666,2,15,Cyan);
    if (game.sceneState()==MissionState::Landing) {
        text("HOLD ENTER  SKIP",1070,687,1.5f,Muted);
        if(game.landingSkipHold>0) rect(1070,705,145*std::min(1.0f,game.landingSkipHold),2,Cyan);
    }
}

void Renderer::moonTakeoff(const Level& game) {
    const float t=game.takeoffTime;
    const auto frame=MoonScene::takeoff(t);
    moon(frame.moon,frame.radius);
    const float ground=frame.moon.y-std::sqrt(std::max(0.0f,frame.radius*frame.radius-(800-frame.moon.x)*(800-frame.moon.x)));
    station(frame.moon,frame.radius,ground,1-LaunchScene::ease((t-3.7f)/1.3f));
    const float haze=LaunchScene::ease((t-0.8f)/1.2f)*(1-LaunchScene::ease((t-4.2f)/1.8f));
    rect(0,76,1280,546,{0.34f,0.42f,0.53f,haze*0.18f});
    for(int i=0;i<7;++i) {
        const float y=std::fmod(i*83.0f+t*82.0f,570.0f)+76;
        disc({i%2?320.0f:950.0f,y},300,14,{0.64f,0.71f,0.8f,haze*0.07f});
    }
    for(int side=-1;side<=1;side+=2) {
        const float x=frame.player.x+side*frame.scale*0.44f;
        const float y=frame.player.y+frame.scale*0.22f;
        const float foot=y+frame.scale*0.28f*frame.legs;
        line({x,y},{x+side*4*frame.legs,foot},2,{0.40f,0.47f,0.48f,frame.legs});
        line({x-5,foot},{x+5,foot},2,{0.7f,0.74f,0.71f,frame.legs});
        const float thrust=LaunchScene::ease((t-0.4f)/0.6f)*(1-LaunchScene::ease((t-3.5f)/1.0f));
        triangle({x-4,y},{x+4,y},{x,y+26*thrust},{0.4f,0.85f,1,thrust*0.8f});
    }
    ship(frame.player,frame.scale,frame.tilt,false,false,t,t>1.2f,frame.engine);
    const float alpha=1-LaunchScene::ease((t-6.2f)/0.8f);
    rect(0,0,1280,76,{0.01f,0.02f,0.035f,alpha});
    rect(0,644,1280,76,{0.01f,0.02f,0.035f,alpha});
    text("LEVEL 03 / SELENE DEPARTURE",36,29,1.8f,{White.r,White.g,White.b,alpha});
    text(t<1?"STARTING LIFT ENGINES":t<3.0f?"LIFTOFF / GEAR RETRACTING":t<5.4f?"CLIMBING THROUGH LUNAR HAZE":"ENTERING OPEN SPACE",36,674,1.5f,{Cyan.r,Cyan.g,Cyan.b,alpha});
    text("ENTER / LMB  SKIP",1055,674,1.5f,{Muted.r,Muted.g,Muted.b,alpha});
}

void Renderer::applyMarsCamera(std::size_t first,const Level& game) {
    const auto camera=MarsScene::camera(game.finaleTime);
    for(std::size_t i=first;i<vertices.size();++i) vertices[i].position=camera.project(vertices[i].position);
}

void Renderer::marsFinale(const Level& game) {
    const float t=game.finaleTime;
    const auto firstSceneVertex=vertices.size();
    const auto camera=MarsScene::camera(t);
    const float descent=LaunchScene::ease(t/6);
    rect(-1000,-600,4000,2000,{0.38f,0.18f,0.10f,descent*0.5f});
    for(int i=5;i>0;--i) disc({640,1600-70*descent},1400.0f+i*12,1020.0f+i*12,{0.85f,0.27f,0.12f,0.06f});
    disc({640,1600-70*descent},1400,1020,{0.32f,0.12f,0.09f,1});
    for(int i=0;i<9;++i) {
        const float y=std::fmod(i*71.0f+t*15.0f,590.0f)+70;
        const float x=std::fmod(i*193.0f-t*25+1400,1600.0f)-150;
        disc({x,y},270,22,{0.72f,0.39f,0.23f,descent*0.07f});
    }
    const auto cruiser=MarsScene::cruiser(t,game.finaleStartScreen);
    for(int i=0;i<MarsScene::Pursuers;++i) if(!game.finaleDestroyed[i]) {
        const auto p=MarsScene::pursuer(t,i);
        const float angle=MarsScene::enemyHeading(t,i,game.finaleStartScreen);
        ship(p,42,angle,true,true,t,true,1,{0,0},true);
        for(int cycle=0;cycle<3;++cycle) for(int bullet=0;bullet<2;++bullet) {
            const auto shot=MarsScene::enemyShot(t,i,cycle,bullet,game.finaleStartScreen);
            if(!shot.active) continue;
            const auto direction=glm::normalize(shot.velocity);
            line(shot.position,shot.position-direction*18.0f,5,{1,0.32f,0.1f,0.08f});
            line(shot.position,shot.position-direction*12.0f,1.8f,{1,0.60f,0.3f,0.9f});
        }
        for(int cycle=0;cycle<9;++cycle) for(int bullet=0;bullet<2;++bullet) {
            const auto shot=MarsScene::counterShot(t,i,cycle,bullet);
            if(!shot.active) continue;
            const auto direction=glm::normalize(shot.velocity);
            line(shot.position,shot.position-direction*25.0f,6,{1,0.12f,0.08f,0.12f});
            line(shot.position,shot.position-direction*19.0f,2,{1,0.28f,0.20f,1});
        }
    }
    for(int i=0;i<MarsScene::Fighters;++i) {
        const auto p=MarsScene::fighter(t,i);
        martianFighter(p,34+i*1.0f,std::sin(t*0.8f+i)*0.08f,t);
        if(t>12.5f && t<21.0f && (camera.reveal>0.7f || t>16.0f)) text("MARS AIR GUARD",p.x-47,p.y+52,1.05f,Cyan);
        if(t>11.9f && t<18.8f) for(int gun=0;gun<2;++gun) {
            int target=(i*2+gun)%MarsScene::Pursuers;
            for(int search=0;search<MarsScene::Pursuers && game.finaleDestroyed[target];++search) target=(target+1)%MarsScene::Pursuers;
            if(game.finaleDestroyed[target]) continue;
            const auto muzzle=p+glm::vec2(-34,gun?5.0f:-5.0f);
            const auto aim=MarsScene::pursuer(t,target);
            const auto direction=glm::normalize(aim-muzzle);
            for(int bullet=0;bullet<4;++bullet) {
                const float travel=std::fmod(t*1.45f+i*0.13f+gun*0.09f+bullet*0.25f,1.0f);
                const auto shot=glm::mix(muzzle,aim,travel);
                line(shot,shot-direction*14.0f,2,{0.95f,0.92f,0.52f,1});
                disc(shot,2.3f,2.3f,{0.8f,1,0.72f,1},8);
            }
        }
    }
    for(int i=0;i<MarsScene::Pursuers;++i) {
        const float untilImpact=MarsScene::destructionTime(i)-t;
        if(untilImpact<0 || untilImpact>0.9f) continue;
        const auto start=MarsScene::fighter(t,i%MarsScene::Fighters)-glm::vec2(34,0);
        const auto end=MarsScene::pursuer(t,i);
        const auto p=glm::mix(start,end,1-untilImpact/0.9f);
        const auto direction=glm::normalize(end-start);
        line(p,p-direction*35.0f,8,{0.2f,1,0.68f,0.13f});
        line(p,p-direction*28.0f,3,{0.5f,1,0.76f,1});
    }
    const auto delta=MarsScene::cruiser(t+0.03f,game.finaleStartScreen)-cruiser;
    const float bank=std::clamp(delta.y*0.09f,-0.35f,0.35f);
    const glm::vec2 jets(std::clamp(-delta.y*0.5f,0.0f,1.0f),std::clamp(delta.y*0.5f,0.0f,1.0f));
    ship(cruiser,36+8*descent,game.finaleStartTilt*(1-descent)+bank,false,false,t,false,1,jets);
    applyMarsCamera(firstSceneVertex,game);
    rect(0,0,1280,76,{0.01f,0.02f,0.035f,1});
    rect(0,644,1280,76,{0.01f,0.02f,0.035f,1});
    text("LEVEL 06 / MARS ARRIVAL",36,22,1.8f,White);
    const int destroyed=static_cast<int>(std::count(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),true));
    text("HEAVY PURSUERS "+std::to_string(MarsScene::Pursuers-destroyed)+" / 8",36,52,1.25f,Orange);
    text(t<2?"DESCENDING INTO MARS STRATOSPHERE":t<4?"WARNING / HEAVY SHIPS ON OUR SIX":t<7.5f?"EVASIVE MANEUVERS / INCOMING BURSTS":t<10.5f?"HOSTILES CLOSING THE RING":t<11.9f?"SURROUNDED / ENEMY GUNS FALL SILENT":t<13.2f?"MARS AIR GUARD / ATTACK ON APPROACH":t<16.0f?"HOSTILES TURNING / GUARDS UNDER FIRE":t<18.8f?"GUARD VOLLEYS / BREAKING THE RING":t<24?"MARS AIR GUARD RETURNING TO PATROL":"WELCOME TO MARS",36,674,1.35f,Cyan);
    if(game.sceneState()==MissionState::MarsFinale) {
        text("HOLD ENTER  SKIP",1070,687,1.4f,Muted);
        if(game.finaleSkipHold>0) rect(1070,706,145*std::min(1.0f,game.finaleSkipHold),2,Cyan);
    }
}

void Renderer::effects(const Level& game) {
    const auto firstEffectVertex=vertices.size();
    auto world = [&](glm::vec2 p) { return glm::vec2(640+(p.x-game.cameraX)*36,370-p.y*36); };
    for(const auto& blast:game.explosions) {
        const auto center=world(blast.position);
        const float radius=(12+blast.age*67)*blast.scale, fade=1-blast.age;
        for(int i=0;i<5;++i) {
            const float angle=i*2*Pi/5;
            disc(center+glm::vec2(std::cos(angle),std::sin(angle))*radius*0.35f,radius*0.5f,radius*0.4f,{0.3f,0.24f,0.22f,fade*0.13f});
        }
        const float fire=std::max(0.0f,1-blast.age/0.45f);
        disc(center,radius*0.75f,radius*0.65f,{1,0.28f,0.06f,fire*0.45f});
        disc(center,radius*0.42f,radius*0.35f,{1,0.72f,0.25f,fire*0.8f});
        disc(center,9*blast.scale,9*blast.scale,{1,0.97f,0.8f,std::max(0.0f,1-blast.age/0.2f)});
        for(int i=0;i<48;++i) {
            const float a=i*2*Pi/48,b=(i+1)*2*Pi/48;
            line(center+glm::vec2(std::cos(a),std::sin(a))*radius,center+glm::vec2(std::cos(b),std::sin(b))*radius,2,{1,0.6f,0.25f,fade*fade*0.45f});
        }
    }
    for(const auto& p : game.particles) {
        if(!p.shard) { disc(world(p.position),2.8f,2.8f,{p.color,p.life/p.total},8); continue; }
        const auto center=world(p.position);
        const glm::vec2 x(std::cos(p.angle)*p.size.x*36,std::sin(p.angle)*p.size.x*36);
        const glm::vec2 y(-std::sin(p.angle)*p.size.y*36,std::cos(p.angle)*p.size.y*36);
        const float fade=std::min(1.0f,p.life/0.4f);
        triangle(center-x-y,center+x-y,center+x+y,{p.color,fade});
        triangle(center-x-y,center+x+y,center-x+y,{p.color*0.65f,fade});
        line(center-x-y,center+x-y,1,{1,0.6f,0.25f,fade*0.65f});
    }
    if(game.sceneState()==MissionState::MarsFinale || (game.state==MissionState::Won && game.levelIndex==5)) applyMarsCamera(firstEffectVertex,game);
}
