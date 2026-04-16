#include "Game.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

LevelData SquareJumpGame::generateSummerLevel(int levelNum) {
    LevelData data;
    data.theme=SUMMER_THEME; data.season=SEASON_SUMMER;
    int relLevel=levelNum-30;

    int pfCount=PLATFORM_COUNT_MIN+(std::rand()%(PLATFORM_COUNT_MAX-PLATFORM_COUNT_MIN+1));
    data.worldH=static_cast<float>(screenH)*1.6f+pfCount*60.0f;

    float sandY=data.worldH-SUMMER_SAND_HEIGHT;
    float waterLineY=sandY-40.0f;
    data.waterLineY=waterLineY;

    Platform sandGround{};
    sandGround.x=0; sandGround.y=sandY; sandGround.w=data.worldW+6000;
    sandGround.h=SUMMER_SAND_HEIGHT; sandGround.isGround=true;
    data.platforms.push_back(sandGround);

    WaterZone sea{};
    sea.x=-200; sea.y=waterLineY; sea.w=10000; sea.h=40+SUMMER_SAND_HEIGHT;
    sea.isDangerous=(relLevel>20);
    data.waterZones.push_back(sea);

    float curX=300, curY=waterLineY-80;
    float highX=0;
    for (int i=0;i<pfCount-1;i++) {
        float pw=randf(80,130);
        float ph=28;
        Platform pf{};
        pf.x=curX; pf.y=curY; pf.w=pw; pf.h=ph;
        pf.isBuoy=true;
        pf.buoyBobOffset=randf(0,PI*2);
        data.platforms.push_back(pf);
        highX=curX+pw;
        float rise=randf(30,60);
        if (curY-rise<80) rise=randf(15,30);
        curX+=pw+randf(PLATFORM_GAP_MIN,PLATFORM_GAP_MAX);
        curY-=rise;
    }
    float finalPw=randf(100,140);
    Platform finalPf{};
    finalPf.x=curX; finalPf.y=curY; finalPf.w=finalPw; finalPf.h=28;
    finalPf.isBuoy=true; finalPf.buoyBobOffset=randf(0,PI*2);
    data.platforms.push_back(finalPf);
    highX=curX+finalPw;

    data.gate={curX+finalPw*0.5f-28,curY-76,56,76};
    data.worldW=highX+900;
    data.platforms.front().w=data.worldW+200;

    if (relLevel>10) {
        WaterZone dw{};
        dw.x=data.worldW*0.35f; dw.y=waterLineY-60; dw.w=400; dw.h=60;
        dw.isDangerous=true;
        data.waterZones.push_back(dw);
    }

    data.startX=60; data.startY=sandY-PLAYER_HEIGHT-10;

    for (int i=0;i<WORLD_STAR_COUNT/2;i++)
        data.stars.push_back({randf(0,data.worldW),randf(0,data.worldH),
                              randf(1,3),randf(0.2f,0.8f),randf(0,PI*2)});
    if (data.platforms.size()>4) {
        size_t mid=data.platforms.size()/2;
        const Platform& cp=data.platforms[mid];
        data.checkpoints.push_back({cp.x+cp.w*0.5f,cp.y});
    }
    return data;
}

LevelData SquareJumpGame::generateAutumnLevel(int levelNum) {
    LevelData data;
    data.theme=AUTUMN_THEME; data.season=SEASON_AUTUMN;
    int relLevel=levelNum-60;
    int pfCount=PLATFORM_COUNT_MIN+(std::rand()%(PLATFORM_COUNT_MAX-PLATFORM_COUNT_MIN+1));
    data.worldH=static_cast<float>(screenH)+pfCount*55+300;
    float groundY=data.worldH-44;
    data.platforms.push_back({0,groundY,5000,44,true});
    float curX=200,curY=groundY-100,highX=0;
    for (int i=0;i<pfCount-1;i++) {
        float pw=randf(PLATFORM_WIDTH_MIN,PLATFORM_WIDTH_MAX);
        Platform pf{};
        pf.x=curX;pf.y=curY;pf.w=pw;pf.h=22;
        if (std::rand()%4==0){pf.isMooncake=true;pf.mooncakeBites=MOONCAKE_BITES;pf.mooncakeActive=true;}
        data.platforms.push_back(pf);
        highX=curX+pw;
        float rise=randf(PLATFORM_RISE_MIN,PLATFORM_RISE_MAX);
        if (curY-rise<80) rise=randf(20,38);
        curX+=pw+randf(PLATFORM_GAP_MIN,PLATFORM_GAP_MAX); curY-=rise;
    }
    float finalW=randf(240,320);
    float bossAreaX=curX;
    data.platforms.push_back({bossAreaX,curY,finalW,22});
    highX=bossAreaX+finalW;
    data.gate={bossAreaX+finalW*0.5f-28,curY-76,56,76};
    data.worldW=highX+900; data.platforms.front().w=data.worldW;
    if (relLevel>=5) {
        data.hasBoss=true;
        Boss b{}; b.x=bossAreaX+30; b.y=curY-80-b.h; b.w=80; b.h=80;
        b.state=BossState::Sleeping; b.stateTimer=BOSS_SLEEP_TICKS; b.active=true;
        data.boss=b;
    }
    data.startX=84; data.startY=groundY-PLAYER_HEIGHT-10;
    for (int i=0;i<WORLD_STAR_COUNT;i++)
        data.stars.push_back({randf(0,data.worldW),randf(0,data.worldH),randf(1,3),randf(0.2f,0.8f),randf(0,PI*2)});
    if (data.platforms.size()>6) {
        size_t mid=data.platforms.size()/2;
        const Platform& cp=data.platforms[mid];
        data.checkpoints.push_back({cp.x+cp.w*0.5f,cp.y});
    }
    return data;
}

LevelData SquareJumpGame::generateWinterLevel(int levelNum) {
    LevelData data;
    data.theme=WINTER_THEME; data.season=SEASON_WINTER;
    int relLevel=levelNum-90;
    int pfCount=PLATFORM_COUNT_MIN+(std::rand()%(PLATFORM_COUNT_MAX-PLATFORM_COUNT_MIN+1));
    data.worldH=static_cast<float>(screenH)+pfCount*60+300;
    float groundY=data.worldH-44;
    Platform gnd{}; gnd.x=0;gnd.y=groundY;gnd.w=5000;gnd.h=44;gnd.isGround=true;gnd.isIcy=true;
    data.platforms.push_back(gnd);
    float curX=200,curY=groundY-100,highX=0;
    for (int i=0;i<pfCount-1;i++) {
        float pw=randf(PLATFORM_WIDTH_MIN,PLATFORM_WIDTH_MAX);
        Platform pf{}; pf.x=curX;pf.y=curY;pf.w=pw;pf.h=22;pf.isIcy=true;
        data.platforms.push_back(pf);
        highX=curX+pw;
        float rise=randf(PLATFORM_RISE_MIN,PLATFORM_RISE_MAX);
        if (curY-rise<80) rise=randf(20,38);
        curX+=pw+randf(PLATFORM_GAP_MIN,PLATFORM_GAP_MAX); curY-=rise;
    }
    float finalW=randf(185,245);
    Platform fp{}; fp.x=curX;fp.y=curY;fp.w=finalW;fp.h=22;fp.isIcy=true;
    data.platforms.push_back(fp);
    highX=curX+finalW;
    data.gate={curX+finalW*0.5f-28,curY-76,56,76};
    data.worldW=highX+900; data.platforms.front().w=data.worldW;
    int slowCount=4+relLevel/5;
    for (int i=0;i<slowCount;i++) {
        SnowZone sz{};
        sz.x=randf(0,data.worldW-300);sz.y=randf(groundY-400,groundY-80);
        sz.w=randf(100,250);sz.h=randf(60,150);sz.isDamaging=(relLevel>15&&std::rand()%3==0);
        data.snowZones.push_back(sz);
    }
    data.startX=84; data.startY=groundY-PLAYER_HEIGHT-10;
    for (int i=0;i<WORLD_STAR_COUNT;i++)
        data.stars.push_back({randf(0,data.worldW),randf(0,data.worldH),randf(1,3),randf(0.2f,0.8f),randf(0,PI*2)});
    if (data.platforms.size()>6) {
        size_t mid=data.platforms.size()/2;
        const Platform& cp=data.platforms[mid];
        data.checkpoints.push_back({cp.x+cp.w*0.5f,cp.y});
    }
    return data;
}

LevelData SquareJumpGame::generateDesertLevel(int levelNum) {
    LevelData data;
    data.theme=DESERT_THEME; data.season=SEASON_DESERT;
    int relLevel=levelNum-120;
    int pfCount=PLATFORM_COUNT_MIN+(std::rand()%(PLATFORM_COUNT_MAX-PLATFORM_COUNT_MIN+1));
    data.worldH=static_cast<float>(screenH)+pfCount*50+300;
    float groundY=data.worldH-44;
    data.platforms.push_back({0,groundY,5000,44,true});
    float curX=200,curY=groundY-120,highX=0;
    float gapMult=1.4f;
    for (int i=0;i<pfCount-1;i++) {
        float pw=randf(PLATFORM_WIDTH_MIN,PLATFORM_WIDTH_MAX);
        data.platforms.push_back({curX,curY,pw,22});
        highX=curX+pw;
        float rise=randf(PLATFORM_RISE_MIN,PLATFORM_RISE_MAX);
        if (curY-rise<80) rise=randf(20,38);
        curX+=pw+randf(PLATFORM_GAP_MIN,PLATFORM_GAP_MAX)*gapMult; curY-=rise;
    }
    float finalW=randf(185,245);
    data.platforms.push_back({curX,curY,finalW,22});
    highX=curX+finalW;
    data.gate={curX+finalW*0.5f-28,curY-76,56,76};
    data.worldW=highX+900; data.platforms.front().w=data.worldW;
    int camelCount=2+std::rand()%3;
    for (int i=0;i<camelCount;i++) {
        Camel c{};
        c.x=randf(200,data.worldW-300); c.y=groundY;
        c.facingRight=(std::rand()%2==0); c.vx=c.facingRight?CAMEL_SPEED:-CAMEL_SPEED;
        data.camels.push_back(c);
    }
    int oasisCount=1+relLevel/10;
    for (int i=0;i<oasisCount;i++) {
        Oasis o{}; o.x=randf(300,data.worldW-600); o.y=groundY-16;
        data.oases.push_back(o);
    }
    data.startX=84; data.startY=groundY-PLAYER_HEIGHT-10;
    for (int i=0;i<WORLD_STAR_COUNT/2;i++)
        data.stars.push_back({randf(0,data.worldW),randf(0,data.worldH),randf(1,3),randf(0.2f,0.8f),randf(0,PI*2)});
    if (data.platforms.size()>6) {
        size_t mid=data.platforms.size()/2;
        const Platform& cp=data.platforms[mid];
        data.checkpoints.push_back({cp.x+cp.w*0.5f,cp.y});
    }
    return data;
}

static void applyPhysics(Player& player, float groundFriction) {
    if (player.onGround) { player.vx*=groundFriction; if(std::fabs(player.vx)<0.1f)player.vx=0; }
    else player.vx*=AIR_FRICTION;
    if (!player.onGround) {
        if (std::fabs(player.vy)<GLIDE_THRESHOLD) player.vy+=GRAVITY*GLIDE_FACTOR;
        else player.vy+=GRAVITY;
    }
    if (player.vy>18) player.vy=18;
    player.x+=player.vx; player.y+=player.vy;
}

void SquareJumpGame::updateBuoys(bool qPressed) {
    for (int bi=0;bi<static_cast<int>(levelData.platforms.size());bi++) {
        Platform& pf=levelData.platforms[bi];
        if (!pf.isBuoy||pf.buoyGone) continue;

        if (!pf.buoyActivated) {
            float bob=std::sin(ticks*0.04f+pf.buoyBobOffset)*5.0f;
            pf.y+=bob*0.05f;

            if (qPressed&&player.ridingBuoyIndex==bi) {
                pf.buoyActivated=true;
                pf.buoyFlyTimer=BUOY_FLY_DURATION;
                pf.buoyVx=BUOY_FLY_SPEED_X;
                pf.buoyVy=BUOY_FLY_SPEED_Y;
            }
        } else {
            pf.x+=pf.buoyVx;
            pf.y+=pf.buoyVy;
            pf.buoyVy+=0.08f;
            pf.buoyFlyTimer--;
            if (pf.buoyFlyTimer<=0) {
                pf.buoyGone=true;
                if (player.ridingBuoyIndex==bi) {
                    player.ridingBuoyIndex=-1;
                    player.onGround=false;
                }
            }
        }
    }
}

void SquareJumpGame::updateSummer(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    bool qDown    =keys[SDL_SCANCODE_Q];
    int  effCharge=upgrades.effectiveMaxCharge();

    player.ridingBuoyIndex=-1;

    if (player.onGround) { player.vx*=GROUND_FRICTION; if(std::fabs(player.vx)<0.1f)player.vx=0; }
    else player.vx*=AIR_FRICTION;

    if (!player.onGround) {
        if (std::fabs(player.vy)<GLIDE_THRESHOLD) player.vy+=GRAVITY*GLIDE_FACTOR;
        else player.vy+=GRAVITY;
    }
    if (player.vy>18) player.vy=18;
    player.x+=player.vx; player.y+=player.vy;
    player.x=std::max(0.0f,player.x);
    if (player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}

    resolvePlayerCollisions();
    resolvePlayerBuoyCollisions();

    if (player.y>levelData.worldH+80){takeDamage(25);respawnAtCheckpoint();}

    if (spaceDown&&!player.prevSpace) {
        if (player.charging&&player.onGround) {
        } else if (player.onGround) {
            player.charging=true; player.chargeType=1; player.chargeTime=0;
        }
    }
    if (spaceDown&&player.charging&&player.onGround) {
        player.chargeTime=std::min(player.chargeTime+1,effCharge);
        if (ticks%3==0) spawnChargeParticle(player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,255,255,255});
    }
    if (!spaceDown&&player.charging) {
        if (player.onGround) releaseChargedJump();
        else { player.charging=false; player.chargeTime=0; }
    }

    updateBuoys(qDown&&!player.prevSpace);

    bool inWater=false;
    for (const WaterZone& wz:levelData.waterZones) {
        if (intersects(player.x,player.y,player.width,player.height,wz.x,wz.y,wz.w,wz.h)) {
            inWater=true;
            player.waterSubmergedTicks+=1.0f;
            if (player.waterSubmergedTicks>=WATER_SUBMERSION_MAX)
                takeDamage(static_cast<int>(wz.isDangerous?WATER_DROWN_RATE:WATER_DROWN_RATE*0.5f));
            break;
        }
    }
    if (!inWater) {
        player.waterSubmergedTicks=0;
        bool inSun=(player.y-camY)<screenH*0.45f;
        if (inSun) player.heat=std::min(HEAT_MAX,player.heat+HEAT_RATE_SUN*upgrades.heatMultiplier());
        else player.heat=std::max(0.0f,player.heat-HEAT_COOL_AIR);
    } else {
        player.heat=std::max(0.0f,player.heat-HEAT_COOL_WATER);
    }
    if (player.heat>=HEAT_DAMAGE_THRESHOLD) takeDamage(static_cast<int>(HEAT_OVERHEAT_RATE));

    updateCheckpoints();
    player.prevSpace=spaceDown;
    if (player.damageFlashTimer>0) player.damageFlashTimer--;
}

void SquareJumpGame::updateAutumn(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    bool eDown=keys[SDL_SCANCODE_E];
    bool iDown=keys[SDL_SCANCODE_I];
    int  effCharge=upgrades.effectiveMaxCharge();
    applyPhysics(player,GROUND_FRICTION);
    player.x=std::max(0.0f,player.x);
    if(player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}
    if (spaceDown) {
        if(player.onGround){if(!player.charging){player.charging=true;player.chargeType=1;player.chargeTime=0;}}
        else if(!player.charging&&ticks-player.lastAirJumpTick>=AIR_JUMP_COOLDOWN){player.charging=true;player.chargeType=2;player.chargeTime=0;}
        if(player.charging){player.chargeTime=std::min(player.chargeTime+1,effCharge);if(ticks%3==0)spawnChargeParticle(player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,255,255,255});}
    }
    if (!spaceDown&&player.charging) releaseChargedJump();
    resolvePlayerCollisions();
    if(player.y>levelData.worldH+80){takeDamage(20);respawnAtCheckpoint();}
    if(player.hunger<=0&&ticks%60==0) takeDamage(static_cast<int>(HUNGER_DAMAGE_RATE*60));
    if (eDown&&player.onGround) {
        for (Platform& pf:levelData.platforms) {
            if(!pf.isMooncake||!pf.mooncakeActive||pf.mooncakeBites<=0) continue;
            float pfBottom=pf.y;
            float playerBottom=player.y+player.height;
            if(std::fabs(playerBottom-pfBottom)<5&&player.x+player.width>pf.x&&player.x<pf.x+pf.w) {
                if(ticks%30==0){pf.mooncakeBites--;heal(MOONCAKE_HEAL);player.hunger=std::min(HUNGER_MAX,player.hunger+25.0f);spawnPurchaseBurst(player.x+player.width*0.5f,player.y);if(pf.mooncakeBites<=0)pf.mooncakeActive=false;}
                break;
            }
        }
    }
    bool iJust=iDown&&!player.prevI;
    if(iJust&&!player.invincible&&ticks-player.lastInvincibleTick>=INVINCIBLE_COOLDOWN) {
        activateInvincibility();
        for(int i=0;i<20;i++) spawnChargeParticle({255,255,100,255});
    }
    if(player.invincible&&ticks-player.lastInvincibleTick>=INVINCIBLE_DURATION) player.invincible=false;
    if(levelData.hasBoss){updateBoss();updateFireballs();}
    if(std::rand()%SPINY_SPAWN_CHANCE==0){SpinyLeaf sl{};sl.x=randf(camX-50,camX+screenW+50);sl.y=camY-20;sl.vy=randf(1.5f,3.5f);sl.angle=randf(0,360);sl.spin=randf(-4,4);levelData.spinyLeaves.push_back(sl);}
    updateSpinyLeaves();
    updateCheckpoints();
    player.prevSpace=spaceDown; player.prevI=iDown;
    if(player.damageFlashTimer>0) player.damageFlashTimer--;
}

void SquareJumpGame::updateBoss() {
    Boss& b=levelData.boss;
    if(!b.active) return;
    b.stateTimer--;
    float px=player.x+player.width*0.5f,py=player.y+player.height*0.5f;
    float bx=b.x+b.w*0.5f,by=b.y+b.h*0.5f;
    float distX=px-bx,distY=py-by;
    switch(b.state) {
        case BossState::Sleeping:
            if(b.stateTimer<=0){b.state=BossState::Waking;b.stateTimer=BOSS_WAKE_TICKS;}
            break;
        case BossState::Waking:
            if(distX>0){b.vx=BOSS_SPEED*0.5f;b.facingRight=true;}else{b.vx=-BOSS_SPEED*0.5f;b.facingRight=false;}
            b.x+=b.vx;
            if(b.stateTimer<=0){b.state=BossState::Attacking;b.stateTimer=BOSS_FIRE_TICKS;b.fireTimer=0;}
            break;
        case BossState::Attacking:{
            if(distX>0){b.vx=BOSS_SPEED;b.facingRight=true;}else{b.vx=-BOSS_SPEED;b.facingRight=false;}
            b.x+=b.vx;
            b.fireTimer--;
            if(b.fireTimer<=0){b.fireTimer=FIREBALL_INTERVAL;float dist=std::sqrt(distX*distX+distY*distY);if(dist<1)dist=1;Fireball f{};f.x=bx;f.y=by;f.vx=distX/dist*FIREBALL_SPEED;f.vy=distY/dist*FIREBALL_SPEED;levelData.fireballs.push_back(f);}
            if(b.stateTimer<=0){b.state=BossState::Sleeping;b.stateTimer=BOSS_SLEEP_TICKS;}
            break;
        }
        case BossState::Rage:{
            if(distX>0){b.vx=BOSS_RAGE_SPEED;b.facingRight=true;}else{b.vx=-BOSS_RAGE_SPEED;b.facingRight=false;}
            b.x+=b.vx;
            b.fireTimer--;
            if(b.fireTimer<=0){b.fireTimer=FIREBALL_INTERVAL/2;float dist=std::sqrt(distX*distX+distY*distY);if(dist<1)dist=1;Fireball f{};f.x=bx;f.y=by;f.vx=distX/dist*FIREBALL_SPEED;f.vy=distY/dist*FIREBALL_SPEED;levelData.fireballs.push_back(f);}
            break;
        }
    }
    b.x=clampf(b.x,0,levelData.worldW-b.w);
    if(b.state!=BossState::Sleeping&&b.state!=BossState::Rage) {
        if(intersects(player.x,player.y,player.width,player.height,b.x,b.y,b.w,b.h)){b.state=BossState::Rage;b.stateTimer=99999;takeDamage(20);}
    } else if(b.state==BossState::Rage) {
        if(intersects(player.x,player.y,player.width,player.height,b.x,b.y,b.w,b.h)&&ticks%30==0) takeDamage(10);
    }
}

void SquareJumpGame::updateFireballs() {
    for(int i=static_cast<int>(levelData.fireballs.size())-1;i>=0;i--) {
        Fireball& f=levelData.fireballs[i];
        f.x+=f.vx;f.y+=f.vy;f.life--;
        if(f.life<=0){levelData.fireballs.erase(levelData.fireballs.begin()+i);continue;}
        if(!player.invincible&&intersects(player.x,player.y,player.width,player.height,f.x-8,f.y-8,16,16)){
            takeDamage(FIREBALL_DAMAGE);levelData.fireballs.erase(levelData.fireballs.begin()+i);
        }
    }
}

void SquareJumpGame::updateSpinyLeaves() {
    for(int i=static_cast<int>(levelData.spinyLeaves.size())-1;i>=0;i--) {
        SpinyLeaf& sl=levelData.spinyLeaves[i];
        sl.y+=sl.vy;sl.angle+=sl.spin;
        if(!player.invincible&&intersects(player.x,player.y,player.width,player.height,sl.x-6,sl.y-6,12,12)){
            takeDamage(SPINY_LEAF_DAMAGE);levelData.spinyLeaves.erase(levelData.spinyLeaves.begin()+i);continue;
        }
        if(sl.y>camY+screenH+50) levelData.spinyLeaves.erase(levelData.spinyLeaves.begin()+i);
    }
}

void SquareJumpGame::updateWinter(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    int  effCharge=upgrades.effectiveMaxCharge();
    applyPhysics(player,WINTER_GROUND_FRICTION);
    player.x=std::max(0.0f,player.x);
    if(player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}
    if(spaceDown){
        if(player.onGround){if(!player.charging){player.charging=true;player.chargeType=1;player.chargeTime=0;}}
        else if(!player.charging&&ticks-player.lastAirJumpTick>=AIR_JUMP_COOLDOWN){player.charging=true;player.chargeType=2;player.chargeTime=0;}
        if(player.charging){player.chargeTime=std::min(player.chargeTime+1,effCharge);if(ticks%3==0)spawnChargeParticle(player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,255,255,255});}
    }
    if(!spaceDown&&player.charging) releaseChargedJump();
    resolvePlayerCollisions();
    if(player.y>levelData.worldH+80){takeDamage(25);respawnAtCheckpoint();}
    for(const SnowZone& sz:levelData.snowZones) {
        if(intersects(player.x,player.y,player.width,player.height,sz.x,sz.y,sz.w,sz.h)){
            if(sz.isDamaging){if(ticks%30==0)takeDamage(static_cast<int>(WINTER_DAMAGE_RATE*30));}
            else{player.vx*=WINTER_SLOW_FACTOR;player.vy*=WINTER_SLOW_FACTOR;}
        }
    }
    if(ticks%SNOW_PARTICLE_RATE==0) spawnSnowParticle();
    updateCheckpoints();
    player.prevSpace=spaceDown;
    if(player.invincible&&ticks-player.lastInvincibleTick>=INVINCIBLE_DURATION) player.invincible=false;
    if(player.damageFlashTimer>0) player.damageFlashTimer--;
}

void SquareJumpGame::updateDesert(const bool* keys) {
    bool spaceDown=keys[SDL_SCANCODE_SPACE];
    bool eDown=keys[SDL_SCANCODE_E];
    int  effCharge=upgrades.effectiveMaxCharge();
    bool riding=(player.camelIndex>=0);
    if (!riding) {
        applyPhysics(player,GROUND_FRICTION);
        player.x=std::max(0.0f,player.x);
        if(player.x>levelData.gate.x+800){player.x=levelData.gate.x+800;player.vx=0;}
        bool canJump=player.jumpsLeft>0;
        if(spaceDown&&canJump){
            if(player.onGround){if(!player.charging){player.charging=true;player.chargeType=1;player.chargeTime=0;}}
            else if(!player.charging&&ticks-player.lastAirJumpTick>=AIR_JUMP_COOLDOWN){player.charging=true;player.chargeType=2;player.chargeTime=0;}
            if(player.charging){player.chargeTime=std::min(player.chargeTime+1,effCharge);if(ticks%3==0)spawnChargeParticle(player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,200,50,255});}
        }
        if(!spaceDown&&player.charging) releaseChargedJump(true);
        resolvePlayerCollisions();
        resolvePlayerCamelCollisions();
    } else {
        Camel& camel=levelData.camels[player.camelIndex];
        if(!camel.active||camel.waterLevel<CAMEL_WATER_NEED*0.1f){player.camelIndex=-1;camel.hasPlayer=false;}
        else{player.x=camel.x+16;player.y=camel.y-30-PLAYER_HEIGHT;player.vx=camel.vx;player.vy=0;player.onGround=true;camel.waterLevel=std::max(0.0f,camel.waterLevel-0.05f);}
        if((spaceDown&&!player.prevSpace)||eDown){if(player.camelIndex>=0){levelData.camels[player.camelIndex].hasPlayer=false;player.camelIndex=-1;}}
        resolvePlayerCollisions();
    }
    if(player.y>levelData.worldH+80){takeDamage(30);respawnAtCheckpoint();}
    player.thirst=std::max(0.0f,player.thirst-THIRST_PER_TICK);
    if(player.thirst<=THIRST_DANGER&&ticks%60==0) takeDamage(static_cast<int>(THIRST_DAMAGE_RATE*60));
    updateCamels();
    if(eDown&&upgrades.hasShovel){
        for(Oasis& o:levelData.oases){
            if(o.depleted) continue;
            if(intersects(player.x,player.y,player.width,player.height,o.x,o.y,o.w,o.h)){
                o.beingDug=true;o.digTimer++;
                if(o.digTimer>=OASIS_DIG_TIME){player.camelOasisWater+=OASIS_WATER_AMOUNT;o.depleted=true;o.beingDug=false;o.digTimer=0;player.thirst=std::min(THIRST_MAX,player.thirst+30.0f);spawnPurchaseBurst(o.x+o.w*0.5f,o.y);}
                break;
            } else if(o.beingDug){o.beingDug=false;o.digTimer=std::max(0,o.digTimer-2);}
        }
    }
    if(player.camelOasisWater>0){
        for(Camel& c:levelData.camels){
            if(!c.active||c.hasPlayer) continue;
            if(intersects(player.x,player.y,player.width,player.height,c.x,c.y,60,60)){
                float give=std::min(player.camelOasisWater,100.0f-c.waterLevel);
                c.waterLevel+=give;player.camelOasisWater-=give;
                if(give>0) spawnPurchaseBurst(c.x+30,c.y-20);
                break;
            }
        }
    }
    updateCheckpoints();
    player.prevSpace=spaceDown;
    if(player.damageFlashTimer>0) player.damageFlashTimer--;
}

void SquareJumpGame::updateCamels() {
    for(Camel& c:levelData.camels){
        if(!c.active||c.hasPlayer) continue;
        c.x+=c.vx;
        if(c.x<0){c.x=0;c.vx=-c.vx;c.facingRight=!c.facingRight;}
        if(c.x>levelData.worldW-80){c.x=levelData.worldW-80;c.vx=-c.vx;c.facingRight=!c.facingRight;}
        c.y=levelData.worldH-44-60;
    }
}
