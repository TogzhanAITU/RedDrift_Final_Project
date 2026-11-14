#include "Renderer.hpp"
#include "Level.hpp"
#include "RenderStyle.hpp"
#include <algorithm>
#include <cmath>

#include "Menu.hpp"
#include "LaunchScene.hpp"
#include "MarsScene.hpp"

using namespace RenderStyle;

void Renderer::centeredText(const std::string& value,float y,float scale,glm::vec4 color) {
    const float width=(static_cast<float>(value.size())*6-1)*scale;
    text(value,640-width*0.5f,y,scale,color);
}

void Renderer::workshop(const Level& game, glm::vec2 cursor) {
    rect(0,0,1280,720,{0.018f,0.03f,0.055f,0.98f});
    text("SELENE / WORKSHOP",100,48,3,White);
    text("SOL "+std::to_string(game.profile.sol),1005,54,2,Cyan);
    text(game.profile.workshopUnlocked?"UPGRADES STAY WITH YOUR CRUISER":"LOCKED / COMPLETE LEVEL 02 AND LAND AT SELENE",100,112,1.5f,Muted);
    for(int tab=0;tab<2;++tab) {
        const auto button=Menu::shopTab(tab);
        const bool selected=game.workshopTab==tab;
        rect(button.x,button.y,button.w,button.h,selected?glm::vec4(0.08f,0.24f,0.27f,1):glm::vec4(0.045f,0.08f,0.11f,1));
        rect(button.x,button.y+button.h-2,button.w,2,selected?Cyan:Muted);
        text(tab==0?"SHIP UPGRADES":"WEAPONS",button.x+26,button.y+15,1.8f,selected?White:Muted);
    }
    const char* names[]={"HULL UPGRADE","LASER CANNON","MISSILE LAUNCHER","ENERGY UPGRADE"};
    const char* first[]={"+25 MAXIMUM HULL PER TIER","HOLD RMB / USES ENERGY","HOLD E / 1.5 SEC LOCK","+25 ENERGY CAPACITY PER TIER"};
    const char* second[]={"5 TIERS / UP TO 225 HULL","STRONG BEAM / MELTS ROCKS","20 ENERGY PER MISSILE","5 TIERS / UP TO 225 ENERGY"};
    for(int i=0;i<2;++i) {
        const auto card=Menu::shopCard(i),button=Menu::buyButton(i);
        const auto upgrade=Menu::shopUpgrade(game.workshopTab,i);
        const int item=static_cast<int>(upgrade);
        const int price=game.profile.price(upgrade);
        const bool installed=price==0,afford=game.profile.sol>=price && game.profile.workshopUnlocked;
        const glm::vec4 accent=upgrade==Upgrade::Missiles?Orange:upgrade==Upgrade::Laser?glm::vec4(0.4f,0.7f,1,1):Cyan;
        rect(card.x,card.y,card.w,card.h,{0.04f,0.075f,0.1f,1});
        rect(card.x,card.y,card.w,2,accent);
        if(upgrade==Upgrade::Hull || upgrade==Upgrade::Energy) {
            rect(card.x+24,card.y+23,36,32,{0.26f,0.37f,0.4f,1});
            rect(card.x+31,card.y+30,22,18,accent);
            if(upgrade==Upgrade::Energy) { rect(card.x+60,card.y+33,5,12,accent); line({card.x+40,card.y+25},{card.x+47,card.y+48},3,White); }
        } else if(upgrade==Upgrade::Laser) {
            line({card.x+24,card.y+39},{card.x+80,card.y+39},9,{accent.r,accent.g,accent.b,0.2f});
            line({card.x+24,card.y+39},{card.x+80,card.y+39},3,accent);
        } else {
            triangle({card.x+24,card.y+30},{card.x+24,card.y+50},{card.x+70,card.y+40},accent);
        }
        text(names[item],card.x+20,card.y+76,2,White);
        if(upgrade==Upgrade::Laser) {
            text("RMB / DAMAGE "+std::to_string(static_cast<int>(game.profile.laserDamage()))+" / SEC",card.x+20,card.y+112,1.35f,Muted);
            text("CONTINUOUS BEAM / 30 ENERGY PER SEC",card.x+20,card.y+134,1.25f,Muted);
        } else if(upgrade==Upgrade::Missiles) {
            text("DAMAGE "+std::to_string(static_cast<int>(game.profile.missileDamage()))+" / SALVO "+std::to_string(game.profile.missileCount()),card.x+20,card.y+112,1.35f,Muted);
            text(std::string("E / LOCK ")+(game.profile.missileLevel()>=5?"1.0S":"1.5S")+" / 20 ENERGY EACH",card.x+20,card.y+134,1.25f,Muted);
        } else {
            text(first[item],card.x+20,card.y+112,1.35f,Muted);
            text(second[item],card.x+20,card.y+134,1.35f,Muted);
        }
        const bool shipUpgrade=upgrade==Upgrade::Hull || upgrade==Upgrade::Energy;
        const int tier=upgrade==Upgrade::Hull?game.profile.hullTier:upgrade==Upgrade::Energy?game.profile.energyTier:upgrade==Upgrade::Laser?game.profile.laserLevel():game.profile.missileLevel();
        const std::string status=shipUpgrade?"TIER "+std::to_string(tier)+" / 5   MAX "+std::to_string(upgrade==Upgrade::Hull?game.profile.maxHull():game.profile.maxEnergy()):"LEVEL "+std::to_string(tier)+" / 5";
        text(status,card.x+20,card.y+160,1.5f,accent);
        const auto preview=game.profile.nextUpgrade(upgrade);
        for(int row=0;row<3;++row) text(preview[row],card.x+20,card.y+184+row*17.0f,1.2f,row==0?White:Muted);
        const glm::vec4 color=!game.profile.workshopUnlocked?Muted:installed?Muted:afford?accent:glm::vec4(0.30f,0.35f,0.40f,1);
        rect(button.x,button.y,button.w,button.h,button.contains(cursor)&&afford&&!installed?White:color);
        std::string label=!game.profile.workshopUnlocked?"LOCKED":installed?"MAXIMUM LEVEL":std::string(tier==0?"BUY / ":"UPGRADE / ")+std::to_string(price)+" SOL";
        text(label,button.x+18,button.y+16,1.5f,{0.02f,0.04f,0.055f,1});
    }
    text(game.shopMessage,100,546,1.55f,White);
    text("25 SOL / SCOUT     60 SOL / HEAVY SHIP",100,580,1.3f,Muted);
    const auto back=Menu::ShopBack;
    rect(back.x,back.y,back.w,back.h,back.contains(cursor)?White:Cyan);
    text("ENTER / ESC  LEVEL SELECT",back.x+32,back.y+16,1.5f,{0.02f,0.04f,0.055f,1});
    text("TAB / ARROWS  SECTION    1-2 BUY    LMB BULLETS / RMB LASER / E MISSILES",100,685,1.3f,Muted);
}

void Renderer::hud(const Level& game) {
    rect(0,0,1280,90,{0.018f,0.03f,0.055f,0.95f});
    text("RED",30,22,3,Orange); text("DRIFT",96,22,3,White);
    text("LEVEL "+std::to_string(game.levelIndex+1)+" / 6",31,57,1.5f,Muted);
    text("HULL "+std::to_string(game.player.health)+"/"+std::to_string(game.player.maxHull),280,23,1.5f,White);
    rect(280,49,150,5,{0.14f,0.19f,0.25f,1}); rect(280,49,150.0f*game.player.health/game.player.maxHull,5,game.player.health<30?Orange:Cyan);
    text("ENERGY "+std::to_string(static_cast<int>(game.player.energy)),465,23,1.5f,Muted); rect(465,49,110,5,{0.14f,0.19f,0.25f,1}); rect(465,49,110*game.player.energy/game.player.maxEnergy,5,game.player.laserExhausted?Orange:Cyan);
    text("ROUTE "+std::to_string(static_cast<int>(game.progress()*100))+"%",625,23,1.5f,White);
    rect(625,49,245,5,{0.14f,0.19f,0.25f,1}); rect(625,49,245*game.progress(),5,Orange);
    text("SCORE "+std::to_string(game.score),925,23,1.5f,White);
    text("SOL "+std::to_string(game.profile.sol),31,77,1.3f,Cyan);
    text("LEVEL 0"+std::to_string(game.levelIndex+1),1100,23,1.5f,Orange);
    text(game.definition().title,925,49,1.2f,Muted);
    rect(0,655,1280,65,{0.018f,0.03f,0.055f,0.94f});
    text(std::string("LMB BULLETS  RMB LASER  HOLD E ")+(game.profile.missileLevel()>=5?"1.0S":"1.5S")+" MISSILE  SHIFT ACCELERATE",30,678,1.3f,Muted);
    if(game.player.laserExhausted) text("RELEASE RMB TO RECHARGE ENERGY",30,643,1.25f,Orange);
    text("P  PAUSE   R  RETRY   M  LEVELS",914,678,1.5f,Muted);
}

void Renderer::menu(const Level& game,glm::vec2 cursor) {
    const bool ready=game.state==MissionState::Ready,won=game.state==MissionState::Won,lost=game.state==MissionState::Lost;
    rect(0,0,1280,720,{0.01f,0.02f,0.04f,0.82f});
    const glm::vec4 accent = lost ? Orange : Cyan;
    if (game.state==MissionState::Workshop) workshop(game,cursor);
    else if(game.workshopNotice) {
        centeredText("SELENE STATION / NEW SERVICE",174,1.5f,Cyan);
        centeredText("WORKSHOP UNLOCKED",225,3.8f,White);
        centeredText("UPGRADE YOUR CRUISER WITH SOL",287,1.8f,Muted);
        centeredText("LASER CANNON / REINFORCED HULL / HOMING MISSILES",344,1.5f,White);
        centeredText("Earn Sol by destroying enemy ships.",385,1.6f,Muted);
        const auto button=Menu::Primary;
        rect(button.x,button.y,button.w,button.h,button.contains(cursor)?White:Cyan);
        centeredText("ENTER  OPEN WORKSHOP",472,1.5f,{0.02f,0.04f,0.06f,1});
        centeredText("M / ESC  LEVEL SELECT",533,1.3f,Muted);
    } else if (ready) {
        centeredText("RED DRIFT",48,4.0f,White);
        centeredText("SELECT A LEVEL",104,1.6f,Cyan);
        text("SOL "+std::to_string(game.profile.sol),100,122,1.6f,Cyan);
        const auto shop=Menu::Workshop;
        rect(shop.x,shop.y,shop.w,shop.h,shop.contains(cursor)?glm::vec4(0.13f,0.25f,0.27f,1):glm::vec4(0.055f,0.12f,0.15f,1));
        text(game.profile.workshopUnlocked?"W  WORKSHOP":"W  WORKSHOP LOCKED",shop.x+12,shop.y+14,1.35f,game.profile.workshopUnlocked?Cyan:Muted);
        for (int i=0;i<Level::StageCount;++i) {
            const auto box=Menu::card(i);
            const bool selected=game.selectedLevel==i, hovered=box.contains(cursor);
            const auto& entry=Stages[i];
            rect(box.x,box.y,box.w,box.h,selected?glm::vec4(0.06f,0.15f,0.18f,1):glm::vec4(0.035f,0.06f,0.09f,1));
            rect(box.x,box.y,box.w,2,selected?Cyan:hovered?White:glm::vec4(0.15f,0.22f,0.28f,1));
            text("0"+std::to_string(i+1),box.x+20,box.y+19,2.5f,selected?Cyan:Muted);
            if(game.completed[i]) text("DONE",box.x+box.w-62,box.y+21,1.5f,Cyan);
            text(entry.title,box.x+20,box.y+61,1.7f,White);
            text(entry.description,box.x+20,box.y+89,1.3f,Muted);
            text(std::to_string(static_cast<int>(entry.length/Player::CruiseSpeed))+" SEC",box.x+20,box.y+121,1.3f,Muted);
            text(entry.enemyTier==0?"DODGE":entry.enemyTier==1?"LIGHT":entry.enemyTier==2?"HARD":"EXPERT",box.x+box.w-78,box.y+121,1.3f,selected?Cyan:Muted);
        }
        const auto button=Menu::Start;
        rect(button.x,button.y,button.w,button.h,button.contains(cursor)?White:Cyan);
        centeredText("ENTER  START LEVEL "+std::to_string(game.selectedLevel+1),571,1.5f,{0.02f,0.04f,0.06f,1});
        centeredText("CLICK A CARD OR PRESS 1-6 TO SELECT",629,1.4f,Muted);
        centeredText("WASD MOVE  LMB BULLETS  RMB LASER  E MISSILES  SHIFT ACCELERATE",660,1.25f,Muted);
        centeredText("ESC  EXIT",695,1.2f,Muted);
    } else {
    centeredText("LEVEL "+std::to_string(game.levelIndex+1)+" / "+game.definition().title,186,1.5f,accent);
    centeredText(won?(game.levelIndex==5?"MARS REACHED":"LEVEL COMPLETE"):lost?"MISSION FAILED":"PAUSED",228,4.0f,White);
    centeredText(won?"ROUTE CLEARED.":lost?"YOUR ROCKET WAS DESTROYED.":"PRESS ENTER OR P TO CONTINUE.",284,1.5f,Muted);
    line({460,326},{820,326},1,{0.3f,0.43f,0.53f,0.3f});
    if (won || lost) {
        centeredText("SCORE  "+std::to_string(game.score),353,2.0f,White);
        centeredText("SHIPS DESTROYED  "+std::to_string(game.kills),390,1.5f,Muted);
    } else {
        centeredText("ROUTE  "+std::to_string(static_cast<int>(game.progress()*100))+"%",353,2.0f,White);
        centeredText("R  RESTART FLIGHT",390,1.5f,Muted);
    }
    const auto button=Menu::Primary;
    rect(button.x,button.y,button.w,button.h,button.contains(cursor)?White:accent);
    centeredText(won?(game.levelIndex==5?"ENTER  LEVEL SELECT":"ENTER  NEXT LEVEL"):lost?"ENTER  TRY AGAIN":"ENTER  RESUME",472,1.6f,{0.02f,0.04f,0.06f,1});
    centeredText("M / ESC  LEVEL SELECT",533,1.3f,Menu::Back.contains(cursor)?White:Muted);
    if(game.profile.workshopUnlocked) centeredText("W  WORKSHOP / SOL "+std::to_string(game.profile.sol),574,1.4f,Cyan);
    }
}

void Renderer::endCard(const Level& game) {
    if(game.finaleTime>24.2f) {
        const float fade=LaunchScene::ease((game.finaleTime-24.2f)/0.8f);
        rect(0,0,1280,720,{0,0,0,fade});
        const float title=LaunchScene::ease((game.finaleTime-MarsScene::EndCardStart)/0.8f);
        centeredText("TO BE CONTINUED",348,3.5f,{White.r,White.g,White.b,title});
        if(game.state==MissionState::Won) text("ENTER / M / ESC  LEVEL SELECT",505,660,1.4f,Muted);
    }
}
