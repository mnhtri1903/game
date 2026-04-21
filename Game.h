#ifndef GAME_H
#define GAME_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
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
    SDL_Window*   window        = nullptr;
    SDL_Renderer* renderer      = nullptr;
    SDL_Texture*  playerTexture = nullptr;
    SDL_Texture*  spriteSheet   = nullptr;

    TTF_Font* fontS  = nullptr;
    TTF_Font* fontM  = nullptr;
    TTF_Font* fontL  = nullptr;
    TTF_Font* fontXL = nullptr;

    bool      running    = true;
    bool      fullscreen = false;
    int       screenW    = DEFAULT_WINDOW_WIDTH;
    int       screenH    = DEFAULT_WINDOW_HEIGHT;
    Uint64    ticks      = 0;
    GameState state      = GameState::Menu;

    float camX=0.0f,camY=0.0f;
    float mouseScreenX=0.0f,mouseScreenY=0.0f;

    int   currentLevel  = DEFAULT_START_LEVEL;
    int   lixiCount     = DEFAULT_LIXI;
    bool  secretLatched = false;
    bool  hasSave       = false;

    Theme     currentTheme = SPRING_THEME;
    LevelData levelData;
    Player    player;
    Upgrades  upgrades;

    std::vector<Particle> particles;
    std::vector<Petal>    petals;
    std::vector<Star>     menuStars;

    float checkpointX=0.0f,checkpointY=0.0f;
    bool  checkpointActive=false;

    int   deathTimer=0,transitionTimer=0,transitionSeason=0;
    int   shopHover=-1;
    int   levelSelectScroll=0;
    int   summerWaveKickCooldown=0;

    static float randf(float lo, float hi);
    static float clampf(float v, float lo, float hi);
    static float lerpf(float a, float b, float t);
    static SDL_Color alpha(SDL_Color c, Uint8 a);
    static SDL_Color blend(SDL_Color a, SDL_Color b, float t);
    static std::string findSystemFont();

    void computeInitialWindowSize(int& w, int& h) const;
    void refreshWindowMetrics();
    void toggleFullscreen();

    TTF_Font* pickFont(float scale) const;

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
    void openPauseMenu();
    void closePauseMenu();
    void openLevelSelect();

    void takeDamage(int amount);
    void heal(float amount);
    void activateInvincibility();
    void buyUpgrade(int item);

    int   getSeasonFromLevel(int level) const;
    Theme themeForLevel(int level) const;
    void  resetSeasonStats(int newSeason);

    void saveGame();
    void loadGame();

    float getSummerShoreX() const;
    float getSummerWaveAmplitude(float worldX) const;
    float getSummerWaterSurfaceY(float worldX) const;
    float getSummerWavePush(float worldX) const;

    LevelData generateLevel(int levelNum);
    LevelData generateDayLevel(int levelNum);
    LevelData generateNightLevel(int levelNum);
    LevelData generateSpringLevel(int levelNum);
    LevelData generateSummerLevel(int levelNum);
    LevelData generateAutumnLevel(int levelNum);
    LevelData generateWinterLevel(int levelNum);
    LevelData generateDesertLevel(int levelNum);

    void  resetAmbient();
    Petal createPetal(bool randomY) const;

    void handleEvents();
    void handleMouseClick(float x, float y);
    void handleKeyDown(const SDL_KeyboardEvent& key);
    void updateSecretShortcut(const bool* keys);

    void update();
    void updateAmbient();
    void updateParticles();
    void updatePlayerAnimation();
    void updateCamera();
    void updateSpringNpcAnim();
    void updateChildNpcAnim();

    void updatePlaying(const bool* keys);
    void updateDay(const bool* keys);
    void updateNight(const bool* keys);
    void updateSummer(const bool* keys);
    void updateAutumn(const bool* keys);
    void updateWinter(const bool* keys);
    void updateDesert(const bool* keys);
    void updateMarket(const bool* keys);
    void updateShop(float mx, float my);

    void updateFans();
    void updateFakePlatforms();
    void updateSpikes();
    void updateTsunamiWaves();

    void releaseChargedJump(bool isDesert=false);
    void resolvePlayerCollisions();
    void resolvePlayerBuoyCollisions();
    void resolvePlayerCamelCollisions();
    void updateBoss();
    void updateFireballs();
    void updateSpinyLeaves();
    void updateCamels();
    void updateBuoys(bool qPressed);
    void updateCheckpoints();
    bool reachedGate() const;

    void spawnChargeParticle(const SDL_Color& c);
    void spawnJumpBurst(int count, const SDL_Color& c, float sx, float sy);
    void spawnPurchaseBurst(float x, float y);
    void spawnDamageBurst();
    void spawnSnowParticle();
    void spawnWindParticle(float x, float y, float vx, const SDL_Color& c);

    SDL_FRect menuPrimaryBtnRect()  const;
    SDL_FRect menuNewGameBtnRect()  const;
    SDL_FRect menuShopBtnRect()     const;
    SDL_FRect menuLevelBtnRect()    const;
    SDL_FRect menuExitBtnRect()     const;
    SDL_FRect pauseResumeBtnRect()  const;
    SDL_FRect pauseRestartBtnRect() const;
    SDL_FRect pauseExitBtnRect()    const;
    SDL_FRect promptYesRect()       const;
    SDL_FRect promptNoRect()        const;

    bool pointInRect(float x, float y, const SDL_FRect& r) const;
    bool intersects(float ax,float ay,float aw,float ah,
                    float bx,float by,float bw,float bh) const;

    void draw();

    void setColor(const SDL_Color& c);
    void fillRect(float x,float y,float w,float h,const SDL_Color& c);
    void fillCircle(float cx,float cy,float r,const SDL_Color& c);
    void fillEllipse(float cx,float cy,float rx,float ry,const SDL_Color& c);
    void fillRoundedRect(float x,float y,float w,float h,float r,const SDL_Color& c);
    void drawGlowRect(float x,float y,float w,float h,const SDL_Color& c,int layers,float spread);
    void drawThickLine(float x1,float y1,float x2,float y2,float th,const SDL_Color& c);
    void drawGradientVertical(const SDL_Color& top,const SDL_Color& bot);
    void drawText(float x,float y,const std::string& s,const SDL_Color& c,float scale=1.0f);
    float textWidth(const std::string& s,float scale=1.0f) const;
    void drawCenteredText(float cx,float y,const std::string& s,const SDL_Color& c,float scale=1.0f);
    void drawDialogueBubble(float x,float y,const std::string& text,const SDL_Color& bg);

    void drawParallaxStars(const std::vector<Star>& stars,float wW,float wH);
    void drawMenuBackground();
    void drawWorldBackground();
    void drawDayBackground();
    void drawNightBackground();
    void drawSpringBackdrop();
    void drawSummerBackground();
    void drawAutumnBackground();
    void drawWinterBackground();
    void drawDesertBackground();
    void drawWinterEffect();
    void drawDesertHeat();

    void drawTree(float x,float y,float len,float angleDeg,float branchW,int depth);
    void drawPetal(const Petal& p,bool screenSpace);
    void drawMetalPlatform(const Platform& pf,float x,float y,float w,float h);
    void drawSpringPlatform(float x,float y,float w,float h,bool isGround);
    void drawBuoyPlatform(const Platform& pf,float sx,float sy);
    void drawBoatPlatform(const Platform& pf,float sx,float sy);
    void drawMooncakePlatform(const Platform& pf,float sx,float sy);
    void drawSandGround(float sx,float sy,float w,float h);
    void drawPlatforms();
    void drawWaterZones();
    void drawSnowZones();

    void drawFans();
    void drawSpikes();

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

    void renderPlayerTexture();
    void drawAimDots();
    void drawPlayer();

    void drawHud();
    void drawHealthBar();
    void drawStatBars();

    void drawMenu();
    void drawButton(const SDL_FRect& r,const std::string& label,bool hover,const SDL_Color& fill);
    void drawPauseMenu();
    void drawMarketPrompt();
    void drawShop();
    void drawLevelSelect();
    void drawDead();
    void drawWin();
    void drawSeasonTransition();
    void drawWorld(bool showPlayer);
};

#endif
