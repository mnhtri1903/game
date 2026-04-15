#ifndef GAME_H
#define GAME_H

#include "SDL3/SDL.h"
#include <string>
#include <vector>
#include "Constants.h"
#include "GameObjects.h"

class SquareJumpGame {
public:
    bool init();
    void run();
    void shutdown();

private:
    // ── SDL handles ──────────────────────────────────────────────────────────
    SDL_Window*   window       = nullptr;
    SDL_Renderer* renderer     = nullptr;
    SDL_Texture*  playerTexture= nullptr;  // fallback procedural texture
    SDL_Texture*  spriteSheet  = nullptr;  // player1.png sprite sheet

    // ── Core loop state ───────────────────────────────────────────────────────
    bool     running    = true;
    bool     fullscreen = false;
    int      screenW    = DEFAULT_WINDOW_WIDTH;
    int      screenH    = DEFAULT_WINDOW_HEIGHT;
    Uint64   ticks      = 0;
    GameState state     = GameState::Menu;

    // ── Camera ───────────────────────────────────────────────────────────────
    float camX = 0.0f;
    float camY = 0.0f;

    // ── Mouse ─────────────────────────────────────────────────────────────────
    float mouseScreenX = 0.0f;
    float mouseScreenY = 0.0f;

    // ── Progression ───────────────────────────────────────────────────────────
    int   currentLevel = DEFAULT_START_LEVEL;
    int   lixiCount    = DEFAULT_LIXI;
    bool  secretLatched= false;
    bool  hasSave      = false;

    // ── World / entities ──────────────────────────────────────────────────────
    Theme      currentTheme = SPRING_THEME;
    LevelData  levelData;
    Player     player;
    Upgrades   upgrades;

    std::vector<Particle> particles;
    std::vector<Petal>    petals;
    std::vector<Star>     menuStars;

    // ── Checkpoint state ──────────────────────────────────────────────────────
    float checkpointX     = 0.0f;
    float checkpointY     = 0.0f;
    bool  checkpointActive= false;

    // ── Death / transition ────────────────────────────────────────────────────
    int   deathTimer      = 0;
    int   transitionTimer = 0;
    int   transitionSeason= 0;  // season being entered

    // ── Shop UI ───────────────────────────────────────────────────────────────
    int   shopHover = -1;

    // ── Shop item names / text ────────────────────────────────────────────────
    // (built lazily in drawShop)

    // ─────────────────────────────────────────────────────────────────────────
    //  PRIVATE HELPERS
    // ─────────────────────────────────────────────────────────────────────────

    // Math helpers
    static float randf(float lo, float hi);
    static float clampf(float v, float lo, float hi);
    static float lerpf(float a, float b, float t);
    static SDL_Color alpha(SDL_Color c, Uint8 a);
    static SDL_Color blend(SDL_Color a, SDL_Color b, float t);

    // Window
    void computeInitialWindowSize(int& w, int& h) const;
    void refreshWindowMetrics();
    void toggleFullscreen();

    // Flow control
    void beginNewGame();
    void loadAndContinue();
    void returnToMenu();
    void startGame(int level);
    void startMarket();
    void handleLevelComplete();
    void triggerDeath();
    void respawnAtCheckpoint();
    void openShop();
    void closeShop();

    // Player state
    void takeDamage(int amount);
    void heal(float amount);
    void activateInvincibility();

    // Shop
    void buyUpgrade(int item);

    // Progression
    int  getSeasonFromLevel(int level) const;
    Theme themeForLevel(int level) const;
    void resetSeasonStats(int newSeason);

    // Save / Load
    void saveGame();
    void loadGame();

    // Level generation
    LevelData generateLevel(int levelNum);
    LevelData generateSpringLevel(int levelNum);
    LevelData generateSummerLevel(int levelNum);
    LevelData generateAutumnLevel(int levelNum);
    LevelData generateWinterLevel(int levelNum);
    LevelData generateDesertLevel(int levelNum);

    // Ambient
    void resetAmbient();
    Petal createPetal(bool randomY) const;

    // Input
    void handleEvents();
    void handleMouseClick(float x, float y);
    void handleKeyDown(const SDL_KeyboardEvent& key);
    void updateSecretShortcut(const bool* keys);

    // Core update
    void update();
    void updateAmbient();
    void updateParticles();
    void updatePlayerAnimation();
    void updateCamera();

    // Season updates
    void updatePlaying(const bool* keys);   // Spring
    void updateSummer(const bool* keys);
    void updateAutumn(const bool* keys);
    void updateWinter(const bool* keys);
    void updateDesert(const bool* keys);
    void updateMarket(const bool* keys);
    void updateShop(float mx, float my);

    // Sub-system updates (called by season handlers)
    void releaseChargedJump(bool isDesert = false);
    void resolvePlayerCollisions();
    void resolvePlayerBuoyCollisions();
    void resolvePlayerCamelCollisions();
    void updateBoss();
    void updateFireballs();
    void updateSpinyLeaves();
    void updateCamels();
    void updateBuoys();
    void updateCheckpoints();
    bool reachedGate() const;

    // Particle spawners
    void spawnChargeParticle(const SDL_Color& c);
    void spawnJumpBurst(int count, const SDL_Color& c, float sx, float sy);
    void spawnPurchaseBurst(float x, float y);
    void spawnDamageBurst();
    void spawnSnowParticle();

    // Button rects
    SDL_FRect playButtonRect()     const;
    SDL_FRect continueButtonRect() const;
    SDL_FRect shopButtonRect()     const;
    SDL_FRect exitButtonRect()     const;
    SDL_FRect promptYesRect()      const;
    SDL_FRect promptNoRect()       const;

    bool pointInRect(float x, float y, const SDL_FRect& r) const;
    bool intersects(float ax,float ay,float aw,float ah,
                    float bx,float by,float bw,float bh) const;

    // ── Render (implemented across GameRender.cpp & SeasonRender.cpp) ─────────

    void draw();

    // Primitives
    void setColor(const SDL_Color& c);
    void fillRect(float x,float y,float w,float h,const SDL_Color& c);
    void fillCircle(float cx,float cy,float r,const SDL_Color& c);
    void fillRoundedRect(float x,float y,float w,float h,float r,const SDL_Color& c);
    void drawGlowRect(float x,float y,float w,float h,const SDL_Color& c,int layers,float spread);
    void drawThickLine(float x1,float y1,float x2,float y2,float th,const SDL_Color& c);
    void drawGradientVertical(const SDL_Color& top,const SDL_Color& bot);
    void drawText(float x,float y,const std::string& s,const SDL_Color& c,float scale=1.0f);
    float textWidth(const std::string& s, float scale=1.0f) const;
    void drawCenteredText(float cx,float y,const std::string& s,const SDL_Color& c,float scale=1.0f);

    // Backgrounds
    void drawParallaxStars(const std::vector<Star>& stars, float wW, float wH);
    void drawMenuBackground();
    void drawWorldBackground();
    void drawSpringBackdrop();
    void drawSummerBackground();
    void drawAutumnBackground();
    void drawWinterBackground();
    void drawDesertBackground();
    void drawWinterEffect();        // overlay snow-storm effect
    void drawDesertHeat();          // overlay heat shimmer

    // World elements
    void drawTree(float x,float y,float len,float angleDeg,float branchW,int depth);
    void drawPetal(const Petal& p, bool screenSpace);
    void drawMetalPlatform(const Platform& pf, float x,float y,float w,float h);
    void drawSpringPlatform(float x,float y,float w,float h,bool isGround);
    void drawBuoyPlatform(const Platform& pf, float sx, float sy);
    void drawMooncakePlatform(const Platform& pf, float sx, float sy);
    void drawPlatforms();
    void drawWaterZones();
    void drawSnowZones();

    // Entities
    void drawSpringNpc();
    void drawChildNpcs();
    void drawStalls();
    void drawBoss();
    void drawFireballs();
    void drawSpinyLeaves();
    void drawCamels();
    void drawOases();
    void drawCheckpoints();
    void drawParticles();
    void drawGate();

    // Player
    void renderPlayerTexture();  // procedural fallback
    void drawAimDots();
    void drawPlayer();

    // HUD
    void drawHud();
    void drawHealthBar();
    void drawStatBars();          // heat / hunger / thirst / jumps (all seasons)

    // Menus / screens
    void drawMenu();
    void drawButton(const SDL_FRect& r,const std::string& label,bool hover,const SDL_Color& fill);
    void drawMarketPrompt();
    void drawShop();
    void drawDead();
    void drawWin();
    void drawSeasonTransition();
    void drawWorld(bool showPlayer);
};

#endif
