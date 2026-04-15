#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL3/SDL.h>

// ── Window ────────────────────────────────────────────────────────────────────
constexpr int   DEFAULT_WINDOW_WIDTH  = 1366;
constexpr int   DEFAULT_WINDOW_HEIGHT = 768;
constexpr int   MIN_WINDOW_WIDTH      = 960;
constexpr int   MIN_WINDOW_HEIGHT     = 540;

// ── Player ────────────────────────────────────────────────────────────────────
constexpr float PLAYER_WIDTH          = 28.0f;
constexpr float PLAYER_HEIGHT         = 28.0f;
constexpr int   PLAYER_TEXTURE_SIZE   = 36;
constexpr int   PLAYER_MAX_HEALTH     = 100;
constexpr int   DAMAGE_FLASH_DURATION = 30;

// ── Physics ───────────────────────────────────────────────────────────────────
constexpr float GRAVITY               = 0.65f;
constexpr float AIR_FRICTION          = 0.96f;
constexpr float GROUND_FRICTION       = 0.40f;
constexpr float GLIDE_FACTOR          = 0.75f;
constexpr float GLIDE_THRESHOLD       = 2.0f;

// ── Jump / Charge ─────────────────────────────────────────────────────────────
constexpr int   MAX_CHARGE            = 40;
constexpr float JUMP_FORCE_MIN        = 6.0f;
constexpr float JUMP_FORCE_MAX        = 22.0f;
constexpr Uint64 AIR_JUMP_COOLDOWN    = 180;

// ── Math / Misc ───────────────────────────────────────────────────────────────
constexpr float PI                    = 3.1415926535f;
constexpr float DEBUG_TEXT_CHAR_W     = 8.0f;
constexpr float DEBUG_TEXT_CHAR_H     = 8.0f;

// ── Ambient ───────────────────────────────────────────────────────────────────
constexpr int   WORLD_STAR_COUNT      = 250;
constexpr int   MENU_STAR_COUNT       = 180;
constexpr int   SPRING_PETAL_COUNT    = 220;

// ── Level generation defaults ─────────────────────────────────────────────────
constexpr int   DEFAULT_START_LEVEL   = 1;
constexpr int   DEFAULT_LIXI          = 0;
constexpr int   PLATFORM_COUNT_MIN    = 10;
constexpr int   PLATFORM_COUNT_MAX    = 15;
constexpr float PLATFORM_WIDTH_MIN    = 130.0f;
constexpr float PLATFORM_WIDTH_MAX    = 190.0f;
constexpr float PLATFORM_GAP_MIN      = 105.0f;
constexpr float PLATFORM_GAP_MAX      = 150.0f;
constexpr float PLATFORM_RISE_MIN     = 44.0f;
constexpr float PLATFORM_RISE_MAX     = 66.0f;

// ── Season identifiers ────────────────────────────────────────────────────────
constexpr int SEASON_SPRING           = 0;  // levels  1-30
constexpr int SEASON_SUMMER           = 1;  // levels 31-60
constexpr int SEASON_AUTUMN           = 2;  // levels 61-90
constexpr int SEASON_WINTER           = 3;  // levels 91-120
constexpr int SEASON_DESERT           = 4;  // levels 121-150

// ── Season 1: Spring / Lunar New Year ─────────────────────────────────────────
constexpr int   SPRING_NPC_REWARD     = 5;
constexpr int   CHILD_NPC_COST        = 3;
constexpr float CHILD_KICK_VX         = 10.0f;
constexpr float CHILD_KICK_VY         = -8.0f;
constexpr int   CHILD_KICK_DAMAGE     = 15;

// ── Season 2: Summer / Beach ──────────────────────────────────────────────────
constexpr float HEAT_MAX              = 100.0f;
constexpr float HEAT_RATE_SUN         = 0.07f;   // per tick in direct sun
constexpr float HEAT_COOL_WATER       = 0.55f;   // per tick in water
constexpr float HEAT_COOL_AIR         = 0.012f;  // passive cooling in shade
constexpr float HEAT_DAMAGE_THRESHOLD = 100.0f;
constexpr float HEAT_OVERHEAT_RATE    = 0.4f;    // HP per tick when overheated
constexpr int   BUOY_FLY_DURATION     = 65;      // ticks the buoy flies
constexpr float BUOY_FLY_SPEED_X     = 7.5f;
constexpr float BUOY_FLY_SPEED_Y     = -2.5f;
constexpr float WATER_SUBMERSION_MAX  = 150.0f;  // ticks before drown damage
constexpr float WATER_DROWN_RATE      = 0.6f;    // HP per tick while drowning

// ── Season 3: Autumn / Mid-Autumn + Boss ──────────────────────────────────────
constexpr float HUNGER_MAX            = 100.0f;
constexpr float HUNGER_PER_JUMP       = 2.5f;
constexpr float HUNGER_DAMAGE_RATE    = 0.05f;   // HP per tick when starving
constexpr int   MOONCAKE_BITES        = 3;
constexpr float MOONCAKE_HEAL         = 20.0f;
constexpr int   BOSS_SLEEP_TICKS      = 320;
constexpr int   BOSS_WAKE_TICKS       = 220;
constexpr int   BOSS_FIRE_TICKS       = 180;
constexpr float BOSS_SPEED            = 2.2f;
constexpr float BOSS_RAGE_SPEED       = 4.2f;
constexpr float FIREBALL_SPEED        = 5.5f;
constexpr int   FIREBALL_LIFETIME     = 130;
constexpr int   FIREBALL_INTERVAL     = 40;      // ticks between fireballs
constexpr int   FIREBALL_DAMAGE       = 20;
constexpr int   SPINY_LEAF_DAMAGE     = 15;
constexpr int   SPINY_SPAWN_CHANCE    = 120;     // 1-in-N chance per tick
constexpr int   INVINCIBLE_DURATION   = 120;     // 2 seconds at 60 fps
constexpr Uint64 INVINCIBLE_COOLDOWN  = 600;     // 10 seconds at 60 fps

// ── Season 4: Winter / Blizzard ───────────────────────────────────────────────
constexpr float WINTER_SLOW_FACTOR    = 0.82f;  // velocity multiplier in slow snow
constexpr float WINTER_DAMAGE_RATE    = 0.8f;   // HP per tick in harmful zone
constexpr float WINTER_GROUND_FRICTION= 0.60f;  // less grip (was GROUND_FRICTION 0.40)
constexpr float WINTER_VISIBILITY     = 0.55f;  // overlay alpha (0-1)
constexpr int   SNOW_PARTICLE_RATE    = 3;      // spawn every N ticks

// ── Season 5: Desert ──────────────────────────────────────────────────────────
constexpr float THIRST_MAX            = 100.0f;
constexpr float THIRST_PER_TICK       = 0.025f;
constexpr float THIRST_DANGER         = 20.0f;
constexpr float THIRST_DAMAGE_RATE    = 0.3f;   // HP per tick below danger
constexpr int   DESERT_JUMP_MAX       = 5;
constexpr float CAMEL_SPEED           = 1.6f;
constexpr float CAMEL_THIRST_RESTORE  = 45.0f;
constexpr float CAMEL_WATER_NEED      = 30.0f;  // water level for camel to be rideable
constexpr int   OASIS_DIG_TIME        = 90;     // ticks to finish digging
constexpr int   OASIS_WATER_AMOUNT    = 60;     // water units per dig

// ── Shop ──────────────────────────────────────────────────────────────────────
constexpr int   SHOP_JUMP_COST        = 10;
constexpr float SHOP_JUMP_BONUS       = 2.5f;
constexpr int   SHOP_JUMP_MAX_LEVEL   = 3;
constexpr int   SHOP_CHARGE_COST      = 8;
constexpr int   SHOP_CHARGE_BONUS     = 5;      // reduces effective MAX_CHARGE
constexpr int   SHOP_CHARGE_MAX_LEVEL = 3;
constexpr int   SHOP_HEALTH_COST      = 15;
constexpr int   SHOP_HEALTH_BONUS     = 25;
constexpr int   SHOP_HEALTH_MAX_LEVEL = 3;
constexpr int   SHOP_HEAT_COST        = 12;
constexpr float SHOP_HEAT_BONUS       = 0.5f;   // multiplier on heat gain
constexpr int   SHOP_HEAT_MAX_LEVEL   = 2;
constexpr int   SHOP_FULL_HEAL_COST   = 20;

// ── Themes ────────────────────────────────────────────────────────────────────
struct Theme {
    SDL_Color bg1;
    SDL_Color bg2;
    SDL_Color playerPrimary;
    SDL_Color playerAccent;
    SDL_Color gate;
    SDL_Color platformBody;
    SDL_Color platformTop;
    SDL_Color platformShadow;
    SDL_Color petalA;
    SDL_Color petalB;
};

inline const Theme DAY_THEME = {
    {135, 206, 235, 255}, {224, 246, 255, 255},
    {211, 47,  47,  255}, {255, 179,  0, 255},
    {255, 214,  0,  255}, { 74,  85, 104, 255},
    {160, 174, 192, 255}, { 45,  55,  72, 255},
    {255, 235,  59, 255}, {255, 245, 157, 255}
};

inline const Theme NIGHT_THEME = {
    { 10,  10,  26, 255}, { 26,  26,  58, 255},
    {211,  47,  47, 255}, {142,  45, 226, 255},
    {  0, 229, 255, 255}, { 74,  85, 104, 255},
    {160, 174, 192, 255}, { 26,  32,  44, 255},
    {234, 128, 252, 255}, {128, 222, 234, 255}
};

inline const Theme SPRING_THEME = {
    {232, 245, 233, 255}, {200, 230, 201, 255},
    {211,  47,  47, 255}, {255, 235,  59, 255},
    {244,  67,  54, 255}, { 46, 125,  50, 255},
    {241, 248, 233, 255}, { 27,  94,  32, 255},
    {255, 235,  59, 255}, {255, 241, 118, 255}
};

inline const Theme SUMMER_THEME = {
    { 64, 164, 223, 255}, {180, 235, 255, 255},
    {211,  47,  47, 255}, {255, 179,   0, 255},
    {  0, 188, 212, 255}, { 33, 150, 243, 255},
    {144, 202, 249, 255}, { 13,  71, 161, 255},
    {255, 214,  60, 255}, {255, 241, 118, 255}
};

inline const Theme AUTUMN_THEME = {
    { 78,  52,  46, 255}, { 30,  20,  10, 255},
    {211,  47,  47, 255}, {255, 152,   0, 255},
    {255, 193,   7, 255}, {121,  85,  72, 255},
    {188, 143, 143, 255}, { 62,  39,  35, 255},
    {255,  87,  34, 255}, {255, 167,  38, 255}
};

inline const Theme WINTER_THEME = {
    {176, 196, 222, 255}, {240, 248, 255, 255},
    {211,  47,  47, 255}, {135, 206, 235, 255},
    {200, 230, 255, 255}, {150, 170, 200, 255},
    {220, 235, 255, 255}, {100, 130, 160, 255},
    {200, 220, 255, 255}, {255, 255, 255, 255}
};

inline const Theme DESERT_THEME = {
    {230, 190,  90, 255}, {200, 150,  50, 255},
    {211,  47,  47, 255}, {255, 152,   0, 255},
    {255, 214,   0, 255}, {180, 140,  80, 255},
    {220, 180, 120, 255}, {140, 100,  40, 255},
    {255, 200, 100, 255}, {255, 220, 150, 255}
};

inline const Theme LOOP_THEMES[3] = {
    {
        {  1,  4,  9, 255}, { 13, 17, 23, 255},
        {211, 47, 47, 255}, {  0,212,255, 255},
        {255,214,  0, 255}, { 74, 85,104, 255},
        {160,174,192, 255}, { 45, 55, 72, 255},
        {  0,212,255, 255}, {255,235, 59, 255}
    },
    {
        { 10,  1,  1, 255}, { 26,  8,  8, 255},
        {211, 47, 47, 255}, {255, 23, 68, 255},
        {  0,229,255, 255}, { 74, 85,104, 255},
        {160,174,192, 255}, { 45, 55, 72, 255},
        {255, 23, 68, 255}, {255,128,171, 255}
    },
    {
        {  1,  4, 10, 255}, {  8, 12, 26, 255},
        {211, 47, 47, 255}, {  0,230,118, 255},
        {234,128,252, 255}, { 74, 85,104, 255},
        {160,174,192, 255}, { 45, 55, 72, 255},
        {  0,230,118, 255}, {234,128,252, 255}
    }
};

#endif
