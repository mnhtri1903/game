#include "Game.h"
#include "SaveSystem.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

float SquareJumpGame::randf(float lo, float hi) {
    return lo+(hi-lo)*(static_cast<float>(std::rand())/static_cast<float>(RAND_MAX));
}
float SquareJumpGame::clampf(float v, float lo, float hi) { return std::max(lo,std::min(hi,v)); }
float SquareJumpGame::lerpf(float a, float b, float t)   { return a+(b-a)*t; }
float SquareJumpGame::getSummerShoreX() const {
    for (const Platform& pf:levelData.platforms)
        if (pf.isGround) return pf.x+pf.w;
    return 0.0f;
}
float SquareJumpGame::getSummerWaveAmplitude(float worldX) const {
    float shoreFactor=1.0f-clampf((worldX-getSummerShoreX())/SUMMER_WAVE_SHORE_RANGE,0.0f,1.0f);
    return SUMMER_WAVE_BASE_HEIGHT+shoreFactor*SUMMER_WAVE_SHORE_BOOST;
}
float SquareJumpGame::getSummerWaterSurfaceY(float worldX) const {
    float amp=getSummerWaveAmplitude(worldX);
    float swell=std::sin(ticks*0.045f-worldX*0.014f+0.8f);
    float chop=std::sin(ticks*0.12f-worldX*0.031f+2.1f);
    return levelData.waterLineY+swell*amp+chop*(2.5f+amp*0.16f);
}
float SquareJumpGame::getSummerWavePush(float worldX) const {
    float shoreFactor=1.0f-clampf((worldX-getSummerShoreX())/SUMMER_WAVE_BREAK_RANGE,0.0f,1.0f);
    float pulse=std::max(0.0f,std::sin(ticks*0.09f-worldX*0.018f+0.5f));
    return pulse*pulse*(0.18f+shoreFactor*1.05f);
}
SDL_Color SquareJumpGame::alpha(SDL_Color c, Uint8 a)    { c.a=a; return c; }
SDL_Color SquareJumpGame::blend(SDL_Color a, SDL_Color b, float t) {
    t=clampf(t,0,1);
    return {static_cast<Uint8>(a.r+(b.r-a.r)*t),static_cast<Uint8>(a.g+(b.g-a.g)*t),
            static_cast<Uint8>(a.b+(b.b-a.b)*t),static_cast<Uint8>(a.a+(b.a-a.a)*t)};
}

std::string SquareJumpGame::findSystemFont() {
    static const char* paths[] = {
        "font.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        nullptr
    };
    for (int i=0;paths[i];i++) {
        SDL_IOStream* io=SDL_IOFromFile(paths[i],"rb");
        if (io) { SDL_CloseIO(io); return paths[i]; }
    }
    return "";
}

TTF_Font* SquareJumpGame::pickFont(float scale) const {
    if (scale>=4.0f) return fontXL;
    if (scale>=3.0f) return fontL;
    if (scale>=2.0f) return fontM;
    return fontS;
}

void SquareJumpGame::computeInitialWindowSize(int& w, int& h) const {
    w=DEFAULT_WINDOW_WIDTH; h=DEFAULT_WINDOW_HEIGHT;
    SDL_DisplayID disp=SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode=SDL_GetDesktopDisplayMode(disp);
    if (!mode) return;
    w=std::max(MIN_WINDOW_WIDTH,static_cast<int>(mode->w*0.82f));
    h=std::max(MIN_WINDOW_HEIGHT,static_cast<int>(mode->h*0.82f));
    w=std::min(w,mode->w); h=std::min(h,mode->h);
}
void SquareJumpGame::refreshWindowMetrics() { SDL_GetWindowSize(window,&screenW,&screenH); }
void SquareJumpGame::toggleFullscreen() {
    fullscreen=!fullscreen;
    SDL_SetWindowFullscreen(window,fullscreen);
    refreshWindowMetrics(); resetAmbient();
}

bool SquareJumpGame::init() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (!SDL_Init(SDL_INIT_VIDEO)) return false;
    if (!TTF_Init()) { SDL_Quit(); return false; }

    int iw=DEFAULT_WINDOW_WIDTH,ih=DEFAULT_WINDOW_HEIGHT;
    computeInitialWindowSize(iw,ih);
    window=SDL_CreateWindow("Square Jump",iw,ih,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) { TTF_Quit(); SDL_Quit(); return false; }
    SDL_SetWindowMinimumSize(window,MIN_WINDOW_WIDTH,MIN_WINDOW_HEIGHT);
    renderer=SDL_CreateRenderer(window,nullptr);
    if (!renderer) { SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit(); return false; }

    playerTexture=SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,PLAYER_TEXTURE_SIZE,PLAYER_TEXTURE_SIZE);
    if (!playerTexture) {
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); TTF_Quit(); SDL_Quit(); return false;
    }
    SDL_SetTextureBlendMode(playerTexture,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);

    spriteSheet=IMG_LoadTexture(renderer,"player1.png");
    if (spriteSheet) SDL_SetTextureBlendMode(spriteSheet,SDL_BLENDMODE_BLEND);

    std::string fp=findSystemFont();
    if (!fp.empty()) {
        fontS  = TTF_OpenFont(fp.c_str(),13.0f);
        fontM  = TTF_OpenFont(fp.c_str(),20.0f);
        fontL  = TTF_OpenFont(fp.c_str(),28.0f);
        fontXL = TTF_OpenFont(fp.c_str(),40.0f);
    }

    refreshWindowMetrics();
    mouseScreenX=screenW*0.5f; mouseScreenY=screenH*0.5f;
    hasSave=SaveSystem::exists("savegame.dat");
    resetAmbient();
    return true;
}

void SquareJumpGame::run() {
    while (running) {
        Uint64 t0=SDL_GetTicks();
        handleEvents(); update(); draw();
        Uint64 dt=SDL_GetTicks()-t0;
        if (dt<16) SDL_Delay(static_cast<Uint32>(16-dt));
    }
}

void SquareJumpGame::shutdown() {
    if (fontS)  { TTF_CloseFont(fontS);  fontS=nullptr; }
    if (fontM)  { TTF_CloseFont(fontM);  fontM=nullptr; }
    if (fontL)  { TTF_CloseFont(fontL);  fontL=nullptr; }
    if (fontXL) { TTF_CloseFont(fontXL); fontXL=nullptr; }
    if (playerTexture) { SDL_DestroyTexture(playerTexture); playerTexture=nullptr; }
    if (spriteSheet)   { SDL_DestroyTexture(spriteSheet);   spriteSheet=nullptr; }
    if (renderer)      { SDL_DestroyRenderer(renderer);     renderer=nullptr; }
    if (window)        { SDL_DestroyWindow(window);         window=nullptr; }
    TTF_Quit(); SDL_Quit();
}

void SquareJumpGame::saveGame() {
    SaveData d;
    d.level=currentLevel; d.lixiCount=lixiCount; d.playerHealth=player.health;
    d.jumpLevel=upgrades.jumpLevel; d.chargeLevel=upgrades.chargeLevel;
    d.healthLevel=upgrades.healthLevel; d.heatLevel=upgrades.heatLevel;
    d.hasShovel=upgrades.hasShovel;
    SaveSystem::save("savegame.dat",d); hasSave=true;
}

void SquareJumpGame::loadGame() {
    SaveData d;
    if (!SaveSystem::load("savegame.dat",d)) return;
    lixiCount=d.lixiCount;
    upgrades.jumpLevel=d.jumpLevel; upgrades.chargeLevel=d.chargeLevel;
    upgrades.healthLevel=d.healthLevel; upgrades.heatLevel=d.heatLevel;
    upgrades.hasShovel=d.hasShovel;
    startGame(d.level);
    player.health=std::min(d.playerHealth,upgrades.effectiveMaxHealth());
}

int SquareJumpGame::getSeasonFromLevel(int level) const {
    if (level<=10)  return SEASON_DAY;
    if (level<=20)  return SEASON_NIGHT;
    if (level<=30)  return SEASON_SPRING;
    if (level<=60)  return SEASON_SUMMER;
    if (level<=90)  return SEASON_AUTUMN;
    if (level<=120) return SEASON_WINTER;
    return SEASON_DESERT;
}

Theme SquareJumpGame::themeForLevel(int level) const {
    switch(getSeasonFromLevel(level)) {
        case SEASON_DAY:    return DAY_THEME;
        case SEASON_NIGHT:  return NIGHT_THEME;
        case SEASON_SPRING: return SPRING_THEME;
        case SEASON_SUMMER: return SUMMER_THEME;
        case SEASON_AUTUMN: return AUTUMN_THEME;
        case SEASON_WINTER: return WINTER_THEME;
        case SEASON_DESERT: return DESERT_THEME;
        default: return LOOP_THEMES[(level-151)%3];
    }
}

void SquareJumpGame::resetSeasonStats(int s) {
    if (s==SEASON_SUMMER) player.heat=0;
    if (s==SEASON_AUTUMN) player.hunger=HUNGER_MAX;
    if (s==SEASON_DESERT) { player.thirst=THIRST_MAX; player.jumpsLeft=DESERT_JUMP_MAX; player.camelIndex=-1; }
    if (s==SEASON_DAY||s==SEASON_NIGHT) { player.spinTimer=0; }
}

void SquareJumpGame::beginNewGame() {
    lixiCount=DEFAULT_LIXI; currentLevel=DEFAULT_START_LEVEL; upgrades={}; player={};
    startGame(currentLevel);
}
void SquareJumpGame::loadAndContinue() { loadGame(); }
void SquareJumpGame::openShop()        { saveGame(); state=GameState::Shop; shopHover=-1; }
void SquareJumpGame::closeShop()       { state=GameState::Menu; }
void SquareJumpGame::openPauseMenu()   { state=GameState::PauseMenu; }
void SquareJumpGame::closePauseMenu()  { state=GameState::Playing; }
void SquareJumpGame::openLevelSelect() { levelSelectScroll=0; state=GameState::LevelSelect; }

void SquareJumpGame::returnToMenu() {
    state=GameState::Menu; currentTheme=SPRING_THEME; player={};
    particles.clear(); camX=camY=0; checkpointActive=false;
}

void SquareJumpGame::startGame(int level) {
    int prevSeason=(currentLevel>0)?getSeasonFromLevel(currentLevel):-1;
    currentLevel=level;
    int cur=getSeasonFromLevel(level);
    levelData=generateLevel(level);
    currentTheme=levelData.theme;
    player.x=levelData.startX; player.y=levelData.startY;
    player.vx=player.vy=0; player.onGround=false; player.charging=false; player.chargeTime=0;
    player.lastAirJumpTick=AIR_JUMP_COOLDOWN;
    player.maxHealth=upgrades.effectiveMaxHealth();
    player.ridingBuoyIndex=-1;
    summerWaveKickCooldown=0;
    if (player.health<=0) player.health=player.maxHealth;
    if (cur!=prevSeason) resetSeasonStats(cur);
    checkpointActive=false; particles.clear();
    camX=0; camY=std::max(0.0f,levelData.worldH-static_cast<float>(screenH));
    state=GameState::Playing;
    if (prevSeason>=0&&cur!=prevSeason) {
        transitionTimer=200; transitionSeason=cur; state=GameState::SeasonTransition;
    }
    saveGame();
}

void SquareJumpGame::startMarket() {
    currentTheme=SPRING_THEME; levelData={};
    levelData.theme=SPRING_THEME; levelData.isSpring=true; levelData.isMarket=true;
    levelData.season=SEASON_SPRING;
    float wW=static_cast<float>(screenW)*4.6f, wH=static_cast<float>(screenH);
    levelData.worldW=wW; levelData.worldH=wH;
    levelData.platforms.push_back({0,wH-44,wW,44,true});
    levelData.stalls.push_back({600,wH-112,120,68,2,"Bánh chưng",false});
    levelData.stalls.push_back({1180,wH-112,120,68,5,"Áo mới",false});
    levelData.stalls.push_back({1760,wH-112,120,68,10,"Cây mai",false});
    levelData.gate={wW-170,wH-124,56,76};
    levelData.startX=50; levelData.startY=wH-44-PLAYER_HEIGHT-10;
    player={}; player.x=levelData.startX; player.y=levelData.startY;
    player.lastAirJumpTick=AIR_JUMP_COOLDOWN;
    player.maxHealth=upgrades.effectiveMaxHealth();
    particles.clear(); camX=camY=0; state=GameState::Market;
}

void SquareJumpGame::handleLevelComplete() {
    saveGame();
    if (state==GameState::Market) startGame(31);
    else if (currentLevel==LEVEL_MAX) state=GameState::Win;
    else if (currentLevel==30) state=GameState::MarketPrompt;
    else startGame(currentLevel+1);
}

void SquareJumpGame::triggerDeath() {
    state=GameState::Dead; deathTimer=0; player.vy=-4; spawnDamageBurst();
}

void SquareJumpGame::respawnAtCheckpoint() {
    if (checkpointActive) {
        player.x=checkpointX; player.y=checkpointY-PLAYER_HEIGHT;
        player.vx=player.vy=0; player.health=std::max(10,player.health);
    } else {
        player.x=levelData.startX; player.y=levelData.startY;
        player.vx=player.vy=0; player.health=std::max(10,player.maxHealth/2);
    }
    player.ridingBuoyIndex=-1;
    summerWaveKickCooldown=0;
}

void SquareJumpGame::takeDamage(int amount) {
    if (player.invincible) return;
    player.health-=amount; player.damageFlashTimer=DAMAGE_FLASH_DURATION;
    if (player.health<=0) { player.health=0; triggerDeath(); }
}
void SquareJumpGame::heal(float amount) {
    player.health=std::min(player.maxHealth,static_cast<int>(player.health+amount));
}
void SquareJumpGame::activateInvincibility() {
    player.invincible=true; player.lastInvincibleTick=ticks;
}

void SquareJumpGame::buyUpgrade(int item) {
    switch(item) {
        case 0: if(upgrades.jumpLevel<SHOP_JUMP_MAX_LEVEL&&lixiCount>=SHOP_JUMP_COST){lixiCount-=SHOP_JUMP_COST;upgrades.jumpLevel++;spawnPurchaseBurst(screenW*0.5f,screenH*0.5f);} break;
        case 1: if(upgrades.chargeLevel<SHOP_CHARGE_MAX_LEVEL&&lixiCount>=SHOP_CHARGE_COST){lixiCount-=SHOP_CHARGE_COST;upgrades.chargeLevel++;spawnPurchaseBurst(screenW*0.5f,screenH*0.5f);} break;
        case 2: if(upgrades.healthLevel<SHOP_HEALTH_MAX_LEVEL&&lixiCount>=SHOP_HEALTH_COST){lixiCount-=SHOP_HEALTH_COST;upgrades.healthLevel++;player.maxHealth=upgrades.effectiveMaxHealth();heal(SHOP_HEALTH_BONUS);spawnPurchaseBurst(screenW*0.5f,screenH*0.5f);} break;
        case 3: if(upgrades.heatLevel<SHOP_HEAT_MAX_LEVEL&&lixiCount>=SHOP_HEAT_COST){lixiCount-=SHOP_HEAT_COST;upgrades.heatLevel++;spawnPurchaseBurst(screenW*0.5f,screenH*0.5f);} break;
        case 4: if(lixiCount>=SHOP_FULL_HEAL_COST){lixiCount-=SHOP_FULL_HEAL_COST;player.health=player.maxHealth;spawnPurchaseBurst(screenW*0.5f,screenH*0.5f);} break;
    }
    saveGame();
}

void SquareJumpGame::updateShop(float mx, float my) {
    shopHover=-1;
    float itemH=64,itemW=420,startY=screenH*0.5f-170,startX=screenW*0.5f-itemW*0.5f;
    for (int i=0;i<5;i++) {
        SDL_FRect r={startX,startY+i*(itemH+12),itemW,itemH};
        if (pointInRect(mx,my,r)) { shopHover=i; break; }
    }
}

void SquareJumpGame::resetAmbient() {
    menuStars.clear();
    for (int i=0;i<MENU_STAR_COUNT;i++)
        menuStars.push_back({randf(0,static_cast<float>(screenW)),randf(0,static_cast<float>(screenH)),
                             randf(1,2.8f),randf(0.25f,0.8f),randf(0,PI*2)});
    petals.clear();
    for (int i=0;i<SPRING_PETAL_COUNT;i++) petals.push_back(createPetal(true));
}

Petal SquareJumpGame::createPetal(bool randomY) const {
    Petal p;
    p.x=randf(-80,static_cast<float>(screenW)+80);
    p.y=randomY?randf(-60,static_cast<float>(screenH)+60):randf(-160,-10);
    p.z=randf(0.2f,1); p.speed=randf(0.8f,2.6f); p.drift=randf(-1.2f,1.2f);
    p.angle=randf(0,360); p.spin=randf(-2,2); p.size=randf(2.5f,6);
    return p;
}

LevelData SquareJumpGame::generateLevel(int levelNum) {
    switch(getSeasonFromLevel(levelNum)) {
        case SEASON_DAY:    return generateDayLevel(levelNum);
        case SEASON_NIGHT:  return generateNightLevel(levelNum);
        case SEASON_SUMMER: return generateSummerLevel(levelNum);
        case SEASON_AUTUMN: return generateAutumnLevel(levelNum);
        case SEASON_WINTER: return generateWinterLevel(levelNum);
        case SEASON_DESERT: return generateDesertLevel(levelNum);
        default: return generateSpringLevel(levelNum);
    }
}

LevelData SquareJumpGame::generateSpringLevel(int levelNum) {
    LevelData data;
    data.theme=themeForLevel(levelNum); data.isSpring=true; data.season=SEASON_SPRING;
    int pfCount=PLATFORM_COUNT_MIN+(std::rand()%(PLATFORM_COUNT_MAX-PLATFORM_COUNT_MIN+1));
    float avgRise=(PLATFORM_RISE_MIN+PLATFORM_RISE_MAX)*0.5f;
    data.worldH=static_cast<float>(screenH)+pfCount*avgRise+280;
    float groundY=data.worldH-44;
    data.platforms.push_back({0,groundY,std::max(static_cast<float>(screenW)+600,5000.0f),44,true});
    float curY=groundY-112,curX=220,highX=0;
    for (int i=0;i<pfCount-1;i++) {
        float pw=randf(PLATFORM_WIDTH_MIN,PLATFORM_WIDTH_MAX);
        data.platforms.push_back({curX,curY,pw,22,false});
        highX=curX+pw;
        float rise=randf(PLATFORM_RISE_MIN,PLATFORM_RISE_MAX);
        if (curY-rise<120) rise=randf(20,38);
        curX+=pw+randf(PLATFORM_GAP_MIN,PLATFORM_GAP_MAX); curY-=rise;
    }
    float finalW=randf(185,245);
    data.platforms.push_back({curX,curY,finalW,22,false});
    highX=curX+finalW;
    data.gate={curX+finalW*0.5f-28,curY-76,56,76};
    data.worldW=highX+900; data.platforms.front().w=data.worldW;
    data.startX=84; data.startY=groundY-PLAYER_HEIGHT-10;
    if (data.isSpring&&data.platforms.size()>4) {
        size_t idx=2+(std::rand()%(data.platforms.size()-3));
        const Platform& np=data.platforms[idx];
        data.springNpc={np.x+np.w*0.5f-14,np.y-55,SPRING_NPC_REWARD,false};
        data.hasSpringNpc=true;
    }
    if (levelNum>=15&&data.isSpring&&data.platforms.size()>5) {
        int cc=(levelNum>=25)?2:1;
        for (int c=0;c<cc;c++) {
            size_t idx=3+(std::rand()%(data.platforms.size()-4));
            const Platform& cp=data.platforms[idx];
            ChildNpc child; child.x=cp.x+cp.w*0.5f+20; child.y=cp.y-52;
            data.childNpcs.push_back(child);
        }
    }
    for (int i=0;i<WORLD_STAR_COUNT;i++)
        data.stars.push_back({randf(0,data.worldW),randf(0,data.worldH),randf(1,3),randf(0.2f,0.8f),randf(0,PI*2)});
    if (data.platforms.size()>6) {
        size_t mid=data.platforms.size()/2;
        const Platform& cp=data.platforms[mid];
        data.checkpoints.push_back({cp.x+cp.w*0.5f,cp.y});
    }
    return data;
}

void SquareJumpGame::handleKeyDown(const SDL_KeyboardEvent& key) {
    if (key.repeat) return;
    if (key.key==SDLK_F11) { toggleFullscreen(); return; }
    if (state==GameState::SeasonTransition) { transitionTimer=0; state=GameState::Playing; return; }
    if (state==GameState::Win) { if(key.key==SDLK_ESCAPE||key.key==SDLK_RETURN) returnToMenu(); return; }
    if (state==GameState::Dead) {
        if (key.key==SDLK_R) { player.health=player.maxHealth/2; startGame(currentLevel); }
        else if (key.key==SDLK_ESCAPE) returnToMenu();
        return;
    }
    if (state==GameState::PauseMenu) {
        if (key.key==SDLK_ESCAPE) closePauseMenu(); return;
    }
    if (state==GameState::Shop) {
        if (key.key==SDLK_ESCAPE) closeShop();
        if (key.key>=SDLK_1&&key.key<=SDLK_5) buyUpgrade(key.key-SDLK_1);
        return;
    }
    if (state==GameState::LevelSelect) {
        if (key.key==SDLK_ESCAPE) returnToMenu();
        return;
    }
    if (state==GameState::Menu) {
        if (key.key==SDLK_RETURN||key.key==SDLK_SPACE) { if(hasSave) loadAndContinue(); else beginNewGame(); }
        else if (key.key==SDLK_ESCAPE) running=false;
        return;
    }
    if (state==GameState::MarketPrompt) {
        if (key.key==SDLK_Y) startMarket();
        else if (key.key==SDLK_N) startGame(31);
        else if (key.key==SDLK_ESCAPE) returnToMenu();
        return;
    }
    if (state==GameState::Playing||state==GameState::Market) {
        if (key.key==SDLK_ESCAPE) openPauseMenu();
        if (key.key==SDLK_P) openShop();
    }
}

void SquareJumpGame::handleMouseClick(float x, float y) {
    if (state==GameState::Menu) {
        if (pointInRect(x,y,menuPrimaryBtnRect())) { if(hasSave) loadAndContinue(); else beginNewGame(); }
        else if (hasSave&&pointInRect(x,y,menuNewGameBtnRect())) beginNewGame();
        else if (pointInRect(x,y,menuShopBtnRect()))  openShop();
        else if (pointInRect(x,y,menuLevelBtnRect())) openLevelSelect();
        else if (pointInRect(x,y,menuExitBtnRect()))  running=false;
    } else if (state==GameState::PauseMenu) {
        if (pointInRect(x,y,pauseResumeBtnRect()))  closePauseMenu();
        else if (pointInRect(x,y,pauseRestartBtnRect())) beginNewGame();
        else if (pointInRect(x,y,pauseExitBtnRect())) returnToMenu();
    } else if (state==GameState::LevelSelect) {
        float cellW=54,cellH=44,marginX=(screenW-LEVEL_SELECT_COLS*cellW)*0.5f;
        float startY=140+levelSelectScroll;
        int rows=(LEVEL_MAX+LEVEL_SELECT_COLS-1)/LEVEL_SELECT_COLS;
        for (int r=0;r<rows;r++) {
            for (int c=0;c<LEVEL_SELECT_COLS;c++) {
                int lvl=r*LEVEL_SELECT_COLS+c+1;
                if (lvl>LEVEL_MAX) break;
                SDL_FRect btn={marginX+c*cellW,startY+r*cellH,cellW-4,cellH-4};
                if (pointInRect(x,y,btn)) { startGame(lvl); return; }
            }
        }
        SDL_FRect upBtn={screenW*0.5f-60,90,120,36};
        SDL_FRect dnBtn={screenW*0.5f-60.0f,(float)screenH-50.0f,120.0f,36.0f};
        if (pointInRect(x,y,upBtn)) levelSelectScroll=std::min(0,levelSelectScroll+static_cast<int>(cellH*2));
        if (pointInRect(x,y,dnBtn)) levelSelectScroll-=static_cast<int>(cellH*2);
    } else if (state==GameState::MarketPrompt) {
        if (pointInRect(x,y,promptYesRect())) startMarket();
        else if (pointInRect(x,y,promptNoRect())) startGame(31);
    } else if (state==GameState::Shop) {
        if (shopHover>=0) buyUpgrade(shopHover);
    } else if (state==GameState::Dead) {
        SDL_FRect rb={screenW*0.5f-120,screenH*0.5f+60,240,60};
        SDL_FRect mb={screenW*0.5f-120,screenH*0.5f+140,240,60};
        if (pointInRect(x,y,rb)) { player.health=player.maxHealth/2; startGame(currentLevel); }
        else if (pointInRect(x,y,mb)) returnToMenu();
    }
}

void SquareJumpGame::handleEvents() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type==SDL_EVENT_QUIT) running=false;
        else if (ev.type==SDL_EVENT_WINDOW_RESIZED) { refreshWindowMetrics(); resetAmbient(); }
        else if (ev.type==SDL_EVENT_MOUSE_MOTION) { mouseScreenX=ev.motion.x; mouseScreenY=ev.motion.y; }
        else if (ev.type==SDL_EVENT_MOUSE_BUTTON_DOWN&&ev.button.button==SDL_BUTTON_LEFT) {
            mouseScreenX=ev.button.x; mouseScreenY=ev.button.y;
            handleMouseClick(mouseScreenX,mouseScreenY);
        }
        else if (ev.type==SDL_EVENT_KEY_DOWN) handleKeyDown(ev.key);
        else if (ev.type==SDL_EVENT_MOUSE_WHEEL&&state==GameState::LevelSelect)
            levelSelectScroll-=static_cast<int>(ev.wheel.y*40);
    }
}

void SquareJumpGame::updateSecretShortcut(const bool* keys) {
    bool ctrl=keys[SDL_SCANCODE_LCTRL]||keys[SDL_SCANCODE_RCTRL];
    bool shift=keys[SDL_SCANCODE_LSHIFT]||keys[SDL_SCANCODE_RSHIFT];
    bool combo=ctrl&&shift&&keys[SDL_SCANCODE_B]&&keys[SDL_SCANCODE_L];
    if (combo&&!secretLatched&&state==GameState::Playing) startGame(currentLevel+5);
    secretLatched=combo;
}

void SquareJumpGame::updateAmbient() {
    for (Petal& p:petals) {
        float depth=lerpf(0.4f,1.35f,p.z);
        p.y+=p.speed*depth;
        p.x+=std::sin(ticks*0.012f+p.drift*2.6f)*(0.35f+p.z*0.65f)+p.drift*0.2f;
        p.angle+=p.spin;
        if (p.y>screenH+40) p=createPetal(false);
        if (p.x<-90) p.x=static_cast<float>(screenW)+90;
        else if (p.x>screenW+90) p.x=-90;
    }
}

void SquareJumpGame::updateParticles() {
    for (int i=static_cast<int>(particles.size())-1;i>=0;i--) {
        Particle& p=particles[i]; p.x+=p.vx; p.y+=p.vy; p.vy+=0.06f; p.life-=1;
        if (p.life<=0) particles.erase(particles.begin()+i);
    }
}

static const char* NPC_DIALOGUES[] = {
    "Biết chú là ai không?",
    "Có bạn gái chưa?",
    "Học hành thế nào rồi?",
    "Làm ở đâu vậy cháu?",
    "Lương tháng bao nhiêu?",
    "Chúc mừng năm mới!",
    "Bao giờ cưới vợ?",
    "Tết này về quê không?",
    "Năm nay mấy tuổi rồi?",
    nullptr
};

static const char* CHILD_DIALOGUES[] = {
    "Cho con xin lì xì!",
    "Chú ơi lì xì đi!",
    "Con chúc chú năm mới!",
    "Lì xì đi chú ơi!",
    nullptr
};

static const char* CHILD_ANGRY_DIALOGUES[] = {
    "Không có à?!",
    "Lấy tiền ra đây!",
    "Đá cho biết tay!",
    nullptr
};

void SquareJumpGame::updateSpringNpcAnim() {
    if (!levelData.hasSpringNpc) return;
    SpringNpc& npc=levelData.springNpc;
    npc.animTick++;
    if (npc.animTick>=NPC_ANIM_SPEED) {
        npc.animTick=0;
        npc.animFrame=(npc.animFrame+1)%4;
    }
    npc.dialogueTick++;
    if (npc.dialogueTick>=NPC_DIALOGUE_TICKS) {
        npc.dialogueTick=0;
        int count=0; while(NPC_DIALOGUES[count]) count++;
        npc.dialogueIndex=(npc.dialogueIndex+1)%count;
        npc.talkActive=true;
    }
    if (npc.talkActive&&npc.dialogueTick>NPC_DIALOGUE_TICKS*0.6f) npc.talkActive=false;
}

void SquareJumpGame::updateChildNpcAnim() {
    for (ChildNpc& child:levelData.childNpcs) {
        if (!child.active) continue;
        child.animTick++;
        if (child.animTick>=NPC_ANIM_SPEED) {
            child.animTick=0;
            if (!child.angered) {
                child.animFrame=(child.animFrame+1)%3;
            } else {
                child.kickFrame=(child.kickFrame+1)%6;
                child.animFrame=2;
            }
        }
        child.dialogueTick++;
        if (child.dialogueTick>=NPC_DIALOGUE_TICKS&&!child.angered) {
            child.dialogueTick=0;
            int count=0; while(CHILD_DIALOGUES[count]) count++;
            child.dialogueIndex=(child.dialogueIndex+1)%count;
        }
        if (child.angered) {
            int count=0; while(CHILD_ANGRY_DIALOGUES[count]) count++;
            child.dialogueIndex=child.kickFrame%count;
        }
    }
}

void SquareJumpGame::spawnChargeParticle(const SDL_Color& c) {
    particles.push_back({player.x+player.width*0.5f+randf(-5,5),player.y+player.height,
                         randf(-0.25f,0.25f),randf(0,1),15,15,randf(1,3),c});
}
void SquareJumpGame::spawnJumpBurst(int count, const SDL_Color& c, float sx, float sy) {
    for (int i=0;i<count;i++)
        particles.push_back({player.x+player.width*0.5f,player.y+player.height,
                             randf(-sx,sx),randf(-sy,sy),randf(16,22),randf(16,22),randf(2,5),c});
}
void SquareJumpGame::spawnPurchaseBurst(float x, float y) {
    for (int i=0;i<22;i++)
        particles.push_back({x,y,randf(-4,4),randf(-3.5f,1.5f),randf(18,26),randf(18,26),randf(2,4),
                             (i%2==0)?currentTheme.playerAccent:currentTheme.petalA});
}
void SquareJumpGame::spawnDamageBurst() {
    for (int i=0;i<18;i++)
        particles.push_back({player.x+player.width*0.5f,player.y+player.height*0.5f,
                             randf(-5,5),randf(-5,5),randf(10,20),randf(10,20),randf(2,4),{255,50,50,255}});
}
void SquareJumpGame::spawnSnowParticle() {
    particles.push_back({randf(0,static_cast<float>(screenW)+camX),-10+camY,
                         randf(-2,0.5f),randf(1,3),randf(80,120),randf(80,120),randf(2,5),{220,235,255,200}});
}
void SquareJumpGame::spawnWindParticle(float x, float y, float vxDir, const SDL_Color& c) {
    particles.push_back({x, y+randf(-20,20), vxDir+randf(-0.5f,0.5f), randf(-0.5f,0.5f),
                         randf(20,35), randf(20,35), randf(1.5f,4.0f), c});
}

void SquareJumpGame::resolvePlayerCollisions() {
    player.onGround=false;
    for (Platform& pf:levelData.platforms) {
        if (pf.buoyGone) continue;
        if (!pf.mooncakeActive&&pf.isMooncake) continue;
        if (pf.isBuoy) continue;
        if (pf.isSpike) continue;
        if (pf.isFake && pf.fakeSunk) continue;
        float pcx=player.x+player.width*0.5f, pcy=player.y+player.height*0.5f;
        float fcx=pf.x+pf.w*0.5f, fcy=pf.y+pf.h*0.5f;
        float dx=pcx-fcx, dy=pcy-fcy;
        float hwx=(player.width+pf.w)*0.5f, hwy=(player.height+pf.h)*0.5f;
        if (std::fabs(dx)<hwx&&std::fabs(dy)<hwy) {
            float ox=hwx-std::fabs(dx), oy=hwy-std::fabs(dy);
            if (ox<oy) { player.x+=(dx>0?ox:-ox); player.vx=0; }
            else {
                if (dy>0) { player.y+=oy; player.vy=0; }
                else {
                    player.y-=oy; player.vy=0; player.onGround=true;
                    if (pf.isIcy&&getSeasonFromLevel(currentLevel)==SEASON_WINTER)
                        player.vx*=(WINTER_GROUND_FRICTION/GROUND_FRICTION);
                }
            }
        }
        // Snap-to-ground: fixes exact-boundary (oy==0) and floating-point gap cases.
        // When the player is precisely ON the platform surface the standard AABB overlap
        // check fails because |dy| == hwy (no penetration). We snap them down here.
        else if (std::fabs(dx)<hwx) {
            float playerBottom=player.y+player.height;
            float gap=pf.y-playerBottom;          // distance above platform top
            if (gap>=0.0f&&gap<=2.0f&&player.vy>=0.0f) {
                player.y=pf.y-player.height;
                player.vy=0.0f;
                player.onGround=true;
                if (pf.isIcy&&getSeasonFromLevel(currentLevel)==SEASON_WINTER)
                    player.vx*=(WINTER_GROUND_FRICTION/GROUND_FRICTION);
            }
        }
    }
}

void SquareJumpGame::resolvePlayerCamelCollisions() {
    if (player.camelIndex>=0) return;
    for (int ci=0;ci<static_cast<int>(levelData.camels.size());ci++) {
        Camel& camel=levelData.camels[ci];
        if (!camel.active) continue;
        float cx=camel.x,cy=camel.y-30,cw=60,ch=10;
        if (intersects(player.x,player.y,player.width,player.height,cx,cy,cw,ch)&&player.vy>=0) {
            player.y=cy-player.height; player.vy=0; player.onGround=true;
            if (camel.waterLevel>=CAMEL_WATER_NEED) { player.camelIndex=ci; camel.hasPlayer=true; }
        }
    }
}

void SquareJumpGame::resolvePlayerBuoyCollisions() {
    for (int bi=0;bi<static_cast<int>(levelData.platforms.size());bi++) {
        Platform& pf=levelData.platforms[bi];
        if (!pf.isBuoy||pf.buoyGone) continue;
        if (pf.buoySinkTimer > 0) continue;
        float pcx=player.x+player.width*0.5f, pcy=player.y+player.height*0.5f;
        float fcx=pf.x+pf.w*0.5f, fcy=pf.y+pf.h*0.5f;
        float dx=pcx-fcx, dy=pcy-fcy;
        float hwx=(player.width+pf.w)*0.5f, hwy=(player.height+pf.h)*0.5f;
        if (std::fabs(dx)<hwx&&std::fabs(dy)<hwy) {
            float ox=hwx-std::fabs(dx), oy=hwy-std::fabs(dy);
            if (ox<oy) { player.x+=(dx>0?ox:-ox); player.vx=0; }
            else if (dy<0) {
                player.y=pf.y-player.height; player.vy=pf.buoyVy;
                player.vx+=pf.buoyVx*0.75f;
                player.onGround=true;
                player.ridingBuoyIndex=bi;
            }
        } else if (std::fabs(dx)<hwx) {
            float playerBottom=player.y+player.height;
            float gap=pf.y-playerBottom;
            if (gap>=-1.0f&&gap<=5.0f&&player.vy>=pf.buoyVy-0.5f) {
                player.y=pf.y-player.height; player.vy=pf.buoyVy;
                player.vx+=pf.buoyVx*0.75f;
                player.onGround=true;
                player.ridingBuoyIndex=bi;
            }
        }
    }
}

void SquareJumpGame::releaseChargedJump(bool isDesert) {
    int effCharge=upgrades.effectiveMaxCharge();
    float ratio=static_cast<float>(player.chargeTime)/static_cast<float>(effCharge);
    float power=JUMP_FORCE_MIN+ratio*(upgrades.effectiveJumpMax()-JUMP_FORCE_MIN);
    // Compute direction from player center to mouse in world space
    float mwx=mouseScreenX+camX, mwy=mouseScreenY+camY;
    float pcx=player.x+player.width*0.5f, pcy=player.y+player.height*0.5f;
    float dx=mwx-pcx, dy=mwy-pcy;
    float dist=std::sqrt(dx*dx+dy*dy);
    // Safe default: jump straight up if mouse is on/near player
    if (dist<1.0f) { dx=0.0f; dy=-1.0f; dist=1.0f; }
    float dirX=dx/dist, dirY=dy/dist;
    // Ground jumps must have an upward component — clicking below wastes the jump
    if (player.chargeType==1 && dirY>-0.1f) {
        dirY=-0.1f;
        float mag=std::sqrt(dirX*dirX+dirY*dirY);
        dirX/=mag; dirY/=mag;
    }
    player.vx=dirX*power; player.vy=dirY*power;
    if (player.chargeType==2) player.lastAirJumpTick=ticks;
    player.onGround=false; player.charging=false; player.chargeTime=0; player.chargeType=0;
    player.jumpAnimTimer=12; player.blinkTimer=10; player.blinkRight=(std::rand()%2)==0;
    player.ridingBuoyIndex=-1;
    if (getSeasonFromLevel(currentLevel)==SEASON_AUTUMN)
        player.hunger=std::max(0.0f,player.hunger-HUNGER_PER_JUMP);
    if (isDesert&&player.camelIndex<0)
        player.jumpsLeft=std::max(0,player.jumpsLeft-1);
    SDL_Color burst=(ratio>=1.0f)?currentTheme.gate:SDL_Color{255,255,255,255};
    spawnJumpBurst(15,burst,6,4);
}

void SquareJumpGame::updateCheckpoints() {
    for (Checkpoint& cp:levelData.checkpoints) {
        if (!cp.reached&&intersects(player.x,player.y,player.width,player.height,cp.x-8,cp.y-40,16,40)) {
            cp.reached=true; checkpointX=cp.x; checkpointY=cp.y; checkpointActive=true;
            spawnPurchaseBurst(cp.x,cp.y);
        }
    }
}

void SquareJumpGame::updatePlaying(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    int  effCharge=upgrades.effectiveMaxCharge();
    if (player.onGround){player.vx*=GROUND_FRICTION;if(std::fabs(player.vx)<0.1f)player.vx=0;}
    else player.vx*=AIR_FRICTION;
    if (spaceDown) {
        if (player.onGround) {
            if (!player.charging){player.charging=true;player.chargeType=1;player.chargeTime=0;}
        } else if (!player.charging&&ticks-player.lastAirJumpTick>=AIR_JUMP_COOLDOWN) {
            player.charging=true;player.chargeType=2;player.chargeTime=0;
        }
        if (player.charging) {
            player.chargeTime=std::min(player.chargeTime+1,effCharge);
            if (ticks%3==0) spawnChargeParticle(player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,255,255,255});
        }
    }
    if (!spaceDown&&player.charging) releaseChargedJump();
    if (!player.onGround) {
        if (std::fabs(player.vy)<GLIDE_THRESHOLD) player.vy+=GRAVITY*GLIDE_FACTOR;
        else player.vy+=GRAVITY;
    }
    if (player.vy>18) player.vy=18;
    player.x+=player.vx; player.y+=player.vy;
    player.x=std::max(0.0f,player.x);
    if (player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}
    resolvePlayerCollisions();
    if (player.y>levelData.worldH+80){takeDamage(20);respawnAtCheckpoint();}
    if (levelData.hasSpringNpc&&!levelData.springNpc.gifted) {
        SpringNpc& npc=levelData.springNpc;
        if (intersects(player.x,player.y,player.width,player.height,npc.x,npc.y,28,55)) {
            npc.gifted=true; lixiCount+=npc.reward; spawnPurchaseBurst(npc.x+14,npc.y+12);
        }
    }
    for (ChildNpc& child:levelData.childNpcs) {
        if (!child.active) continue;
        if (intersects(player.x,player.y,player.width,player.height,child.x,child.y,28,52)) {
            if (lixiCount>=child.cost) { lixiCount-=child.cost; child.active=false; spawnPurchaseBurst(child.x+14,child.y+12); }
            else if (!child.angered) {
                child.angered=true; child.angerTimer=60;
                player.vx=(player.x>child.x)?CHILD_KICK_VX:-CHILD_KICK_VX;
                player.vy=CHILD_KICK_VY; takeDamage(CHILD_KICK_DAMAGE);
            }
        }
    }
    updateCheckpoints();
    updateSpringNpcAnim(); updateChildNpcAnim();
    player.prevSpace=spaceDown;
    if (player.invincible&&ticks-player.lastInvincibleTick>=INVINCIBLE_DURATION) player.invincible=false;
    if (player.damageFlashTimer>0) player.damageFlashTimer--;
}

void SquareJumpGame::updateMarket(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    bool spaceJust=spaceDown&&!player.prevSpace;
    if (player.onGround){player.vx*=GROUND_FRICTION;if(std::fabs(player.vx)<0.1f)player.vx=0;}
    else player.vx*=AIR_FRICTION;
    if (spaceJust&&player.onGround) {
        player.vx=17;player.vy=-3;player.jumpAnimTimer=12;player.blinkTimer=10;
        spawnJumpBurst(10,{255,255,255,255},5,2);
    }
    if (!player.onGround){player.vy+=GRAVITY;if(player.vy>10)player.vy=10;}
    player.x+=player.vx;player.y+=player.vy;
    if(player.x<0){player.x=0;player.vx=0;}
    if(player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}
    resolvePlayerCollisions();
    for (Stall& s:levelData.stalls)
        if(!s.bought&&intersects(player.x,player.y,player.width,player.height,s.x,s.y,s.w,s.h))
            if(lixiCount>=s.cost){lixiCount-=s.cost;s.bought=true;spawnPurchaseBurst(s.x+s.w*0.5f,s.y);}
    player.prevSpace=spaceDown;
}

void SquareJumpGame::updatePlayerAnimation() {
    if (player.blinkTimer>0) player.blinkTimer--;
    if (player.charging) {
        if (!player.prevCharging){player.chargeAnimFrame=0;player.chargeAnimTick=0;player.chargeAnimDone=false;}
        if (!player.chargeAnimDone) {
            if (++player.chargeAnimTick>=3){player.chargeAnimTick=0;if(++player.chargeAnimFrame>=3){player.chargeAnimFrame=3;player.chargeAnimDone=true;}}
        }
    }
    player.prevCharging=player.charging;
}

void SquareJumpGame::updateCamera() {
    float tx=player.x-screenW*0.4f+player.width*0.5f;
    tx=clampf(tx,0,std::max(0.0f,levelData.worldW-screenW));
    camX+=(tx-camX)*0.1f;
    float ty=player.y-screenH*0.62f+player.height*0.5f;
    ty=clampf(ty,0,std::max(0.0f,levelData.worldH-screenH));
    camY+=(ty-camY)*0.1f;
}

bool SquareJumpGame::reachedGate() const {
    return intersects(player.x,player.y,player.width,player.height,
                      levelData.gate.x,levelData.gate.y,levelData.gate.w,levelData.gate.h);
}

void SquareJumpGame::update() {
    ticks++; updateAmbient();
    const bool* keys=SDL_GetKeyboardState(nullptr);
    updateSecretShortcut(keys);
    if (state==GameState::Menu||state==GameState::MarketPrompt||state==GameState::Win||
        state==GameState::LevelSelect) { updateParticles(); return; }
    if (state==GameState::PauseMenu) { updateParticles(); return; }
    if (state==GameState::Shop) { updateShop(mouseScreenX,mouseScreenY); updateParticles(); return; }
    if (state==GameState::Dead) { deathTimer++; updateParticles(); return; }
    if (state==GameState::SeasonTransition) { transitionTimer--; if(transitionTimer<=0) state=GameState::Playing; return; }
    updatePlayerAnimation();
    if (state==GameState::Market) updateMarket(keys);
    else {
        switch(getSeasonFromLevel(currentLevel)) {
            case SEASON_DAY:    updateDay(keys);     break;
            case SEASON_NIGHT:  updateNight(keys);   break;
            case SEASON_SPRING: updatePlaying(keys); break;
            case SEASON_SUMMER: updateSummer(keys);  break;
            case SEASON_AUTUMN: updateAutumn(keys);  break;
            case SEASON_WINTER: updateWinter(keys);  break;
            case SEASON_DESERT: updateDesert(keys);  break;
            default: updatePlaying(keys); break;
        }
    }
    updateCamera(); updateParticles();
    if ((state==GameState::Playing||state==GameState::Market)&&reachedGate()) handleLevelComplete();
}

SDL_FRect SquareJumpGame::menuPrimaryBtnRect()  const { return {screenW*0.5f-140,screenH*0.5f-60,280,60}; }
SDL_FRect SquareJumpGame::menuNewGameBtnRect()  const { return {screenW*0.5f-140,screenH*0.5f+12,280,60}; }
SDL_FRect SquareJumpGame::menuShopBtnRect()     const { return {screenW*0.5f-140,screenH*0.5f+84,280,60}; }
SDL_FRect SquareJumpGame::menuLevelBtnRect()    const { return {screenW*0.5f-140,screenH*0.5f+156,280,60}; }
SDL_FRect SquareJumpGame::menuExitBtnRect()     const { return {screenW*0.5f-140,screenH*0.5f+228,280,60}; }
SDL_FRect SquareJumpGame::pauseResumeBtnRect()  const { return {screenW*0.5f-150,screenH*0.5f-20,300,60}; }
SDL_FRect SquareJumpGame::pauseRestartBtnRect() const { return {screenW*0.5f-150,screenH*0.5f+56,300,60}; }
SDL_FRect SquareJumpGame::pauseExitBtnRect()    const { return {screenW*0.5f-150,screenH*0.5f+132,300,60}; }
SDL_FRect SquareJumpGame::promptYesRect()       const { return {screenW*0.5f-170,screenH*0.5f+55,140,60}; }
SDL_FRect SquareJumpGame::promptNoRect()        const { return {screenW*0.5f+30, screenH*0.5f+55,140,60}; }

bool SquareJumpGame::pointInRect(float x, float y, const SDL_FRect& r) const {
    return x>=r.x&&x<=r.x+r.w&&y>=r.y&&y<=r.y+r.h;
}
bool SquareJumpGame::intersects(float ax,float ay,float aw,float ah,float bx,float by,float bw,float bh) const {
    return ax<bx+bw&&ax+aw>bx&&ay<by+bh&&ay+ah>by;
}

void SquareJumpGame::updateFans() {
    for (Fan& fan : levelData.fans) {
        fan.animTick++;
        float px = player.x + player.width * 0.5f;
        float py = player.y + player.height * 0.5f;
        float fanCX = fan.x + fan.w * 0.5f;
        float fanCY = fan.y + fan.h * 0.5f;
        float blowDir = (fan.forceX > 0) ? 1.0f : -1.0f;
        float fanEdgeX = (blowDir > 0) ? (fan.x + fan.w) : fan.x;
        float distAlongWind = blowDir * (px - fanEdgeX);
        bool inWindCone = distAlongWind >= 0 && distAlongWind < fan.rangeW;
        bool inWindHeight = std::fabs(py - fanCY) < fan.rangeH * 0.5f;
        if (inWindCone && inWindHeight) {
            float falloff = 1.0f - clampf(distAlongWind / fan.rangeW, 0.0f, 1.0f);
            falloff = falloff * falloff;
            float impulse = std::fabs(fan.forceX) * falloff * 0.11f;
            player.vx += blowDir * impulse;
            player.vx = clampf(player.vx, -28.0f, 28.0f);
            if (player.onGround && std::fabs(player.vx) > 2.0f)
                player.vx *= 0.96f;
            if (ticks % 3 == 0 && falloff > 0.1f) {
                float spawnX = fanEdgeX + blowDir * randf(0, 20);
                float spawnY = fanCY + randf(-fan.rangeH * 0.35f, fan.rangeH * 0.35f);
                SDL_Color wc = fan.reverse
                    ? SDL_Color{180, 80, 255, 170}
                    : SDL_Color{180, 220, 255, 150};
                spawnWindParticle(spawnX, spawnY, blowDir * std::fabs(fan.forceX) * falloff * 0.5f, wc);
            }
            if (fan.reverse && falloff > 0.45f && player.spinTimer <= 0) {
                player.spinTimer = SPIN_TICKS;
                player.vx *= -1.3f;
                player.vy = std::min(player.vy, -3.5f);
                spawnDamageBurst();
            }
        }
    }
    if (player.spinTimer > 0) {
        player.spinTimer--;
        if (ticks % 2 == 0) {
            particles.push_back({player.x + player.width*0.5f + randf(-16,16),
                                  player.y + player.height*0.5f + randf(-16,16),
                                  randf(-3,3), randf(-3,3), 20, 20, randf(2,5),
                                  SDL_Color{180,60,255,210}});
        }
    }
}

void SquareJumpGame::updateDay(const bool* keys) {
    bool spaceDown = keys[SDL_SCANCODE_SPACE];
    int effCharge = upgrades.effectiveMaxCharge();

    resolvePlayerCollisions();

    if (player.onGround) { player.vx *= GROUND_FRICTION; if (std::fabs(player.vx) < 0.1f) player.vx = 0; }
    else player.vx *= AIR_FRICTION;

    updateFans();

    if (spaceDown) {
        if (player.onGround) {
            if (!player.charging) { player.charging = true; player.chargeType = 1; player.chargeTime = 0; }
        } else if (!player.charging && ticks - player.lastAirJumpTick >= AIR_JUMP_COOLDOWN) {
            player.charging = true; player.chargeType = 2; player.chargeTime = 0;
        }
        if (player.charging) {
            player.chargeTime = std::min(player.chargeTime + 1, effCharge);
            if (ticks % 3 == 0) spawnChargeParticle(player.chargeTime >= effCharge ? currentTheme.gate : SDL_Color{255,255,255,255});
        }
    }
    if (!spaceDown && player.charging) releaseChargedJump();

    if (!player.onGround) {
        if (std::fabs(player.vy) < GLIDE_THRESHOLD) player.vy += GRAVITY * GLIDE_FACTOR;
        else player.vy += GRAVITY;
    }
    if (player.vy > 18) player.vy = 18;
    player.x += player.vx; player.y += player.vy;
    player.x = std::max(0.0f, player.x);
    if (player.x > levelData.gate.x + 800) { player.x = levelData.gate.x + 800; player.vx = 0; }

    resolvePlayerCollisions();
    if (player.y > levelData.worldH + 80) { takeDamage(20); respawnAtCheckpoint(); }
    updateCheckpoints();
    player.prevSpace = spaceDown;
    if (player.invincible && ticks - player.lastInvincibleTick >= INVINCIBLE_DURATION) player.invincible = false;
    if (player.damageFlashTimer > 0) player.damageFlashTimer--;
}

void SquareJumpGame::updateNight(const bool* keys) {
    bool spaceDown = keys[SDL_SCANCODE_SPACE];
    int effCharge = upgrades.effectiveMaxCharge();

    resolvePlayerCollisions();

    if (player.onGround) { player.vx *= GROUND_FRICTION; if (std::fabs(player.vx) < 0.1f) player.vx = 0; }
    else player.vx *= AIR_FRICTION;

    updateFans();
    updateFakePlatforms();
    updateSpikes();

    if (spaceDown) {
        if (player.onGround) {
            if (!player.charging) { player.charging = true; player.chargeType = 1; player.chargeTime = 0; }
        } else if (!player.charging && ticks - player.lastAirJumpTick >= AIR_JUMP_COOLDOWN) {
            player.charging = true; player.chargeType = 2; player.chargeTime = 0;
        }
        if (player.charging) {
            player.chargeTime = std::min(player.chargeTime + 1, effCharge);
            if (ticks % 3 == 0) spawnChargeParticle(player.chargeTime >= effCharge ? currentTheme.gate : SDL_Color{142,45,226,255});
        }
    }
    if (!spaceDown && player.charging) releaseChargedJump();

    if (!player.onGround) {
        if (std::fabs(player.vy) < GLIDE_THRESHOLD) player.vy += GRAVITY * GLIDE_FACTOR;
        else player.vy += GRAVITY;
    }
    if (player.vy > 18) player.vy = 18;
    player.x += player.vx; player.y += player.vy;
    player.x = std::max(0.0f, player.x);
    if (player.x > levelData.gate.x + 800) { player.x = levelData.gate.x + 800; player.vx = 0; }

    resolvePlayerCollisions();
    if (player.y > levelData.worldH + 80) { takeDamage(25); respawnAtCheckpoint(); }
    updateCheckpoints();
    player.prevSpace = spaceDown;
    if (player.invincible && ticks - player.lastInvincibleTick >= INVINCIBLE_DURATION) player.invincible = false;
    if (player.damageFlashTimer > 0) player.damageFlashTimer--;
}
