#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include "Constants.h"
#include <string>
#include <vector>

// ── Game States ───────────────────────────────────────────────────────────────
enum class GameState {
    Menu,
    Playing,
    MarketPrompt,
    Market,
    Shop,
    Dead,
    SeasonTransition,
    Win
};

// ── Ambient ───────────────────────────────────────────────────────────────────
struct Particle {
    float x=0, y=0, vx=0, vy=0;
    float life=0, maxLife=0, size=1.0f;
    SDL_Color color={255,255,255,255};
};

struct Star {
    float x=0, y=0, size=1.0f, alpha=1.0f, phase=0.0f;
};

struct Petal {
    float x=0, y=0, z=0, speed=0, drift=0;
    float angle=0, spin=0, size=0;
};

// ── World Geometry ────────────────────────────────────────────────────────────
struct Platform {
    float x=0, y=0, w=0, h=0;
    bool  isGround   = false;
    bool  isMooncake = false;  // Autumn: edible platform
    bool  isBuoy     = false;  // Summer: flying buoy
    bool  isIcy      = false;  // Winter: slippery
    int   mooncakeBites = MOONCAKE_BITES;
    bool  mooncakeActive = true;
    // Buoy state
    bool  buoyActivated = false;
    int   buoyFlyTimer  = 0;
    float buoyVx = 0, buoyVy = 0;
};

struct WaterZone {
    float x=0, y=0, w=0, h=0;
    bool  isDangerous = false;  // immediate damage vs slow drown
};

struct SnowZone {
    float x=0, y=0, w=0, h=0;
    bool  isDamaging = false;  // false=slow, true=damage
};

// ── NPCs / Entities ───────────────────────────────────────────────────────────
struct Envelope {
    float x=0, y=0;
    bool  collected = false;
};

struct SpringNpc {
    float x=0, y=0;
    int   reward = SPRING_NPC_REWARD;
    bool  gifted = false;
};

struct ChildNpc {
    float x=0, y=0;
    int   cost    = CHILD_NPC_COST;
    bool  active  = true;      // false once interaction resolved
    bool  angered = false;     // kicked player (no money)
    int   angerTimer = 0;
};

struct Stall {
    float x=0, y=0, w=0, h=0;
    int   cost = 0;
    std::string name;
    bool  bought = false;
};

// ── Boss (Autumn – Lion Dance Creature) ───────────────────────────────────────
enum class BossState { Sleeping, Waking, Attacking, Rage };

struct Boss {
    float x=0, y=0, w=80.0f, h=80.0f;
    float vx = 0;
    BossState state = BossState::Sleeping;
    int   stateTimer  = BOSS_SLEEP_TICKS;
    int   fireTimer   = 0;
    bool  facingRight = true;
    bool  active      = false;
};

struct Fireball {
    float x=0, y=0, vx=0, vy=0;
    int   life = FIREBALL_LIFETIME;
};

struct SpinyLeaf {
    float x=0, y=0, vy=0.0f, angle=0, spin=0;
};

// ── Camel (Desert) ───────────────────────────────────────────────────────────
struct Camel {
    float x=0, y=0, vx=0;
    float waterLevel = 0.0f;   // 0-100
    bool  facingRight = true;
    bool  hasPlayer   = false;
    bool  active      = true;
};

// ── Oasis (Desert) ───────────────────────────────────────────────────────────
struct Oasis {
    float x=0, y=0, w=70.0f, h=16.0f;
    int   waterAmount = OASIS_WATER_AMOUNT;
    int   digTimer    = 0;
    bool  beingDug    = false;
    bool  depleted    = false;
};

// ── Checkpoint ───────────────────────────────────────────────────────────────
struct Checkpoint {
    float x=0, y=0;
    bool  reached = false;
};

// ── Upgrades (persisted) ──────────────────────────────────────────────────────
struct Upgrades {
    int  jumpLevel   = 0;   // 0-SHOP_JUMP_MAX_LEVEL
    int  chargeLevel = 0;   // 0-SHOP_CHARGE_MAX_LEVEL
    int  healthLevel = 0;   // 0-SHOP_HEALTH_MAX_LEVEL
    int  heatLevel   = 0;   // 0-SHOP_HEAT_MAX_LEVEL
    bool hasShovel   = false;

    float effectiveJumpMax() const {
        return JUMP_FORCE_MAX + jumpLevel * SHOP_JUMP_BONUS;
    }
    int effectiveMaxCharge() const {
        return std::max(10, MAX_CHARGE - chargeLevel * SHOP_CHARGE_BONUS);
    }
    int effectiveMaxHealth() const {
        return PLAYER_MAX_HEALTH + healthLevel * SHOP_HEALTH_BONUS;
    }
    // Returns heat-gain multiplier (1.0 = no reduction)
    float heatMultiplier() const {
        return 1.0f - heatLevel * SHOP_HEAT_BONUS * 0.5f;
    }
};

// ── Level Data ────────────────────────────────────────────────────────────────
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
    SDL_FRect gate    = {0,0,0,0};
    Theme     theme   = DAY_THEME;

    float worldW = 0, worldH = 0;
    float startX = 0, startY = 0;

    bool isSpring      = false;
    bool isMarket      = false;
    bool hasSpringNpc  = false;
    bool hasBoss       = false;
    int  season        = SEASON_SPRING;
};

// ── Player ────────────────────────────────────────────────────────────────────
struct Player {
    float x=0, y=0, vx=0, vy=0;
    float width=PLAYER_WIDTH, height=PLAYER_HEIGHT;

    // Ground / jump state
    bool  onGround     = false;
    bool  charging     = false;
    int   chargeTime   = 0;
    int   chargeType   = 0;          // 1=ground, 2=air
    Uint64 lastAirJumpTick = 0;
    bool  prevSpace    = false;

    // Animation
    int   blinkTimer     = 0;
    bool  blinkRight     = false;
    int   jumpAnimTimer  = 0;
    int   chargeAnimFrame= 0;
    int   chargeAnimTick = 0;
    bool  chargeAnimDone = false;
    bool  prevCharging   = false;

    // Vitals
    int   health     = PLAYER_MAX_HEALTH;
    int   maxHealth  = PLAYER_MAX_HEALTH;
    int   damageFlashTimer = 0;

    // Summer
    float heat             = 0.0f;
    float waterSubmergedTicks = 0.0f;  // how long in water this dip

    // Autumn
    float hunger           = HUNGER_MAX;
    bool  invincible       = false;
    Uint64 lastInvincibleTick = 0;
    bool  prevI            = false;    // previous frame I-key state

    // Desert
    float thirst           = THIRST_MAX;
    int   jumpsLeft        = DESERT_JUMP_MAX;
    int   camelIndex       = -1;       // -1 = not riding
    float camelOasisWater  = 0.0f;     // water carried for camel
};

#endif
