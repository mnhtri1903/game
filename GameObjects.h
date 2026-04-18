#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include "Constants.h"
#include <string>
#include <vector>

enum class GameState {
    Menu,
    Playing,
    MarketPrompt,
    Market,
    Shop,
    Dead,
    SeasonTransition,
    PauseMenu,
    LevelSelect,
    Win
};

struct Particle {
    float x=0,y=0,vx=0,vy=0,life=0,maxLife=0,size=1.0f;
    SDL_Color color={255,255,255,255};
};

struct Star {
    float x=0,y=0,size=1.0f,alpha=1.0f,phase=0.0f;
};

struct Petal {
    float x=0,y=0,z=0,speed=0,drift=0,angle=0,spin=0,size=0;
};

struct Platform {
    float x=0,y=0,w=0,h=0;
    bool  isGround=false,isMooncake=false,isBuoy=false,isIcy=false;
    int   mooncakeBites=MOONCAKE_BITES;
    bool  mooncakeActive=true;
    bool  buoyActivated=false,buoyGone=false;
    int   buoyFlyTimer=0;
    float buoyVx=0,buoyVy=0;
    float buoyBobOffset=0;
    bool  isBoat=false;
    float baseX=0,baseY=0;
};

struct WaterZone {
    float x=0,y=0,w=0,h=0;
    bool  isDangerous=false;
};

struct SnowZone {
    float x=0,y=0,w=0,h=0;
    bool  isDamaging=false;
};

struct Envelope {
    float x=0,y=0;
    bool  collected=false;
};

struct SpringNpc {
    float x=0,y=0;
    int   reward=SPRING_NPC_REWARD;
    bool  gifted=false;
    int   animFrame=0;
    int   animTick=0;
    int   dialogueIndex=0;
    int   dialogueTick=0;
    bool  talkActive=false;
};

struct ChildNpc {
    float x=0,y=0;
    int   cost=CHILD_NPC_COST;
    bool  active=true;
    bool  angered=false;
    int   angerTimer=0;
    int   animFrame=0;
    int   animTick=0;
    int   kickFrame=0;
    int   dialogueIndex=0;
    int   dialogueTick=0;
};

struct Stall {
    float x=0,y=0,w=0,h=0;
    int   cost=0;
    std::string name;
    bool  bought=false;
};

enum class BossState { Sleeping, Waking, Attacking, Rage };

struct Boss {
    float x=0,y=0,w=80.0f,h=80.0f,vx=0;
    BossState state=BossState::Sleeping;
    int   stateTimer=BOSS_SLEEP_TICKS,fireTimer=0;
    bool  facingRight=true,active=false;
};

struct Fireball {
    float x=0,y=0,vx=0,vy=0;
    int   life=FIREBALL_LIFETIME;
};

struct SpinyLeaf {
    float x=0,y=0,vy=0.0f,angle=0,spin=0;
};

struct Camel {
    float x=0,y=0,vx=0,waterLevel=0.0f;
    bool  facingRight=true,hasPlayer=false,active=true;
};

struct Oasis {
    float x=0,y=0,w=70.0f,h=16.0f;
    int   waterAmount=OASIS_WATER_AMOUNT,digTimer=0;
    bool  beingDug=false,depleted=false;
};

struct Checkpoint {
    float x=0,y=0;
    bool  reached=false;
};

struct Upgrades {
    int  jumpLevel=0,chargeLevel=0,healthLevel=0,heatLevel=0;
    bool hasShovel=false;

    float effectiveJumpMax() const { return JUMP_FORCE_MAX+jumpLevel*SHOP_JUMP_BONUS; }
    int effectiveMaxCharge() const { return std::max(10,MAX_CHARGE-chargeLevel*SHOP_CHARGE_BONUS); }
    int effectiveMaxHealth() const { return PLAYER_MAX_HEALTH+healthLevel*SHOP_HEALTH_BONUS; }
    float heatMultiplier() const { return 1.0f-heatLevel*SHOP_HEAT_BONUS*0.5f; }
};

struct LevelData {
    std::vector<Platform>   platforms;
    std::vector<Envelope>   envelopes;
    std::vector<Stall>      stalls;
    std::vector<Star>       stars;
    std::vector<ChildNpc>   childNpcs;
    std::vector<WaterZone>  waterZones;
    std::vector<SnowZone>   snowZones;
    std::vector<Camel>      camels;
    std::vector<Oasis>      oases;
    std::vector<Checkpoint> checkpoints;
    std::vector<Fireball>   fireballs;
    std::vector<SpinyLeaf>  spinyLeaves;

    SpringNpc springNpc;
    Boss      boss;
    SDL_FRect gate={0,0,0,0};
    Theme     theme=DAY_THEME;

    float worldW=0,worldH=0,startX=0,startY=0;
    float waterLineY=0;
    bool  isSpring=false,isMarket=false,hasSpringNpc=false,hasBoss=false;
    int   season=SEASON_SPRING;
};

struct Player {
    float x=0,y=0,vx=0,vy=0;
    float width=PLAYER_WIDTH,height=PLAYER_HEIGHT;

    bool  onGround=false,charging=false;
    int   chargeTime=0,chargeType=0;
    Uint64 lastAirJumpTick=0;
    bool  prevSpace=false;

    int   blinkTimer=0;
    bool  blinkRight=false;
    int   jumpAnimTimer=0,chargeAnimFrame=0,chargeAnimTick=0;
    bool  chargeAnimDone=false,prevCharging=false;

    int   health=PLAYER_MAX_HEALTH,maxHealth=PLAYER_MAX_HEALTH;
    int   damageFlashTimer=0;

    float heat=0.0f,waterSubmergedTicks=0.0f;
    float hunger=HUNGER_MAX;
    bool  invincible=false,prevI=false;
    Uint64 lastInvincibleTick=0;

    float thirst=THIRST_MAX,camelOasisWater=0.0f;
    int   jumpsLeft=DESERT_JUMP_MAX,camelIndex=-1;

    int   ridingBuoyIndex=-1;
};

#endif
