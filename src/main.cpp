#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderer.hpp"
#include "Level.hpp"
#include "Menu.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
std::filesystem::path executableDirectory(const char* arg) {
#ifdef _WIN32
    wchar_t path[32768]{};
    const DWORD length = GetModuleFileNameW(nullptr,path,32768);
    if(length>0 && length<32768) return std::filesystem::path(path).parent_path();
#endif
    return std::filesystem::absolute(arg).parent_path();
}
std::filesystem::path progressFile(const std::filesystem::path& executable) {
#ifdef _WIN32
    wchar_t localData[32768]{};
    const DWORD length=GetEnvironmentVariableW(L"LOCALAPPDATA",localData,32768);
    if(length>0 && length<32768) return std::filesystem::path(localData)/"RedDrift"/"progress.txt";
#else
    if(const char* data=std::getenv("XDG_DATA_HOME")) return std::filesystem::path(data)/"RedDrift"/"progress.txt";
    if(const char* homePath=std::getenv("HOME")) return std::filesystem::path(homePath)/".local"/"share"/"RedDrift"/"progress.txt";
#endif
    return executable/"saves"/"progress.txt";
}
void screenshot(const std::string& path, int width, int height) {
    const int stride = (width*3+3)&~3;
    std::vector<unsigned char> pixels(static_cast<size_t>(stride)*height);
    glPixelStorei(GL_PACK_ALIGNMENT,4); glReadBuffer(GL_BACK);
    glReadPixels(0,0,width,height,GL_BGR,GL_UNSIGNED_BYTE,pixels.data());
    std::array<unsigned char,54> header{}; header[0]='B'; header[1]='M';
    auto integer = [&](int offset, unsigned int n) { for(int i=0;i<4;++i) header[offset+i]=static_cast<unsigned char>((n>>(8*i))&255); };
    integer(2,54+static_cast<unsigned int>(pixels.size())); integer(10,54); integer(14,40);
    integer(18,width); integer(22,height); header[26]=1; header[28]=24;
    std::ofstream file(path,std::ios::binary);
    file.write(reinterpret_cast<const char*>(header.data()),header.size());
    file.write(reinterpret_cast<const char*>(pixels.data()),static_cast<std::streamsize>(pixels.size()));
    if(!file) throw std::runtime_error("Could not save screenshot");
}
}

int main(int argc, char** argv) {
    bool smoke=false,previewLanding=false,previewMars=false,smokeFull=false,smokeUpgrades=false; std::string capture;
    int smokeStage=0;
    int smokeWeaponLevel=1;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--smoke-test") smoke=true;
        else if(arg=="--smoke-upgrades") { smoke=true; smokeUpgrades=true; }
        else if(arg=="--smoke-full-level") { smoke=true; smokeFull=true; }
        else if(arg=="--preview-landing") { smoke=true; previewLanding=true; }
        else if(arg=="--preview-mars") { smoke=true; previewMars=true; }
        else if(arg=="--screenshot" && i+1<argc) capture=argv[++i];
        else if(arg=="--stage" && i+1<argc) smokeStage=std::clamp(std::atoi(argv[++i])-1,0,Level::StageCount-1);
        else if(arg=="--weapon-level" && i+1<argc) smokeWeaponLevel=std::clamp(std::atoi(argv[++i]),1,5);
    }
    if(previewLanding) smokeStage=1;
    if(smokeUpgrades) smokeStage=4;
    if(previewMars) smokeStage=5;
    glfwSetErrorCallback([](int code,const char* message){ std::cerr<<"GLFW "<<code<<": "<<message<<'\n'; });
    if(!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE);
    if(smoke) glfwWindowHint(GLFW_VISIBLE,GL_FALSE);
    GLFWwindow* window=glfwCreateWindow(1280,720,"Red Drift | Earth to Mars",nullptr,nullptr);
    if(!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window); glfwSwapInterval(smoke?0:1);
    glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK) { std::cerr<<"Could not initialize GLEW\n"; glfwDestroyWindow(window); glfwTerminate(); return 1; }
    while(glGetError()!=GL_NO_ERROR) {}
    int result=0;
    try {
        Renderer renderer(executableDirectory(argv[0])/"shaders");
        std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<"\nWASD/arrows: move | LMB/Space: bullets | RMB: laser (uses Energy) | E: homing missiles | Shift: accelerate | P: pause | R: restart | M: levels | F11: fullscreen\n";
        Level game;
        const auto saveFile=progressFile(executableDirectory(argv[0]));
        if(!smoke && std::filesystem::exists(saveFile) && !ProgressStore::load(saveFile,game.profile,game.completed))
            std::cerr<<"Could not load progress; starting with a fresh profile.\n";
        if(smokeUpgrades) {
            game.profile.workshopUnlocked=true; game.profile.sol=smokeWeaponLevel>1?10000:1000;
            game.openWorkshop(); game.buy(Upgrade::Hull); game.buy(Upgrade::Energy); game.buy(Upgrade::Laser); game.buy(Upgrade::Missiles);
            for(int tier=1;tier<smokeWeaponLevel;++tier) { game.buy(Upgrade::Laser); game.buy(Upgrade::Missiles); }
        }
        if(smoke) game.start(smokeStage);
        if(previewLanding || previewMars) {
            game.skipScene();
            game.distance=game.definition().length-0.02f;
            game.cameraX=game.distance+4;
            game.player.position={game.distance,0};
        }
        if(smokeUpgrades) {
            const glm::vec2 target(15,3);
            game.enemies.push_back({target,target,3,0,100,4,true});
            if(smokeWeaponLevel>=3) {
                const glm::vec2 second(20,-3);
                game.enemies.push_back({second,second,-3,0,100,4,true});
            }
            game.asteroids.push_back({{8,0},0.8f,0});
        }
        bool previousMouse=false, mouseFireArmed=true;
        std::array<bool,GLFW_KEY_LAST+1> previous{};
        auto pressed = [&](int key) {
            const bool down=glfwGetKey(window,key)==GLFW_PRESS;
            const bool edge=down&&!previous[key]; previous[key]=down; return edge;
        };
        auto down = [&](int key) { return glfwGetKey(window,key)==GLFW_PRESS; };
        int windowX=100,windowY=100,windowW=1280,windowH=720;
        double last=glfwGetTime(),accumulator=0;
        constexpr double Step=1.0/120.0;
        int frames=0;
        bool reportedEnemy=false;
        bool savedExplosion=false,savedMissile=false;
        double nextSaveRetry=0;
        const float fullSeconds=game.definition().length/Player::CruiseSpeed+
            (smokeStage==0?Level::LaunchDuration:smokeStage==1?Level::EncounterDuration+Level::LandingDuration:smokeStage==2?Level::TakeoffDuration:smokeStage==5?Level::MarsFinaleDuration:0)+2;
        const int smokeFrames=smokeFull?static_cast<int>(fullSeconds*60):previewMars?static_cast<int>((Level::MarsFinaleDuration+2)*60):previewLanding?840:smokeStage==0?690:smokeStage==1?720:smokeStage==2?780:360;
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            const double now=glfwGetTime();
            const double elapsed=smoke?1.0/60.0:std::clamp(now-last,0.0,0.1); last=now;
            int clientW,clientH; glfwGetWindowSize(window,&clientW,&clientH);
            double mouseX,mouseY; glfwGetCursorPos(window,&mouseX,&mouseY);
            const glm::vec2 cursor=Menu::cursor(mouseX,mouseY,clientW,clientH);
            const bool mouseDown=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
            const bool click=(mouseDown&&!previousMouse) || (smokeFull && (game.state==MissionState::Landing || game.state==MissionState::MarsFinale) && frames%7==0);
            previousMouse=mouseDown;
            if(!mouseDown) mouseFireArmed=true;
            const bool escape=pressed(GLFW_KEY_ESCAPE),menuPressed=pressed(GLFW_KEY_M);
            const bool backClick=click && (game.state==MissionState::Paused || game.state==MissionState::Won || game.state==MissionState::Lost) && Menu::Back.contains(cursor);
            if(escape || menuPressed || backClick) {
                if(escape && game.state==MissionState::Ready) glfwSetWindowShouldClose(window,GLFW_TRUE);
                else { game.openMenu(); accumulator=0; mouseFireArmed=false; }
            }
            if(pressed(GLFW_KEY_F11)) {
                if(glfwGetWindowMonitor(window)) glfwSetWindowMonitor(window,nullptr,windowX,windowY,windowW,windowH,GLFW_DONT_CARE);
                else {
                    GLFWmonitor* monitor=glfwGetPrimaryMonitor(); const GLFWvidmode* mode=glfwGetVideoMode(monitor);
                    if(mode) {
                        glfwGetWindowPos(window,&windowX,&windowY); glfwGetWindowSize(window,&windowW,&windowH);
                        glfwSetWindowMonitor(window,monitor,0,0,mode->width,mode->height,mode->refreshRate);
                    }
                }
            }
            const bool enter=pressed(GLFW_KEY_ENTER),pause=pressed(GLFW_KEY_P),restart=pressed(GLFW_KEY_R);
            const bool shopKey=pressed(GLFW_KEY_W);
            const bool left=pressed(GLFW_KEY_LEFT),right=pressed(GLFW_KEY_RIGHT),up=pressed(GLFW_KEY_UP),downArrow=pressed(GLFW_KEY_DOWN);
            const bool tabKey=pressed(GLFW_KEY_TAB);
            std::array<bool,Level::StageCount> numbers{};
            for(int i=0;i<Level::StageCount;++i) {
                numbers[i]=pressed(GLFW_KEY_1+i);
                if(numbers[i] && game.state==MissionState::Ready) game.selectedLevel=i;
            }
            const bool shopRequest=shopKey || (click && game.state==MissionState::Ready && Menu::Workshop.contains(cursor));
            if(shopRequest && (game.state==MissionState::Ready || game.state==MissionState::Won || game.state==MissionState::Lost)) {
                game.openWorkshop(); accumulator=0; mouseFireArmed=false;
            } else if(game.state==MissionState::Workshop) {
                if(tabKey || left || right) game.workshopTab=1-game.workshopTab;
                for(int i=0;i<2;++i) if(click && Menu::shopTab(i).contains(cursor)) game.workshopTab=i;
                for(int i=0;i<2;++i) if(numbers[i] || (click && Menu::buyButton(i).contains(cursor))) game.buy(Menu::shopUpgrade(game.workshopTab,i));
                if(enter || (click && Menu::ShopBack.contains(cursor))) game.openMenu();
            } else if(game.workshopNotice) {
                if(enter || (click && Menu::Primary.contains(cursor))) game.openWorkshop();
                accumulator=0; mouseFireArmed=false;
            } else if(game.state==MissionState::Ready) {
                if(left) game.selectedLevel=(game.selectedLevel+Level::StageCount-1)%Level::StageCount;
                if(right) game.selectedLevel=(game.selectedLevel+1)%Level::StageCount;
                if(up||downArrow) game.selectedLevel=(game.selectedLevel+3)%Level::StageCount;
                const int card=Menu::cardAt(cursor);
                if(click && card>=0) game.selectedLevel=card;
                if(enter || (click && Menu::Start.contains(cursor))) {
                    game.start(game.selectedLevel); accumulator=0; mouseFireArmed=false;
                }
            } else if(restart) {
                game.reset(); accumulator=0; mouseFireArmed=false;
            } else if(game.cinematicActive() && game.state!=MissionState::Paused && game.state!=MissionState::Landing && game.state!=MissionState::MarsFinale && (enter || click)) {
                game.skipScene(); accumulator=0; mouseFireArmed=false;
            } else if((enter || (click && Menu::Primary.contains(cursor))) && (game.state==MissionState::Won || game.state==MissionState::Lost || game.state==MissionState::Paused)) {
                if(game.state==MissionState::Won) game.advance();
                else if(game.state==MissionState::Lost) game.reset();
                else if(game.state==MissionState::Paused) game.resume();
                accumulator=0; mouseFireArmed=false;
            } else if(pause) {
                if(game.state==MissionState::Paused) game.resume();
                else game.pause();
                accumulator=0;
            }
            if(!smoke && (!glfwGetWindowAttrib(window,GLFW_FOCUSED) || glfwGetWindowAttrib(window,GLFW_ICONIFIED))) {
                game.pause();
            }
            FlightInput input;
            input.direction.x=static_cast<float>(down(GLFW_KEY_D)||down(GLFW_KEY_RIGHT))-static_cast<float>(down(GLFW_KEY_A)||down(GLFW_KEY_LEFT));
            input.direction.y=static_cast<float>(down(GLFW_KEY_W)||down(GLFW_KEY_UP))-static_cast<float>(down(GLFW_KEY_S)||down(GLFW_KEY_DOWN));
            input.fire=down(GLFW_KEY_SPACE)||(mouseDown&&mouseFireArmed); input.boost=down(GLFW_KEY_LEFT_SHIFT)||down(GLFW_KEY_RIGHT_SHIFT);
            input.skipCinematic=down(GLFW_KEY_ENTER);
            input.missile=down(GLFW_KEY_E);
            input.laser=glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;
            if(smoke) { input.fire=true; input.direction.y=std::sin(frames*0.03f); }
            if(smokeUpgrades) { input.missile=true; input.laser=frames%360<240; input.fire=!input.laser; input.direction.y=frames<240?0.0f:frames<300?1.0f:-1.0f; }
            if(smokeUpgrades && frames==300) {
                const auto target=game.player.position+glm::vec2(7,0);
                game.enemies.push_back({target,target,target.y,-game.time*1.8f,100,1,false});
                const auto shot=game.player.position+glm::vec2(3,0);
                game.bullets.push_back({shot,shot,{600,0},false,1});
            }
            if(smokeFull) game.player.invulnerability=2.0f;
            const auto beforeUpdate=game.state;
            if(game.state==MissionState::Flying || (game.cinematicActive() && game.state!=MissionState::Paused)) {
                accumulator+=elapsed;
                while(accumulator>=Step) { game.update(static_cast<float>(Step),input); accumulator-=Step; }
            } else accumulator=0;
            if(smokeFull && beforeUpdate==MissionState::Flying && game.state==MissionState::Landing)
                std::cout<<"LANDING START: full route completed; shooting clicks will continue during descent\n";
            if(smokeFull && !reportedEnemy && !game.enemies.empty()) {
                std::cout<<"FIRST ENEMY: route="<<game.progress()*100<<"%, heavy="<<game.enemies.front().heavy<<'\n';
                reportedEnemy=true;
            }
            if(smokeFull && beforeUpdate==MissionState::Landing && game.state==MissionState::Won)
                std::cout<<"TOUCHDOWN: landing scene played for "<<game.landingTime<<" seconds\n";
            if(smokeFull && beforeUpdate==MissionState::Flying && game.state==MissionState::MarsFinale)
                std::cout<<"MARS FINALE START: eight heavy pursuers incoming\n";
            if(smokeFull && beforeUpdate==MissionState::MarsFinale && game.state==MissionState::Won)
                std::cout<<"MARS RESCUE COMPLETE: all eight pursuers destroyed; "<<game.finaleTime<<" seconds\n";
            if(!smoke && game.progressDirty && now>=nextSaveRetry) {
                if(ProgressStore::save(saveFile,game.profile,game.completed)) game.progressDirty=false;
                else {
                    game.shopMessage="Progress could not be saved. Check disk access.";
                    std::cerr<<"Progress save failed: "<<saveFile<<'\n'; nextSaveRetry=now+5;
                }
            }
            int width,height; glfwGetFramebufferSize(window,&width,&height);
            if(smoke && frames==0 && !capture.empty()) {
                Level menu;
                menu.selectedLevel=smokeStage;
                renderer.draw(menu,0.0f,width,height);
                const std::filesystem::path imagePath(capture);
                screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-menu.bmp")).string(),width,height);
                if(smokeUpgrades) {
                    Level shop=game; shop.state=MissionState::Workshop; shop.shopMessage="Upgrades installed. Ready for the next flight.";
                    renderer.draw(shop,0,width,height);
                    screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-workshop.bmp")).string(),width,height);
                    shop.workshopTab=1;
                    renderer.draw(shop,0,width,height);
                    screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-workshop-weapons.bmp")).string(),width,height);
                }
            }
            renderer.draw(game,smoke?frames/60.0f:static_cast<float>(now),width,height,cursor);
            if(previewMars && !capture.empty() && (frames==270 || frames==630 || frames==756 || frames==828 || frames==930 || frames==1044 || frames==1116 || frames==1392 || frames==1560)) {
                const std::filesystem::path imagePath(capture);
                screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-mars-"+std::to_string(frames)+".bmp")).string(),width,height);
            }
            if(smokeUpgrades && !capture.empty()) {
                const std::filesystem::path imagePath(capture);
                auto effectScreenshot=[&](const std::string& suffix) {
                    screenshot((imagePath.parent_path()/(imagePath.stem().string()+suffix+".bmp")).string(),width,height);
                };
                if(!savedExplosion && std::any_of(game.explosions.begin(),game.explosions.end(),[&](const Explosion& blast) {
                    return blast.age>0.16f && std::abs(blast.position.x-game.cameraX)<14 && std::abs(blast.position.y)<7;
                })) {
                    effectScreenshot("-explosion"); savedExplosion=true;
                }
                if(!savedMissile && std::any_of(game.bullets.begin(),game.bullets.end(),[](const Bullet& bullet){return bullet.homing && bullet.life<3.98f;})) {
                    effectScreenshot("-missile"); savedMissile=true;
                }
                if(frames==285 || frames==345) effectScreenshot("-thrusters-"+std::to_string(frames));
            }
            if(smokeUpgrades && !capture.empty() && (frames==90 || frames==120 || frames==239)) {
                const std::filesystem::path imagePath(capture);
                screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-laser-"+std::to_string(frames)+".bmp")).string(),width,height);
            }
            if(smoke && (smokeStage<=2 || previewLanding) && !capture.empty() && (frames==45 || frames==150 || frames==285 || (previewLanding && (frames==480 || frames==660)))) {
                const std::filesystem::path imagePath(capture);
                const std::string scene=previewLanding?"landing":smokeStage==1?"encounter":smokeStage==2?"takeoff":"launch";
                screenshot((imagePath.parent_path()/(imagePath.stem().string()+"-"+scene+"-"+std::to_string(frames)+".bmp")).string(),width,height);
            }
            if(smoke && frames==smokeFrames-1 && !capture.empty()) screenshot(capture,width,height);
            glfwSwapBuffers(window);
            ++frames;
            if(smoke && frames>=smokeFrames) {
                const GLenum error=glGetError();
                if(error!=GL_NO_ERROR) throw std::runtime_error("OpenGL smoke test failed: "+std::to_string(error));
                if(smokeFull && (game.state!=MissionState::Won || (smokeStage==1 && game.landingTime<Level::LandingDuration)))
                    throw std::runtime_error("Full-level smoke test failed to finish the landing sequence");
                if(smokeFull && smokeStage==5 && (game.finaleTime<Level::MarsFinaleDuration || !std::all_of(game.finaleDestroyed.begin(),game.finaleDestroyed.end(),[](bool value){return value;})))
                    throw std::runtime_error("Mars rescue sequence did not finish");
                std::cout<<"SMOKE PASS: "<<frames<<" frames, distance="<<game.distance<<", enemies="<<game.enemies.size()<<'\n';
                break;
            }
            if(glfwGetWindowAttrib(window,GLFW_ICONIFIED)) glfwWaitEventsTimeout(0.05);
        }
        if(!smoke && game.progressDirty && !ProgressStore::save(saveFile,game.profile,game.completed)) std::cerr<<"Final progress save failed.\n";
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; result=1; }
    glfwDestroyWindow(window); glfwTerminate(); return result;
}
