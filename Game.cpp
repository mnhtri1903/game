#include "Game.h"
#include "SaveSystem.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ── Static helpers ────────────────────────────────────────────────────────────
float SquareJumpGame::randf(float lo, float hi) {
    return lo + (hi - lo) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
}
float SquareJumpGame::clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}
float SquareJumpGame::lerpf(float a, float b, float t) { return a + (b - a) * t; }
SDL_Color SquareJumpGame::alpha(SDL_Color c, Uint8 a) { c.a = a; return c; }
SDL_Color SquareJumpGame::blend(SDL_Color a, SDL_Color b, float t) {
    t = clampf(t, 0.0f, 1.0f);
    return {
        static_cast<Uint8>(a.r + (b.r - a.r) * t),
        static_cast<Uint8>(a.g + (b.g - a.g) * t),
        static_cast<Uint8>(a.b + (b.b - a.b) * t),
        static_cast<Uint8>(a.a + (b.a - a.a) * t)
    };
}

// ── Window management ─────────────────────────────────────────────────────────
void SquareJumpGame::computeInitialWindowSize(int& w, int& h) const {
    w = DEFAULT_WINDOW_WIDTH; h = DEFAULT_WINDOW_HEIGHT;
    SDL_DisplayID disp = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(disp);
    if (!mode) return;
    w = std::max(MIN_WINDOW_WIDTH,  static_cast<int>(mode->w * 0.82f));
    h = std::max(MIN_WINDOW_HEIGHT, static_cast<int>(mode->h * 0.82f));
    w = std::min(w, mode->w);
    h = std::min(h, mode->h);
}
void SquareJumpGame::refreshWindowMetrics() { SDL_GetWindowSize(window, &screenW, &screenH); }
void SquareJumpGame::toggleFullscreen() {
    fullscreen = !fullscreen;
    SDL_SetWindowFullscreen(window, fullscreen);
    refreshWindowMetrics();
    resetAmbient();
}

// ── Init / Shutdown / Run ─────────────────────────────────────────────────────
bool SquareJumpGame::init() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    if (!SDL_Init(SDL_INIT_VIDEO)) return false;

    int iw = DEFAULT_WINDOW_WIDTH, ih = DEFAULT_WINDOW_HEIGHT;
    computeInitialWindowSize(iw, ih);
    window = SDL_CreateWindow("Square Jump", iw, ih,
                              SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) { SDL_Quit(); return false; }
    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) { SDL_DestroyWindow(window); SDL_Quit(); return false; }

    // Procedural player texture (fallback when sprite sheet unavailable)
    playerTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET,
                                      PLAYER_TEXTURE_SIZE, PLAYER_TEXTURE_SIZE);
    if (!playerTexture) {
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit();
        return false;
    }
    SDL_SetTextureBlendMode(playerTexture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Optional sprite sheet – game continues even if missing
    spriteSheet = IMG_LoadTexture(renderer, "player1.png");
    if (spriteSheet) SDL_SetTextureBlendMode(spriteSheet, SDL_BLENDMODE_BLEND);

    refreshWindowMetrics();
    mouseScreenX = screenW * 0.5f;
    mouseScreenY = screenH * 0.5f;

    hasSave = SaveSystem::exists("savegame.dat");
    resetAmbient();
    return true;
}

void SquareJumpGame::run() {
    while (running) {
        Uint64 t0 = SDL_GetTicks();
        handleEvents();
        update();
        draw();
        Uint64 dt = SDL_GetTicks() - t0;
        if (dt < 16) SDL_Delay(static_cast<Uint32>(16 - dt));
    }
}

void SquareJumpGame::shutdown() {
    if (playerTexture) { SDL_DestroyTexture(playerTexture); playerTexture = nullptr; }
    if (spriteSheet)   { SDL_DestroyTexture(spriteSheet);   spriteSheet   = nullptr; }
    if (renderer)      { SDL_DestroyRenderer(renderer);     renderer      = nullptr; }
    if (window)        { SDL_DestroyWindow(window);         window        = nullptr; }
    SDL_Quit();
}

// ── Save / Load ───────────────────────────────────────────────────────────────
void SquareJumpGame::saveGame() {
    SaveData d;
    d.level        = currentLevel;
    d.lixiCount    = lixiCount;
    d.playerHealth = player.health;
    d.jumpLevel    = upgrades.jumpLevel;
    d.chargeLevel  = upgrades.chargeLevel;
    d.healthLevel  = upgrades.healthLevel;
    d.heatLevel    = upgrades.heatLevel;
    d.hasShovel    = upgrades.hasShovel;
    SaveSystem::save("savegame.dat", d);
    hasSave = true;
}

void SquareJumpGame::loadGame() {
    SaveData d;
    if (!SaveSystem::load("savegame.dat", d)) return;
    lixiCount            = d.lixiCount;
    upgrades.jumpLevel   = d.jumpLevel;
    upgrades.chargeLevel = d.chargeLevel;
    upgrades.healthLevel = d.healthLevel;
    upgrades.heatLevel   = d.heatLevel;
    upgrades.hasShovel   = d.hasShovel;
    startGame(d.level);
    player.health = std::min(d.playerHealth, upgrades.effectiveMaxHealth());
}

// ── Theme / Season helpers ────────────────────────────────────────────────────
int SquareJumpGame::getSeasonFromLevel(int level) const {
    if (level <=  30) return SEASON_SPRING;
    if (level <=  60) return SEASON_SUMMER;
    if (level <=  90) return SEASON_AUTUMN;
    if (level <= 120) return SEASON_WINTER;
    return SEASON_DESERT;
}

Theme SquareJumpGame::themeForLevel(int level) const {
    switch (getSeasonFromLevel(level)) {
        case SEASON_SPRING: return (level <= 10) ? DAY_THEME : (level <= 20) ? NIGHT_THEME : SPRING_THEME;
        case SEASON_SUMMER: return SUMMER_THEME;
        case SEASON_AUTUMN: return AUTUMN_THEME;
        case SEASON_WINTER: return WINTER_THEME;
        case SEASON_DESERT: return DESERT_THEME;
        default:            return LOOP_THEMES[(level - 151) % 3];
    }
}

void SquareJumpGame::resetSeasonStats(int newSeason) {
    if (newSeason == SEASON_SUMMER)  player.heat   = 0.0f;
    if (newSeason == SEASON_AUTUMN)  player.hunger  = HUNGER_MAX;
    if (newSeason == SEASON_WINTER)  { /* nothing extra */ }
    if (newSeason == SEASON_DESERT)  {
        player.thirst    = THIRST_MAX;
        player.jumpsLeft = DESERT_JUMP_MAX;
        player.camelIndex= -1;
    }
}

// ── Flow control ──────────────────────────────────────────────────────────────
void SquareJumpGame::beginNewGame() {
    lixiCount    = DEFAULT_LIXI;
    currentLevel = DEFAULT_START_LEVEL;
    upgrades     = {};
    player       = {};
    startGame(currentLevel);
}

void SquareJumpGame::loadAndContinue() {
    loadGame();
}

void SquareJumpGame::returnToMenu() {
    state = GameState::Menu;
    currentTheme = SPRING_THEME;
    player = {};
    particles.clear();
    camX = camY = 0.0f;
    checkpointActive = false;
}

void SquareJumpGame::openShop() {
    saveGame();
    state = GameState::Shop;
    shopHover = -1;
}
void SquareJumpGame::closeShop() {
    state = GameState::Menu;
}

void SquareJumpGame::startGame(int level) {
    int prevSeason = (currentLevel > 0) ? getSeasonFromLevel(currentLevel) : -1;
    currentLevel = level;
    int curSeason = getSeasonFromLevel(level);

    levelData    = generateLevel(level);
    currentTheme = levelData.theme;

    player.x      = levelData.startX;
    player.y      = levelData.startY;
    player.vx = player.vy = 0.0f;
    player.onGround = false;
    player.charging = false;
    player.chargeTime = 0;
    player.lastAirJumpTick = AIR_JUMP_COOLDOWN;
    player.maxHealth = upgrades.effectiveMaxHealth();
    if (player.health <= 0) player.health = player.maxHealth;

    if (curSeason != prevSeason) resetSeasonStats(curSeason);

    checkpointActive = false;
    particles.clear();
    camX = 0.0f;
    camY = std::max(0.0f, levelData.worldH - static_cast<float>(screenH));
    state = GameState::Playing;

    // Season transition screen when entering a new season
    if (prevSeason >= 0 && curSeason != prevSeason) {
        transitionTimer  = 200;
        transitionSeason = curSeason;
        state = GameState::SeasonTransition;
    }
    saveGame();
}

void SquareJumpGame::startMarket() {
    currentTheme   = SPRING_THEME;
    levelData      = {};
    levelData.theme   = SPRING_THEME;
    levelData.isSpring= true;
    levelData.isMarket= true;
    levelData.season  = SEASON_SPRING;
    float wW = static_cast<float>(screenW) * 4.6f;
    float wH = static_cast<float>(screenH);
    levelData.worldW = wW;
    levelData.worldH = wH;
    levelData.platforms.push_back({0.0f, wH - 44.0f, wW, 44.0f, true});
    levelData.stalls.push_back({600.0f,  wH-112.0f, 120.0f, 68.0f, 2,  "BANH CHUNG", false});
    levelData.stalls.push_back({1180.0f, wH-112.0f, 120.0f, 68.0f, 5,  "AO MOI",     false});
    levelData.stalls.push_back({1760.0f, wH-112.0f, 120.0f, 68.0f, 10, "CAY MAI",    false});
    levelData.gate    = {wW - 170.0f, wH - 124.0f, 56.0f, 76.0f};
    levelData.startX  = 50.0f;
    levelData.startY  = wH - 44.0f - PLAYER_HEIGHT - 10.0f;
    player = {};
    player.x = levelData.startX;
    player.y = levelData.startY;
    player.lastAirJumpTick = AIR_JUMP_COOLDOWN;
    player.maxHealth = upgrades.effectiveMaxHealth();
    particles.clear();
    camX = camY = 0.0f;
    state = GameState::Market;
}

void SquareJumpGame::handleLevelComplete() {
    saveGame();
    if (state == GameState::Market) {
        startGame(31); // After market, go to Summer
    } else if (currentLevel == 150) {
        state = GameState::Win;
    } else if (currentLevel == 30) {
        state = GameState::MarketPrompt;
    } else {
        startGame(currentLevel + 1);
    }
}

void SquareJumpGame::triggerDeath() {
    state      = GameState::Dead;
    deathTimer = 0;
    player.vx  = 0;
    player.vy  = -4.0f; // pop upward on death
    spawnDamageBurst();
}

void SquareJumpGame::respawnAtCheckpoint() {
    if (checkpointActive) {
        player.x  = checkpointX;
        player.y  = checkpointY - PLAYER_HEIGHT;
        player.vx = player.vy = 0.0f;
        player.health = std::max(10, player.health); // survive with 10 hp
    } else {
        player.x = levelData.startX;
        player.y = levelData.startY;
        player.vx = player.vy = 0.0f;
        player.health = std::max(10, player.maxHealth / 2);
    }
}

// ── Player state ──────────────────────────────────────────────────────────────
void SquareJumpGame::takeDamage(int amount) {
    if (player.invincible) return;
    player.health -= amount;
    player.damageFlashTimer = DAMAGE_FLASH_DURATION;
    if (player.health <= 0) {
        player.health = 0;
        triggerDeath();
    }
}

void SquareJumpGame::heal(float amount) {
    player.health = std::min(player.maxHealth,
                             static_cast<int>(player.health + amount));
}

void SquareJumpGame::activateInvincibility() {
    player.invincible         = true;
    player.lastInvincibleTick = ticks;
}

// ── Shop ──────────────────────────────────────────────────────────────────────
void SquareJumpGame::buyUpgrade(int item) {
    switch (item) {
        case 0: // Jump Power
            if (upgrades.jumpLevel < SHOP_JUMP_MAX_LEVEL && lixiCount >= SHOP_JUMP_COST) {
                lixiCount -= SHOP_JUMP_COST;
                upgrades.jumpLevel++;
                spawnPurchaseBurst(screenW * 0.5f, screenH * 0.5f);
            }
            break;
        case 1: // Faster Charge
            if (upgrades.chargeLevel < SHOP_CHARGE_MAX_LEVEL && lixiCount >= SHOP_CHARGE_COST) {
                lixiCount -= SHOP_CHARGE_COST;
                upgrades.chargeLevel++;
                spawnPurchaseBurst(screenW * 0.5f, screenH * 0.5f);
            }
            break;
        case 2: // Max Health
            if (upgrades.healthLevel < SHOP_HEALTH_MAX_LEVEL && lixiCount >= SHOP_HEALTH_COST) {
                lixiCount -= SHOP_HEALTH_COST;
                upgrades.healthLevel++;
                player.maxHealth = upgrades.effectiveMaxHealth();
                heal(static_cast<float>(SHOP_HEALTH_BONUS));
                spawnPurchaseBurst(screenW * 0.5f, screenH * 0.5f);
            }
            break;
        case 3: // Heat Shield
            if (upgrades.heatLevel < SHOP_HEAT_MAX_LEVEL && lixiCount >= SHOP_HEAT_COST) {
                lixiCount -= SHOP_HEAT_COST;
                upgrades.heatLevel++;
                spawnPurchaseBurst(screenW * 0.5f, screenH * 0.5f);
            }
            break;
        case 4: // Full Heal
            if (lixiCount >= SHOP_FULL_HEAL_COST) {
                lixiCount -= SHOP_FULL_HEAL_COST;
                player.health = player.maxHealth;
                spawnPurchaseBurst(screenW * 0.5f, screenH * 0.5f);
            }
            break;
    }
    saveGame();
}

void SquareJumpGame::updateShop(float mx, float my) {
    shopHover = -1;
    float itemH = 64.0f, itemW = 420.0f;
    float startY = screenH * 0.5f - 170.0f;
    float startX = screenW * 0.5f - itemW * 0.5f;
    for (int i = 0; i < 5; i++) {
        SDL_FRect r = {startX, startY + i * (itemH + 12.0f), itemW, itemH};
        if (pointInRect(mx, my, r)) { shopHover = i; break; }
    }
}

// ── Ambient ───────────────────────────────────────────────────────────────────
void SquareJumpGame::resetAmbient() {
    menuStars.clear();
    for (int i = 0; i < MENU_STAR_COUNT; i++)
        menuStars.push_back({randf(0,static_cast<float>(screenW)),
                             randf(0,static_cast<float>(screenH)),
                             randf(1,2.8f), randf(0.25f,0.8f), randf(0,PI*2)});
    petals.clear();
    for (int i = 0; i < SPRING_PETAL_COUNT; i++)
        petals.push_back(createPetal(true));
}

Petal SquareJumpGame::createPetal(bool randomY) const {
    Petal p;
    p.x     = randf(-80.0f, static_cast<float>(screenW)+80);
    p.y     = randomY ? randf(-60.0f, static_cast<float>(screenH)+60) : randf(-160,-10);
    p.z     = randf(0.2f, 1.0f);
    p.speed = randf(0.8f, 2.6f);
    p.drift = randf(-1.2f, 1.2f);
    p.angle = randf(0, 360);
    p.spin  = randf(-2.0f, 2.0f);
    p.size  = randf(2.5f, 6.0f);
    return p;
}

// ── Level generation – Spring ─────────────────────────────────────────────────
LevelData SquareJumpGame::generateLevel(int levelNum) {
    int season = getSeasonFromLevel(levelNum);
    switch (season) {
        case SEASON_SUMMER: return generateSummerLevel(levelNum);
        case SEASON_AUTUMN: return generateAutumnLevel(levelNum);
        case SEASON_WINTER: return generateWinterLevel(levelNum);
        case SEASON_DESERT: return generateDesertLevel(levelNum);
        default:            return generateSpringLevel(levelNum);
    }
}

LevelData SquareJumpGame::generateSpringLevel(int levelNum) {
    LevelData data;
    data.theme    = themeForLevel(levelNum);
    data.isSpring = (levelNum >= 21);
    data.season   = SEASON_SPRING;

    int pfCount = PLATFORM_COUNT_MIN + (std::rand() % (PLATFORM_COUNT_MAX - PLATFORM_COUNT_MIN + 1));
    float avgRise = (PLATFORM_RISE_MIN + PLATFORM_RISE_MAX) * 0.5f;
    data.worldH = static_cast<float>(screenH) + pfCount * avgRise + 280.0f;
    float groundY = data.worldH - 44.0f;
    data.platforms.push_back({0, groundY, std::max(static_cast<float>(screenW)+600,5000.0f), 44.0f, true});

    float curY = groundY - 112.0f;
    float curX = 220.0f;
    float highX = 0.0f;
    for (int i = 0; i < pfCount - 1; i++) {
        float pw = randf(PLATFORM_WIDTH_MIN, PLATFORM_WIDTH_MAX);
        data.platforms.push_back({curX, curY, pw, 22.0f, false});
        highX = curX + pw;
        float rise = randf(PLATFORM_RISE_MIN, PLATFORM_RISE_MAX);
        if (curY - rise < 120.0f) rise = randf(20,38);
        curX += pw + randf(PLATFORM_GAP_MIN, PLATFORM_GAP_MAX);
        curY -= rise;
    }
    float finalW = randf(185, 245);
    data.platforms.push_back({curX, curY, finalW, 22.0f, false});
    highX = curX + finalW;
    data.gate   = {curX + finalW*0.5f - 28, curY - 76, 56, 76};
    data.worldW = highX + 900.0f;
    data.platforms.front().w = data.worldW;
    data.startX = 84.0f;
    data.startY = groundY - PLAYER_HEIGHT - 10.0f;

    // Spring NPCs (money givers)
    if (data.isSpring && data.platforms.size() > 4) {
        size_t idx = 2 + (std::rand() % (data.platforms.size() - 3));
        const Platform& np = data.platforms[idx];
        data.springNpc = {np.x + np.w*0.5f - 14, np.y - 38, SPRING_NPC_REWARD, false};
        data.hasSpringNpc = true;
    }

    // Child NPCs (appear from level 15 onwards in spring)
    if (levelNum >= 15 && data.isSpring && data.platforms.size() > 5) {
        int childCount = (levelNum >= 25) ? 2 : 1;
        for (int c = 0; c < childCount; c++) {
            size_t idx = 3 + (std::rand() % (data.platforms.size() - 4));
            const Platform& cp = data.platforms[idx];
            ChildNpc child;
            child.x = cp.x + cp.w * 0.5f + 20;
            child.y = cp.y - 38;
            data.childNpcs.push_back(child);
        }
    }

    // Stars
    for (int i = 0; i < WORLD_STAR_COUNT; i++)
        data.stars.push_back({randf(0,data.worldW), randf(0,data.worldH),
                              randf(1,3), randf(0.2f,0.8f), randf(0,PI*2)});
    // Checkpoints
    if (data.platforms.size() > 6) {
        size_t mid = data.platforms.size() / 2;
        const Platform& cp = data.platforms[mid];
        data.checkpoints.push_back({cp.x + cp.w*0.5f, cp.y});
    }
    return data;
}

// ── Input ─────────────────────────────────────────────────────────────────────
void SquareJumpGame::handleKeyDown(const SDL_KeyboardEvent& key) {
    if (key.repeat) return;
    if (key.key == SDLK_F11) { toggleFullscreen(); return; }

    if (state == GameState::SeasonTransition) {
        if (transitionTimer > 0) { transitionTimer = 0; }
        // Immediately resume game
        state = GameState::Playing;
        return;
    }
    if (state == GameState::Win) {
        if (key.key == SDLK_ESCAPE || key.key == SDLK_RETURN) returnToMenu();
        return;
    }
    if (state == GameState::Dead) {
        if (key.key == SDLK_R) {
            // Respawn
            player.health = player.maxHealth / 2;
            startGame(currentLevel);
        } else if (key.key == SDLK_ESCAPE) returnToMenu();
        return;
    }
    if (state == GameState::Shop) {
        if (key.key == SDLK_ESCAPE) closeShop();
        if (key.key >= SDLK_1 && key.key <= SDLK_5) buyUpgrade(key.key - SDLK_1);
        return;
    }
    if (state == GameState::Menu) {
        if (key.key == SDLK_RETURN || key.key == SDLK_SPACE) beginNewGame();
        else if (key.key == SDLK_ESCAPE) running = false;
        return;
    }
    if (state == GameState::MarketPrompt) {
        if (key.key == SDLK_Y) startMarket();
        else if (key.key == SDLK_N) startGame(31);
        else if (key.key == SDLK_ESCAPE) returnToMenu();
        return;
    }
    // In-game keys
    if (key.key == SDLK_ESCAPE) returnToMenu();
    if (key.key == SDLK_P) openShop();  // pause / shop
}

void SquareJumpGame::handleMouseClick(float x, float y) {
    if (state == GameState::Menu) {
        if (pointInRect(x,y,playButtonRect()))     beginNewGame();
        else if (hasSave && pointInRect(x,y,continueButtonRect())) loadAndContinue();
        else if (pointInRect(x,y,shopButtonRect())) openShop();
        else if (pointInRect(x,y,exitButtonRect())) running = false;
    } else if (state == GameState::MarketPrompt) {
        if (pointInRect(x,y,promptYesRect())) startMarket();
        else if (pointInRect(x,y,promptNoRect())) startGame(31);
    } else if (state == GameState::Shop) {
        if (shopHover >= 0) buyUpgrade(shopHover);
    } else if (state == GameState::Dead) {
        // "Restart" button handled in drawDead
        SDL_FRect restartBtn = {screenW*0.5f-120,screenH*0.5f+60,240,60};
        if (pointInRect(x,y,restartBtn)) {
            player.health = player.maxHealth / 2;
            startGame(currentLevel);
        }
        SDL_FRect menuBtn = {screenW*0.5f-120,screenH*0.5f+140,240,60};
        if (pointInRect(x,y,menuBtn)) returnToMenu();
    }
}

void SquareJumpGame::handleEvents() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_EVENT_QUIT) { running = false; }
        else if (ev.type == SDL_EVENT_WINDOW_RESIZED) { refreshWindowMetrics(); resetAmbient(); }
        else if (ev.type == SDL_EVENT_MOUSE_MOTION) { mouseScreenX=ev.motion.x; mouseScreenY=ev.motion.y; }
        else if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN && ev.button.button == SDL_BUTTON_LEFT) {
            mouseScreenX = ev.button.x; mouseScreenY = ev.button.y;
            handleMouseClick(mouseScreenX, mouseScreenY);
        }
        else if (ev.type == SDL_EVENT_KEY_DOWN) handleKeyDown(ev.key);
    }
}

void SquareJumpGame::updateSecretShortcut(const bool* keys) {
    bool ctrl  = keys[SDL_SCANCODE_LCTRL]  || keys[SDL_SCANCODE_RCTRL];
    bool shift = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    bool combo = ctrl && shift && keys[SDL_SCANCODE_B] && keys[SDL_SCANCODE_L];
    if (combo && !secretLatched && state == GameState::Playing)
        startGame(currentLevel + 5);
    secretLatched = combo;
}

// ── Ambient update ────────────────────────────────────────────────────────────
void SquareJumpGame::updateAmbient() {
    for (Petal& p : petals) {
        float depth = lerpf(0.4f, 1.35f, p.z);
        p.y += p.speed * depth;
        p.x += std::sin(ticks*0.012f + p.drift*2.6f) * (0.35f+p.z*0.65f) + p.drift*0.2f;
        p.angle += p.spin;
        if (p.y > screenH + 40)  p = createPetal(false);
        if (p.x < -90)  p.x = static_cast<float>(screenW)+90;
        else if (p.x > screenW+90) p.x = -90;
    }
}

void SquareJumpGame::updateParticles() {
    for (int i = static_cast<int>(particles.size())-1; i >= 0; i--) {
        Particle& p = particles[i];
        p.x += p.vx; p.y += p.vy;
        p.vy += 0.06f;  // slight gravity on particles
        p.life -= 1.0f;
        if (p.life <= 0) particles.erase(particles.begin()+i);
    }
}

// ── Particle spawners ─────────────────────────────────────────────────────────
void SquareJumpGame::spawnChargeParticle(const SDL_Color& c) {
    particles.push_back({player.x+player.width*0.5f+randf(-5,5),
                         player.y+player.height,
                         randf(-0.25f,0.25f), randf(0,1),
                         15,15, randf(1,3), c});
}
void SquareJumpGame::spawnJumpBurst(int count, const SDL_Color& c, float sx, float sy) {
    for (int i = 0; i < count; i++)
        particles.push_back({player.x+player.width*0.5f, player.y+player.height,
                             randf(-sx,sx), randf(-sy,sy),
                             randf(16,22), randf(16,22), randf(2,5), c});
}
void SquareJumpGame::spawnPurchaseBurst(float x, float y) {
    for (int i = 0; i < 22; i++)
        particles.push_back({x,y, randf(-4,4), randf(-3.5f,1.5f),
                             randf(18,26), randf(18,26), randf(2,4),
                             (i%2==0) ? currentTheme.playerAccent : currentTheme.petalA});
}
void SquareJumpGame::spawnDamageBurst() {
    for (int i = 0; i < 18; i++)
        particles.push_back({player.x+player.width*0.5f, player.y+player.height*0.5f,
                             randf(-5,5), randf(-5,5),
                             randf(10,20), randf(10,20), randf(2,4),
                             {255, 50, 50, 255}});
}
void SquareJumpGame::spawnSnowParticle() {
    particles.push_back({randf(0,static_cast<float>(screenW)+camX),
                         -10.0f+camY,
                         randf(-2,0.5f), randf(1,3),
                         randf(80,120), randf(80,120), randf(2,5),
                         {220,235,255,200}});
}

// ── Collision resolution ──────────────────────────────────────────────────────
void SquareJumpGame::resolvePlayerCollisions() {
    player.onGround = false;
    for (Platform& pf : levelData.platforms) {
        if (!pf.mooncakeActive && pf.isMooncake) continue; // depleted mooncake
        if (pf.isBuoy && pf.buoyActivated) continue;       // flying buoy: handled separately

        float pcx = player.x+player.width*0.5f,  pcy = player.y+player.height*0.5f;
        float fcx = pf.x+pf.w*0.5f,              fcy = pf.y+pf.h*0.5f;
        float dx = pcx-fcx, dy = pcy-fcy;
        float hwx = (player.width+pf.w)*0.5f, hwy = (player.height+pf.h)*0.5f;
        if (std::fabs(dx)<hwx && std::fabs(dy)<hwy) {
            float ox = hwx-std::fabs(dx), oy = hwy-std::fabs(dy);
            if (ox < oy) {
                player.x += (dx>0 ? ox : -ox);
                player.vx = 0;
            } else {
                if (dy > 0) { player.y += oy; player.vy = 0; }
                else {
                    player.y -= oy; player.vy = 0;
                    player.onGround = true;
                    // Winter: extra slippery
                    if (pf.isIcy && getSeasonFromLevel(currentLevel)==SEASON_WINTER)
                        player.vx *= (WINTER_GROUND_FRICTION / GROUND_FRICTION);
                }
            }
        }
    }
}

void SquareJumpGame::resolvePlayerBuoyCollisions() {
    for (Platform& pf : levelData.platforms) {
        if (!pf.isBuoy || !pf.buoyActivated) continue;
        float pcx = player.x+player.width*0.5f, pcy = player.y+player.height*0.5f;
        float fcx = pf.x+pf.w*0.5f,             fcy = pf.y+pf.h*0.5f;
        float dx = pcx-fcx, dy = pcy-fcy;
        float hwx = (player.width+pf.w)*0.5f, hwy = (player.height+pf.h)*0.5f;
        if (std::fabs(dx)<hwx && std::fabs(dy)<hwy) {
            float ox = hwx-std::fabs(dx), oy = hwy-std::fabs(dy);
            if (ox < oy) {
                player.x += (dx>0 ? ox : -ox); player.vx = 0;
            } else {
                if (dy > 0) { player.y += oy; player.vy = 0; }
                else {
                    // Player rides the buoy
                    player.y -= oy;
                    player.vy = pf.buoyVy;
                    player.vx = pf.buoyVx;
                    player.onGround = true;
                }
            }
        }
    }
}

void SquareJumpGame::resolvePlayerCamelCollisions() {
    if (player.camelIndex >= 0) return; // already riding, handled in updateCamels
    for (int ci = 0; ci < static_cast<int>(levelData.camels.size()); ci++) {
        Camel& camel = levelData.camels[ci];
        if (!camel.active) continue;
        // Simple rectangle for camel's back
        float cx = camel.x, cy = camel.y - 30.0f, cw = 60.0f, ch = 10.0f;
        if (intersects(player.x, player.y, player.width, player.height, cx, cy, cw, ch)) {
            if (player.vy >= 0) {
                // Land on camel's back
                player.y = cy - player.height;
                player.vy = 0;
                player.onGround = true;
                if (camel.waterLevel >= CAMEL_WATER_NEED) {
                    player.camelIndex = ci;
                    camel.hasPlayer  = true;
                }
            }
        }
    }
}

// ── Jump release ──────────────────────────────────────────────────────────────
void SquareJumpGame::releaseChargedJump(bool isDesert) {
    int effCharge = upgrades.effectiveMaxCharge();
    float ratio   = static_cast<float>(player.chargeTime) / static_cast<float>(effCharge);
    float power   = JUMP_FORCE_MIN + ratio * (upgrades.effectiveJumpMax() - JUMP_FORCE_MIN);
    float mwx     = mouseScreenX + camX;
    float mwy     = mouseScreenY + camY;
    float ang     = std::atan2(mwy-(player.y+player.height*0.5f),
                               mwx-(player.x+player.width *0.5f));
    player.vx = std::cos(ang) * power;
    player.vy = std::sin(ang) * power;
    if (player.chargeType==2) player.lastAirJumpTick = ticks;
    player.onGround    = false;
    player.charging    = false;
    player.chargeTime  = 0;
    player.chargeType  = 0;
    player.jumpAnimTimer= 12;
    player.blinkTimer  = 10;
    player.blinkRight  = (std::rand()%2)==0;

    // Autumn: consume hunger
    if (getSeasonFromLevel(currentLevel)==SEASON_AUTUMN) {
        player.hunger = std::max(0.0f, player.hunger - HUNGER_PER_JUMP);
    }
    // Desert: consume jump token
    if (isDesert && player.camelIndex < 0) {
        player.jumpsLeft = std::max(0, player.jumpsLeft - 1);
    }

    SDL_Color burst = (ratio >= 1.0f) ? currentTheme.gate : SDL_Color{255,255,255,255};
    spawnJumpBurst(15, burst, 6.0f, 4.0f);
}

// ── Spring update (reachGate, checkpoints, child NPCs) ────────────────────────
void SquareJumpGame::updatePlaying(const bool* keys) {
    bool spaceDown = keys[SDL_SCANCODE_SPACE];
    int effCharge  = upgrades.effectiveMaxCharge();

    // Ground/air friction
    if (player.onGround) {
        player.vx *= GROUND_FRICTION;
        if (std::fabs(player.vx) < 0.1f) player.vx = 0;
    } else {
        player.vx *= AIR_FRICTION;
    }

    // Charge / jump
    if (spaceDown) {
        if (player.onGround) {
            if (!player.charging) { player.charging=true; player.chargeType=1; player.chargeTime=0; }
        } else if (!player.charging && ticks - player.lastAirJumpTick >= AIR_JUMP_COOLDOWN) {
            player.charging=true; player.chargeType=2; player.chargeTime=0;
        }
        if (player.charging) {
            player.chargeTime = std::min(player.chargeTime+1, effCharge);
            if (ticks%3==0)
                spawnChargeParticle(player.chargeTime>=effCharge ? currentTheme.gate : SDL_Color{255,255,255,255});
        }
    }
    if (!spaceDown && player.charging) releaseChargedJump();

    // Gravity / glide
    if (!player.onGround) {
        if (std::fabs(player.vy) < GLIDE_THRESHOLD) player.vy += GRAVITY * GLIDE_FACTOR;
        else                                         player.vy += GRAVITY;
    }
    if (player.vy > 18) player.vy = 18;

    player.x += player.vx;
    player.y += player.vy;
    player.x  = std::max(0.0f, player.x);
    float maxX = levelData.gate.x + 800.0f;
    if (player.x > maxX) { player.x = maxX; player.vx = 0; }

    resolvePlayerCollisions();

    // Fall off bottom of world → respawn
    if (player.y > levelData.worldH + 80) {
        takeDamage(20);
        respawnAtCheckpoint();
    }

    // Spring NPC (money)
    if (levelData.hasSpringNpc && !levelData.springNpc.gifted) {
        SpringNpc& npc = levelData.springNpc;
        if (intersects(player.x,player.y,player.width,player.height, npc.x,npc.y,28,38)) {
            npc.gifted = true;
            lixiCount += npc.reward;
            spawnPurchaseBurst(npc.x+14, npc.y+12);
        }
    }

    // Child NPCs (lì xì demand)
    for (ChildNpc& child : levelData.childNpcs) {
        if (!child.active) continue;
        if (intersects(player.x,player.y,player.width,player.height, child.x,child.y,28,38)) {
            if (lixiCount >= child.cost) {
                lixiCount -= child.cost;
                child.active = false;
                spawnPurchaseBurst(child.x+14, child.y+12);
            } else if (!child.angered) {
                child.angered = true;
                child.angerTimer = 60;
                // Kick the player
                player.vx = (player.x > child.x) ? CHILD_KICK_VX : -CHILD_KICK_VX;
                player.vy = CHILD_KICK_VY;
                takeDamage(CHILD_KICK_DAMAGE);
            }
        }
    }

    // Checkpoints
    updateCheckpoints();

    player.prevSpace = spaceDown;

    // Invincibility timer
    if (player.invincible && ticks - player.lastInvincibleTick >= INVINCIBLE_DURATION)
        player.invincible = false;
    if (player.damageFlashTimer > 0) player.damageFlashTimer--;
}

void SquareJumpGame::updateCheckpoints() {
    for (Checkpoint& cp : levelData.checkpoints) {
        if (!cp.reached &&
            intersects(player.x,player.y,player.width,player.height,
                       cp.x-8,cp.y-40,16,40)) {
            cp.reached      = true;
            checkpointX     = cp.x;
            checkpointY     = cp.y;
            checkpointActive= true;
            spawnPurchaseBurst(cp.x, cp.y);
        }
    }
}

// ── Market update ─────────────────────────────────────────────────────────────
void SquareJumpGame::updateMarket(const bool* keys) {
    bool spaceDown = keys[SDL_SCANCODE_SPACE];
    bool spaceJust = spaceDown && !player.prevSpace;
    if (player.onGround)  { player.vx *= GROUND_FRICTION; if(std::fabs(player.vx)<0.1f) player.vx=0; }
    else                    player.vx *= AIR_FRICTION;
    if (spaceJust && player.onGround) {
        player.vx=17; player.vy=-3;
        player.jumpAnimTimer=12; player.blinkTimer=10;
        spawnJumpBurst(10,{255,255,255,255},5,2);
    }
    if (!player.onGround) { player.vy += GRAVITY; if(player.vy>10) player.vy=10; }
    player.x+=player.vx; player.y+=player.vy;
    if(player.x<0){player.x=0;player.vx=0;}
    float maxX = levelData.gate.x+800;
    if(player.x>maxX){player.x=maxX;player.vx=0;}
    resolvePlayerCollisions();

    for (Stall& stall : levelData.stalls) {
        if(!stall.bought && intersects(player.x,player.y,player.width,player.height,
                                       stall.x,stall.y,stall.w,stall.h)) {
            if(lixiCount>=stall.cost) {
                lixiCount-=stall.cost; stall.bought=true;
                spawnPurchaseBurst(stall.x+stall.w*0.5f,stall.y);
            }
        }
    }
    player.prevSpace = spaceDown;
}

// ── Player animation ──────────────────────────────────────────────────────────
void SquareJumpGame::updatePlayerAnimation() {
    if (player.blinkTimer>0) player.blinkTimer--;
    if (player.charging) {
        if (!player.prevCharging) {
            player.chargeAnimFrame=0; player.chargeAnimTick=0; player.chargeAnimDone=false;
        }
        if (!player.chargeAnimDone) {
            if (++player.chargeAnimTick >= 3) {
                player.chargeAnimTick=0;
                if (++player.chargeAnimFrame >= 3) { player.chargeAnimFrame=3; player.chargeAnimDone=true; }
            }
        }
    }
    player.prevCharging = player.charging;
}

// ── Camera ─────────────────────────────────────────────────────────────────────
void SquareJumpGame::updateCamera() {
    float tx = player.x - screenW*0.4f + player.width*0.5f;
    tx = clampf(tx, 0, std::max(0.0f, levelData.worldW-screenW));
    camX += (tx-camX)*0.1f;
    float ty = player.y - screenH*0.62f + player.height*0.5f;
    ty = clampf(ty, 0, std::max(0.0f, levelData.worldH-screenH));
    camY += (ty-camY)*0.1f;
}

bool SquareJumpGame::reachedGate() const {
    return intersects(player.x,player.y,player.width,player.height,
                      levelData.gate.x,levelData.gate.y,levelData.gate.w,levelData.gate.h);
}

// ── Master update ─────────────────────────────────────────────────────────────
void SquareJumpGame::update() {
    ticks++;
    updateAmbient();
    const bool* keys = SDL_GetKeyboardState(nullptr);
    updateSecretShortcut(keys);

    // States that don't advance simulation
    if (state == GameState::Menu || state == GameState::MarketPrompt ||
        state == GameState::Win) {
        updateParticles();
        return;
    }
    if (state == GameState::Shop) {
        updateShop(mouseScreenX, mouseScreenY);
        updateParticles();
        return;
    }
    if (state == GameState::Dead) {
        deathTimer++;
        updateParticles();
        return;
    }
    if (state == GameState::SeasonTransition) {
        transitionTimer--;
        if (transitionTimer <= 0) state = GameState::Playing;
        return;
    }

    updatePlayerAnimation();

    if (state == GameState::Market) {
        updateMarket(keys);
    } else {
        int season = getSeasonFromLevel(currentLevel);
        switch (season) {
            case SEASON_SPRING: updatePlaying(keys); break;
            case SEASON_SUMMER: updateSummer(keys);  break;
            case SEASON_AUTUMN: updateAutumn(keys);  break;
            case SEASON_WINTER: updateWinter(keys);  break;
            case SEASON_DESERT: updateDesert(keys);  break;
            default:            updatePlaying(keys); break;
        }
    }

    updateCamera();
    updateParticles();

    if (state == GameState::Playing && reachedGate())
        handleLevelComplete();
    if (state == GameState::Market && reachedGate())
        handleLevelComplete();
}

// ── Button rects ──────────────────────────────────────────────────────────────
SDL_FRect SquareJumpGame::playButtonRect()     const { return {screenW*0.5f-130, screenH*0.5f+72,  260,60}; }
SDL_FRect SquareJumpGame::continueButtonRect() const { return {screenW*0.5f-130, screenH*0.5f+142, 260,60}; }
SDL_FRect SquareJumpGame::shopButtonRect()     const { return {screenW*0.5f-130, screenH*0.5f+212, 260,60}; }
SDL_FRect SquareJumpGame::exitButtonRect()     const { return {screenW*0.5f-130, screenH*0.5f+282, 260,60}; }
SDL_FRect SquareJumpGame::promptYesRect()      const { return {screenW*0.5f-170, screenH*0.5f+55,  140,60}; }
SDL_FRect SquareJumpGame::promptNoRect()       const { return {screenW*0.5f+30,  screenH*0.5f+55,  140,60}; }

bool SquareJumpGame::pointInRect(float x,float y,const SDL_FRect& r) const {
    return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h;
}
bool SquareJumpGame::intersects(float ax,float ay,float aw,float ah,
                                 float bx,float by,float bw,float bh) const {
    return ax<bx+bw && ax+aw>bx && ay<by+bh && ay+ah>by;
}
