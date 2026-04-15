#include "Game.h"
#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
//  PRIMITIVES
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::setColor(const SDL_Color& c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}
void SquareJumpGame::fillRect(float x, float y, float w, float h, const SDL_Color& c) {
    SDL_FRect r={x,y,w,h}; setColor(c); SDL_RenderFillRect(renderer, &r);
}
void SquareJumpGame::fillCircle(float cx, float cy, float radius, const SDL_Color& c) {
    if (radius<=0) return;
    setColor(c);
    int ir=static_cast<int>(std::ceil(radius));
    for (int iy=-ir;iy<=ir;iy++) {
        float dy=static_cast<float>(iy);
        float dx=std::sqrt(std::max(0.0f,radius*radius-dy*dy));
        SDL_RenderLine(renderer, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}
void SquareJumpGame::fillRoundedRect(float x, float y, float w, float h, float r, const SDL_Color& c) {
    r=clampf(r,0,std::min(w,h)*0.5f);
    if (r<=0.5f) { fillRect(x,y,w,h,c); return; }
    fillRect(x+r, y,   w-r*2, h,   c);
    fillRect(x,   y+r, r,     h-r*2, c);
    fillRect(x+w-r, y+r, r, h-r*2, c);
    fillCircle(x+r,   y+r,   r, c);
    fillCircle(x+w-r, y+r,   r, c);
    fillCircle(x+r,   y+h-r, r, c);
    fillCircle(x+w-r, y+h-r, r, c);
}
void SquareJumpGame::drawGlowRect(float x, float y, float w, float h, const SDL_Color& c, int layers, float spread) {
    for (int i=layers;i>=1;i--) {
        float t=static_cast<float>(i)/static_cast<float>(layers);
        float pad=spread*t;
        fillRoundedRect(x-pad,y-pad,w+pad*2,h+pad*2,14+pad,alpha(c,static_cast<Uint8>(22*t)));
    }
}
void SquareJumpGame::drawThickLine(float x1,float y1,float x2,float y2,float th,const SDL_Color& c) {
    float dx=x2-x1,dy=y2-y1;
    float len=std::sqrt(dx*dx+dy*dy);
    if (len<=0.001f){fillCircle(x1,y1,th*0.5f,c);return;}
    float nx=-dy/len, ny=dx/len;
    int bands=std::max(1,static_cast<int>(std::ceil(th)));
    for (int i=0;i<bands;i++) {
        float off=i-(bands-1)*0.5f;
        setColor(c);
        SDL_RenderLine(renderer, x1+nx*off, y1+ny*off, x2+nx*off, y2+ny*off);
    }
}
void SquareJumpGame::drawGradientVertical(const SDL_Color& top, const SDL_Color& bot) {
    const int bands=80;
    for (int i=0;i<bands;i++) {
        float t0=static_cast<float>(i)/bands;
        float t1=static_cast<float>(i+1)/bands;
        SDL_Color c=blend(top,bot,(t0+t1)*0.5f);
        fillRect(0, screenH*t0, static_cast<float>(screenW), std::ceil(screenH*(t1-t0))+1, c);
    }
}
void SquareJumpGame::drawText(float x, float y, const std::string& s, const SDL_Color& c, float scale) {
    if (s.empty()) return;
    setColor(c);
    SDL_SetRenderScale(renderer, scale, scale);
    SDL_RenderDebugText(renderer, x/scale, y/scale, s.c_str());
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}
float SquareJumpGame::textWidth(const std::string& s, float scale) const {
    return static_cast<float>(s.size()) * DEBUG_TEXT_CHAR_W * scale;
}
void SquareJumpGame::drawCenteredText(float cx, float y, const std::string& s, const SDL_Color& c, float scale) {
    drawText(cx - textWidth(s, scale)*0.5f, y, s, c, scale);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  BACKGROUNDS
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawParallaxStars(const std::vector<Star>& stars, float wW, float wH) {
    for (const Star& s : stars) {
        float sx=std::fmod(s.x-camX*0.3f, wW);
        float sy=std::fmod(s.y-camY*0.2f, wH);
        if(sx<0)sx+=wW; if(sy<0)sy+=wH;
        float pulse=0.7f+0.3f*std::sin(ticks*0.03f+s.phase);
        fillRect(sx,sy,s.size,s.size,alpha({255,255,255,255},static_cast<Uint8>(255*s.alpha*pulse)));
    }
}

void SquareJumpGame::drawMenuBackground() {
    drawGradientVertical({1,4,9,255},{13,17,23,255});
    fillCircle(screenW*0.25f, screenH*0.28f, 170, alpha({0,212,255,255},20));
    fillCircle(screenW*0.78f, screenH*0.22f, 190, alpha({255,214,0,255},16));
    for (const Star& s : menuStars) {
        float pulse=0.65f+0.35f*std::sin(ticks*0.025f+s.phase);
        fillRect(s.x,s.y,s.size,s.size,alpha({255,255,255,255},static_cast<Uint8>(255*s.alpha*pulse)));
    }
    for (const Petal& p : petals) drawPetal(p, true);
    fillRoundedRect(screenW*0.5f-280,screenH*0.5f-160,560,400,24,alpha({1,4,9,255},210));
    drawGlowRect   (screenW*0.5f-280,screenH*0.5f-160,560,400,{0,230,118,255},4,10);
}

void SquareJumpGame::drawSpringBackdrop() {
    drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
    fillCircle(screenW*0.82f, screenH*0.20f, 110, alpha({255,241,118,255},70));
    fillCircle(screenW*0.82f, screenH*0.20f,  70, alpha({255,255,255,255},30));
    fillCircle(screenW*0.18f, screenH*0.78f, 220, alpha({129,199,132,255},28));
    for (const Petal& p : petals) drawPetal(p, true);
    float step=std::max(240.0f, screenW/4.5f);
    for (float x=100; x<screenW+120; x+=step) {
        float sc=0.9f+std::sin(x*0.01f)*0.12f;
        drawTree(x, static_cast<float>(screenH)-38, 82*sc, 0, 8*sc, 0);
    }
}

void SquareJumpGame::drawSummerBackground() {
    // Sky gradient – tropical blue
    drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
    // Sun disc
    float sunX=screenW*0.80f, sunY=screenH*0.15f;
    fillCircle(sunX,sunY,55,alpha({255,255,200,255},200));
    fillCircle(sunX,sunY,40,alpha({255,255,255,255},200));
    // Heat shimmer rings
    for (int i=0;i<3;i++) {
        float r=65+i*22+std::sin(ticks*0.06f+i)*6;
        fillCircle(sunX,sunY,r,alpha({255,230,100,255},static_cast<Uint8>(18-i*5)));
    }
    // Horizon glow
    fillRect(0, screenH*0.78f, static_cast<float>(screenW), screenH*0.22f,
             alpha({25,118,210,255},90));
    // Distant hills
    for (int i=0;i<5;i++) {
        float hx=i*screenW*0.22f - std::fmod(camX*0.06f, screenW*0.22f);
        fillCircle(hx, screenH*0.82f, 100+i*20, alpha({33,150,243,255},40));
    }
    // Stars at top (night-ish portion)
    drawParallaxStars(levelData.stars,
        std::max(levelData.worldW,static_cast<float>(screenW)),
        std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawAutumnBackground() {
    drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
    // Moon
    fillCircle(screenW*0.75f, screenH*0.12f, 55, alpha({255,235,59,255},200));
    fillCircle(screenW*0.75f, screenH*0.12f, 42, alpha({255,255,200,255},220));
    // Glow rings
    for (int i=0;i<3;i++) {
        float r=65+i*20+std::sin(ticks*0.04f+i)*5;
        fillCircle(screenW*0.75f, screenH*0.12f, r, alpha({255,193,7,255},static_cast<Uint8>(15-i*4)));
    }
    // Background trees (silhouette)
    for (int i=0;i<6;i++) {
        float tx=i*screenW*0.18f - std::fmod(camX*0.05f,screenW*0.18f);
        float ty=static_cast<float>(screenH)-80;
        fillRect(tx-4,ty-80,8,80,alpha({30,15,5,255},180));
        fillCircle(tx,ty-80,35,alpha({60,30,10,255},160));
    }
    drawParallaxStars(levelData.stars,
        std::max(levelData.worldW,static_cast<float>(screenW)),
        std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawWinterBackground() {
    drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
    // Pale moon
    fillCircle(screenW*0.5f, screenH*0.10f, 40, alpha({200,220,255,255},160));
    // Far snow hills
    for (int i=0;i<5;i++) {
        float hx=i*screenW*0.24f - std::fmod(camX*0.04f,screenW*0.24f);
        fillCircle(hx, screenH*0.88f, 140+i*15, alpha({200,215,235,255},60));
    }
    drawParallaxStars(levelData.stars,
        std::max(levelData.worldW,static_cast<float>(screenW)),
        std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawDesertBackground() {
    drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
    // Blazing sun
    float sunX=screenW*0.85f, sunY=screenH*0.12f;
    for (int i=4;i>=0;i--) {
        float r=50+i*18+std::sin(ticks*0.05f)*4;
        fillCircle(sunX,sunY,r,alpha({255,200,50,255},static_cast<Uint8>(40-i*7)));
    }
    fillCircle(sunX,sunY,48,alpha({255,240,120,255},240));
    // Dunes
    for (int i=0;i<5;i++) {
        float dx=i*screenW*0.25f - std::fmod(camX*0.07f,screenW*0.25f);
        fillCircle(dx, static_cast<float>(screenH), 220+i*30,
                   alpha({180,140,70,255},static_cast<Uint8>(80+i*10)));
    }
    drawParallaxStars(levelData.stars,
        std::max(levelData.worldW,static_cast<float>(screenW)),
        std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawWorldBackground() {
    if (levelData.isSpring || state==GameState::Market) {
        drawSpringBackdrop();
    } else {
        switch (levelData.season) {
            case SEASON_SUMMER: drawSummerBackground(); break;
            case SEASON_AUTUMN: drawAutumnBackground(); break;
            case SEASON_WINTER: drawWinterBackground(); break;
            case SEASON_DESERT: drawDesertBackground(); break;
            default:
                drawGradientVertical(currentTheme.bg1, currentTheme.bg2);
                drawParallaxStars(levelData.stars,
                    std::max(levelData.worldW,static_cast<float>(screenW)),
                    std::max(levelData.worldH,static_cast<float>(screenH)));
                break;
        }
    }
}

// ── Winter overlay / Desert heat shimmer ──────────────────────────────────────
void SquareJumpGame::drawWinterEffect() {
    // Blizzard overlay: alpha-pulsing white veil
    float pulse = 0.5f + 0.5f * std::sin(ticks * 0.03f);
    fillRect(0, 0, static_cast<float>(screenW), static_cast<float>(screenH),
             alpha({200,220,255,255}, static_cast<Uint8>(WINTER_VISIBILITY * 255 * (0.6f + 0.4f * pulse))));
}
void SquareJumpGame::drawDesertHeat() {
    // Subtle shimmer lines near the ground
    float groundScreenY = (levelData.worldH - 44.0f) - camY;
    for (int i = 0; i < 6; i++) {
        float shimY = groundScreenY - i * 12.0f - std::sin(ticks*0.08f + i) * 4.0f;
        if (shimY < 0 || shimY > screenH) continue;
        fillRect(0, shimY, static_cast<float>(screenW), 2.0f,
                 alpha({255, 200, 100, 255}, static_cast<Uint8>(20 - i * 3)));
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  TREE / PETAL
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawTree(float x,float y,float len,float angleDeg,float branchW,int depth) {
    float rad=angleDeg*PI/180;
    float x2=x+std::sin(rad)*len, y2=y-std::cos(rad)*len;
    drawThickLine(x,y,x2,y2,branchW,{93,64,55,255});
    if (depth>=6||len<12) {
        fillCircle(x2,y2,4.5f,currentTheme.petalA);
        fillCircle(x2+4,y2+1,3.5f,currentTheme.petalB);
        return;
    }
    drawTree(x2,y2,len*0.75f,angleDeg-20,branchW*0.72f,depth+1);
    drawTree(x2,y2,len*0.75f,angleDeg+20,branchW*0.72f,depth+1);
}

void SquareJumpGame::drawPetal(const Petal& p, bool screenSpace) {
    float depth=lerpf(0.45f,1.35f,p.z);
    float x=p.x, y=p.y;
    if (!screenSpace) { x-=camX*depth*0.05f; y-=camY*depth*0.05f; }
    SDL_Color c=blend(currentTheme.petalA,currentTheme.petalB,
                      std::sin((ticks+p.angle)*0.02f)*0.5f+0.5f);
    c.a=static_cast<Uint8>(120+p.z*110);
    float r=p.size*depth;
    fillCircle(x,y,r,alpha(c,c.a));
    fillCircle(x+r*0.6f,y-r*0.2f,r*0.55f,alpha(currentTheme.petalB,static_cast<Uint8>(c.a*0.8f)));
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PLATFORMS
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawMetalPlatform(const Platform& pf, float x, float y, float w, float h) {
    fillRect(x,y,w,h,currentTheme.platformBody);
    fillRect(x,y,w,2,currentTheme.platformTop);
    fillRect(x,y,2,h,currentTheme.platformTop);
    fillRect(x,y+h-2,w,2,currentTheme.platformShadow);
    fillRect(x+w-2,y,2,h,currentTheme.platformShadow);
    int startSeam=static_cast<int>(std::max(20.0f,std::floor((camX-pf.x)/35)*35+20));
    for (int local=startSeam; local<pf.w-10; local+=35) {
        float sx=x+local;
        fillRect(sx,y,2,h,{26,32,44,255});
        fillCircle(sx+1,y+5,1.5f,{113,128,150,255});
        fillCircle(sx+1,y+h-5,1.5f,{113,128,150,255});
    }
}

void SquareJumpGame::drawSpringPlatform(float x,float y,float w,float h,bool isGround) {
    if (isGround) {
        fillRect(x,y,w,h,{93,64,55,255});
        fillRect(x,y,w,12,{76,175,80,255});
        for (float stripe=12;stripe<w;stripe+=22)
            fillRect(x+stripe,y+8,2,h-8,alpha({121,85,72,255},90));
    } else {
        fillRect(x,y,w,h,currentTheme.platformBody);
        fillRect(x,y,w,3,currentTheme.platformTop);
        fillRect(x+w*0.33f,y,2,h,currentTheme.platformTop);
        fillRect(x+w*0.66f,y,2,h,currentTheme.platformTop);
        fillRect(x,y+h*0.5f-1,w,2,alpha(currentTheme.platformTop,160));
    }
}

void SquareJumpGame::drawBuoyPlatform(const Platform& pf, float sx, float sy) {
    // Orange buoy body
    fillCircle(sx+pf.w*0.5f, sy+pf.h*0.5f, pf.w*0.5f, {255,152,0,255});
    fillCircle(sx+pf.w*0.5f, sy+pf.h*0.5f, pf.w*0.4f, {255,193,7,255});
    // Cross stripe
    fillRect(sx, sy+pf.h*0.5f-2, pf.w, 4, {211,47,47,255});
    fillRect(sx+pf.w*0.5f-2, sy, 4, pf.h, {211,47,47,255});
    // Top flat surface
    fillRect(sx+4, sy, pf.w-8, 4, alpha({255,255,255,255},120));
    if (pf.buoyActivated) {
        // Flying trail
        drawGlowRect(sx,sy,pf.w,pf.h,{255,193,7,255},3,8);
    } else {
        // "Press E" hint when near player
        float pdx = (player.x+player.width*0.5f) - (pf.x+pf.w*0.5f);
        float pdy = (player.y+player.height*0.5f) - (pf.y+pf.h*0.5f);
        if (pdx*pdx+pdy*pdy < 120*120)
            drawText(sx+pf.w*0.5f-16, sy-18, "[E]", {255,255,200,255}, 1.0f);
    }
}

void SquareJumpGame::drawMooncakePlatform(const Platform& pf, float sx, float sy) {
    // Mooncake-colored platform
    if (!pf.mooncakeActive) {
        fillRect(sx,sy,pf.w,pf.h,alpha({121,85,72,255},120));
        drawText(sx+pf.w*0.5f-20, sy-14, "EATEN", {150,100,50,255}, 1.0f);
        return;
    }
    SDL_Color crust={183,128,64,255};
    SDL_Color top  ={230,180,90,255};
    fillRect(sx,sy,pf.w,pf.h,crust);
    fillRect(sx+2,sy,pf.w-4,5,top);
    // Pattern lines
    for (float lx=10;lx<pf.w-5;lx+=14)
        fillRect(sx+lx,sy,2,pf.h,alpha({150,100,40,255},100));
    // Bite counter
    drawText(sx+4, sy+5, std::to_string(pf.mooncakeBites)+" BITES", {255,235,59,255}, 1.0f);
    // E hint
    float pdx=(player.x+player.width*0.5f)-(pf.x+pf.w*0.5f);
    float pdy=(player.y+player.height*0.5f)-(pf.y+pf.h*0.5f);
    if (pdx*pdx+pdy*pdy < 130*130)
        drawText(sx+pf.w*0.5f-12, sy-18, "[E]EAT", {255,255,200,255}, 1.0f);
}

void SquareJumpGame::drawPlatforms() {
    for (const Platform& pf : levelData.platforms) {
        float x=pf.x-camX, y=pf.y-camY;
        if (x+pf.w<0||x>screenW||y+pf.h<0||y>screenH) continue;

        if (pf.isMooncake) {
            drawMooncakePlatform(pf,x,y);
        } else if (pf.isBuoy) {
            drawBuoyPlatform(pf,x,y);
        } else if (levelData.isSpring || state==GameState::Market) {
            drawSpringPlatform(x,y,pf.w,pf.h,pf.isGround);
        } else if (levelData.season==SEASON_WINTER) {
            // Icy look
            drawMetalPlatform(pf,x,y,pf.w,pf.h);
            fillRect(x,y,pf.w,3,alpha({200,230,255,255},180));
        } else {
            drawMetalPlatform(pf,x,y,pf.w,pf.h);
        }
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  WATER / SNOW ZONES
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawWaterZones() {
    for (const WaterZone& wz : levelData.waterZones) {
        float x=wz.x-camX, y=wz.y-camY;
        if (x+wz.w<0||x>screenW||y+wz.h<0||y>screenH) continue;
        // Animated water
        SDL_Color wc = wz.isDangerous ? SDL_Color{180,30,30,255} : SDL_Color{30,100,200,255};
        fillRect(x,y,wz.w,wz.h,alpha(wc,160));
        // Wave lines
        for (int i=0;i<4;i++) {
            float wy=y+i*16+std::sin(ticks*0.05f+i)*4;
            if (wy<y||wy>y+wz.h) continue;
            fillRect(x,wy,wz.w,3,alpha({255,255,255,255},40));
        }
        if (wz.isDangerous) {
            drawText(x+10, y+8, "DANGER!", {255,100,100,255}, 1.5f);
        }
    }
}

void SquareJumpGame::drawSnowZones() {
    for (const SnowZone& sz : levelData.snowZones) {
        float x=sz.x-camX, y=sz.y-camY;
        if (x+sz.w<0||x>screenW||y+sz.h<0||y>screenH) continue;
        SDL_Color sc = sz.isDamaging ? SDL_Color{150,100,200,255} : SDL_Color{200,220,255,255};
        fillRect(x,y,sz.w,sz.h,alpha(sc,60));
        // Border
        setColor(alpha(sc,120));
        SDL_FRect br={x,y,sz.w,sz.h};
        SDL_RenderRect(renderer,&br);
        drawText(x+4, y+4, sz.isDamaging ? "BLIZZARD" : "SLOW SNOW",
                 sz.isDamaging ? SDL_Color{200,150,255,255} : SDL_Color{180,210,255,255}, 1.0f);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  NPCs / ENTITIES
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawSpringNpc() {
    if (!levelData.isSpring || !levelData.hasSpringNpc) return;
    const SpringNpc& npc=levelData.springNpc;
    float x=npc.x-camX, y=npc.y-camY;
    // Head
    fillCircle(x+14,y+8,7,{255,224,178,255});
    // Body
    fillRoundedRect(x+4,y+16,20,18,5,{211,47,47,255});
    // Arms
    fillRect(x+2,y+20,4,14,{255,224,178,255});
    fillRect(x+22,y+20,4,14,{255,224,178,255});
    // Legs
    fillRect(x+8,y+34,4,6,{76,175,80,255});
    fillRect(x+16,y+34,4,6,{76,175,80,255});
    // Hat
    fillRect(x+8,y+14,12,3,{255,235,59,255});
    if (!npc.gifted) {
        // Li xi envelope
        fillRect(x+23,y+18,8,10,{255,193,7,255});
        fillRect(x+25,y+15,4,3,{255,152,0,255});
        drawGlowRect(x+20,y+14,12,16,{255,235,59,255},3,4);
        drawText(x-6,y-18,"LIXI +"+std::to_string(npc.reward),{255,255,255,255},1.0f);
    } else {
        drawText(x+1,y-18,"CHUC MUNG",{255,235,59,255},1.0f);
    }
}

void SquareJumpGame::drawChildNpcs() {
    for (const ChildNpc& child : levelData.childNpcs) {
        if (!child.active) continue;
        float x=child.x-camX, y=child.y-camY;
        SDL_Color bodyColor = child.angered ? SDL_Color{220,30,30,255} : SDL_Color{100,181,246,255};
        // Head
        fillCircle(x+14,y+7,6,{255,224,178,255});
        // Body
        fillRoundedRect(x+6,y+14,16,14,4,bodyColor);
        // Legs
        fillRect(x+8,y+28,5,7,{33,33,120,255});
        fillRect(x+15,y+28,5,7,{33,33,120,255});
        // Face – angry or neutral
        if (child.angered) {
            // X eyes
            fillRect(x+9,y+4,4,2,{0,0,0,255});
            fillRect(x+18,y+4,4,2,{0,0,0,255});
            // Angry mouth
            fillRect(x+10,y+11,9,2,{0,0,0,255});
        } else {
            // Normal eyes
            fillCircle(x+11,y+6,2,{0,0,0,255});
            fillCircle(x+17,y+6,2,{0,0,0,255});
            fillRoundedRect(x+10,y+10,8,2,1,{0,0,0,255});
        }
        // Demand label
        drawText(x-8, y-20,
                 child.angered ? "GIVE! -"+std::to_string(child.cost)+"LIXI" :
                                 "PAY "+std::to_string(child.cost)+" LIXI",
                 child.angered ? SDL_Color{255,80,80,255} : SDL_Color{255,220,100,255},
                 1.0f);
    }
}

void SquareJumpGame::drawStalls() {
    if (state!=GameState::Market) return;
    for (const Stall& stall : levelData.stalls) {
        float x=stall.x-camX, y=stall.y-camY;
        fillRect(x,y,stall.w,stall.h,{141,110,99,255});
        fillRect(x-10,y-20,stall.w+20,20,{216,67,21,255});
        drawGlowRect(x-6,y-12,stall.w+12,stall.h+12,{255,235,59,255},2,4);
        if (stall.bought) drawText(x+12,y+18,"DA MUA",{255,255,255,255},1.0f);
        else {
            drawText(x+12,y+18,std::to_string(stall.cost)+" LIXI",{255,255,255,255},1.0f);
            drawText(x+12,y+40,stall.name,{255,255,255,255},1.0f);
        }
    }
}

void SquareJumpGame::drawBoss() {
    if (!levelData.hasBoss || !levelData.boss.active) return;
    const Boss& b=levelData.boss;
    float x=b.x-camX, y=b.y-camY;
    if (x+b.w<-20||x>screenW+20||y+b.h<-20||y>screenH+20) return;

    // Glow based on state
    SDL_Color glowC = (b.state==BossState::Rage)     ? SDL_Color{255,23,68,255}  :
                      (b.state==BossState::Attacking)? SDL_Color{255,152,0,255}  :
                      (b.state==BossState::Waking)   ? SDL_Color{255,235,59,255} :
                                                        SDL_Color{100,200,100,255};
    drawGlowRect(x,y,b.w,b.h,glowC,4,12);

    // Body
    fillRoundedRect(x+4,y+20,b.w-8,b.h-20,10,{50,20,10,255});

    // Lion head (top)
    float headX=x+b.w*0.5f, headY=y+18;
    fillCircle(headX,headY,26,{200,100,20,255});
    // Mane
    for (int i=0;i<8;i++) {
        float ma=i*PI*2/8+ticks*0.02f;
        fillCircle(headX+std::cos(ma)*26, headY+std::sin(ma)*24, 10,
                   blend({200,100,20,255},{255,152,0,255},std::sin(ticks*0.05f+i)*0.5f+0.5f));
    }
    // Face
    fillCircle(headX,headY,18,{230,140,40,255});
    // Eyes
    bool sleeping=(b.state==BossState::Sleeping);
    if (sleeping) {
        fillRect(headX-10,headY-4,9,2,{0,0,0,255});
        fillRect(headX+2, headY-4,9,2,{0,0,0,255});
    } else {
        fillCircle(headX-7,headY-4,4,{255,255,255,255});
        fillCircle(headX+7,headY-4,4,{255,255,255,255});
        SDL_Color pupil=(b.state==BossState::Rage) ? SDL_Color{255,23,68,255} : SDL_Color{0,0,0,255};
        fillCircle(headX-7,headY-4,2,pupil);
        fillCircle(headX+7,headY-4,2,pupil);
    }
    // Mouth
    fillRoundedRect(headX-8,headY+6,16,5,2,{50,10,5,255});
    // Fire breath indicator
    if (b.state==BossState::Attacking||b.state==BossState::Rage) {
        float fireScale=0.6f+0.4f*std::sin(ticks*0.2f);
        for (int fi=0;fi<4;fi++) {
            float fr=8+fi*5+fireScale*4;
            float fx=headX+(b.facingRight ? fr*2 : -fr*2);
            fillCircle(fx,headY+6,fr,alpha(blend({255,152,0,255},{255,50,0,255},static_cast<float>(fi)/4),
                       static_cast<Uint8>(180-fi*35)));
        }
    }
    // State label
    const char* stateName="ZZZ";
    if (b.state==BossState::Waking)   stateName="!";
    if (b.state==BossState::Attacking)stateName="FIRE!";
    if (b.state==BossState::Rage)     stateName="RAGE!";
    drawText(x+b.w*0.5f-20, y-22, stateName,
             b.state==BossState::Rage ? SDL_Color{255,23,68,255} : SDL_Color{255,235,59,255}, 1.5f);
}

void SquareJumpGame::drawFireballs() {
    for (const Fireball& f : levelData.fireballs) {
        float sx=f.x-camX, sy=f.y-camY;
        for (int i=3;i>=0;i--) {
            float r=10-i*2+std::sin(ticks*0.3f)*1.5f;
            fillCircle(sx,sy,r,alpha(blend({255,152,0,255},{255,50,0,255},static_cast<float>(i)/3),
                       static_cast<Uint8>(200-i*40)));
        }
    }
}

void SquareJumpGame::drawSpinyLeaves() {
    for (const SpinyLeaf& sl : levelData.spinyLeaves) {
        float sx=sl.x-camX, sy=sl.y-camY;
        if (sx<-20||sx>screenW+20||sy<-20||sy>screenH+20) continue;
        // Leaf diamond shape (rotated square)
        SDL_Color lc = blend({180,100,20,255},{220,60,20,255},std::sin(sl.angle*0.05f)*0.5f+0.5f);
        fillCircle(sx,sy,6,lc);
        // Spike tips
        for (int i=0;i<4;i++) {
            float sa=(sl.angle+i*90)*PI/180;
            fillCircle(sx+std::cos(sa)*8, sy+std::sin(sa)*8, 3, alpha({255,100,30,255},200));
        }
    }
}

void SquareJumpGame::drawCamels() {
    for (const Camel& c : levelData.camels) {
        if (!c.active) continue;
        float x=c.x-camX, y=c.y-camY;
        bool flip=!c.facingRight;
        float dir=flip ? -1.0f : 1.0f;

        // Body
        fillRoundedRect(x+8,y-40,50,30,12,{180,140,80,255});
        // Hump (or two)
        fillCircle(x+20,y-50,14,{160,120,60,255});
        fillCircle(x+38,y-50,14,{160,120,60,255});
        // Head
        float hx = flip ? x+10 : x+48;
        fillCircle(hx,y-42,12,{200,160,90,255});
        // Eye
        fillCircle(hx+(flip?-5:5),y-46,3,{0,0,0,255});
        // Legs
        for (int i=0;i<4;i++) {
            float lx=x+12+i*12;
            float bob=std::sin(ticks*0.12f+i*0.7f)*3;
            fillRect(lx,y-14+bob,6,18,{150,110,55,255});
            fillRect(lx-1,y+4+bob,8,5,{120,90,40,255});
        }
        // Water level indicator
        if (c.waterLevel > 0) {
            float barW=50.0f, barH=5.0f;
            fillRoundedRect(x+8,y-60,barW,barH,2,alpha({100,180,255,255},100));
            fillRoundedRect(x+8,y-60,barW*(c.waterLevel/100.0f),barH,2,{30,150,255,255});
        }
        // Rideable indicator
        if (c.waterLevel >= CAMEL_WATER_NEED)
            drawGlowRect(x+8,y-40,50,30,{30,150,255,255},2,6);
        // Feed hint
        if (c.waterLevel < CAMEL_WATER_NEED) {
            float pdx=(player.x+player.width*0.5f)-(c.x+30);
            float pdy=(player.y+player.height*0.5f)-(c.y-20);
            if (pdx*pdx+pdy*pdy < 130*130)
                drawText(x, y-70, "NEEDS WATER", {100,180,255,255}, 1.0f);
        }
    }
}

void SquareJumpGame::drawOases() {
    for (const Oasis& o : levelData.oases) {
        float x=o.x-camX, y=o.y-camY;
        if (o.depleted) {
            fillRect(x,y,o.w,o.h,alpha({120,90,50,255},100));
            drawText(x+4,y-14,"DRY",{150,100,50,255},1.0f);
            continue;
        }
        // Water surface
        fillRect(x,y,o.w,o.h,alpha({30,120,200,255},200));
        // Shimmer
        fillRect(x+2, y+2, o.w*0.6f, 4, alpha({255,255,255,255},60));
        // Digging progress bar
        if (o.beingDug) {
            float prog=static_cast<float>(o.digTimer)/OASIS_DIG_TIME;
            fillRoundedRect(x,y-10,o.w,6,3,alpha({255,255,255,255},80));
            fillRoundedRect(x,y-10,o.w*prog,6,3,{30,200,255,255});
        }
        // Hint
        float pdx=(player.x+player.width*0.5f)-(o.x+o.w*0.5f);
        float pdy=(player.y+player.height*0.5f)-(o.y+o.h*0.5f);
        if (pdx*pdx+pdy*pdy < 120*120)
            drawText(x, y-22,
                     upgrades.hasShovel ? "[E]DIG" : "NEED SHOVEL",
                     {30,200,255,255}, 1.0f);
    }
}

void SquareJumpGame::drawCheckpoints() {
    for (const Checkpoint& cp : levelData.checkpoints) {
        float x=cp.x-camX, y=cp.y-camY;
        if (x<-30||x>screenW+30||y<-60||y>screenH+30) continue;
        SDL_Color flagC = cp.reached ? SDL_Color{255,235,59,255} : SDL_Color{255,255,255,180};
        // Pole
        drawThickLine(x,y,x,y-40,2,flagC);
        // Flag
        for (int fy=0;fy<3;fy++) {
            float wave=std::sin(ticks*0.1f+fy*0.5f)*3;
            fillRect(x+1, y-40+fy*7+wave, 18, 6, flagC);
        }
        if (cp.reached) drawGlowRect(x-8,y-48,20,48,{255,235,59,255},3,8);
    }
}

void SquareJumpGame::drawParticles() {
    for (const Particle& p : particles) {
        float ar=p.maxLife>0 ? p.life/p.maxLife : 0;
        SDL_Color c=alpha(p.color,static_cast<Uint8>(255*clampf(ar,0,1)));
        fillCircle(p.x-camX, p.y-camY, p.size, c);
    }
}

void SquareJumpGame::drawGate() {
    float x=levelData.gate.x-camX, y=levelData.gate.y-camY;
    float glow=18+std::sin(ticks*0.1f)*8;
    drawGlowRect(x-6,y-6,levelData.gate.w+12,levelData.gate.h+12,currentTheme.gate,5,glow*0.35f);
    fillRoundedRect(x+10,y+10,levelData.gate.w-20,levelData.gate.h-20,10,
                    alpha(currentTheme.gate,static_cast<Uint8>(180+std::sin(ticks*0.1f)*35)));
    fillRoundedRect(x+15,y+15,levelData.gate.w-30,levelData.gate.h-30,8,{255,255,255,235});
    // Arrow
    float arrowY=y+levelData.gate.h*0.5f-8;
    fillRect(x+levelData.gate.w*0.5f-2,arrowY,4,16,{0,0,0,200});
    fillRect(x+levelData.gate.w*0.5f-8,arrowY-8,17,4,{0,0,0,200});
}

// ═══════════════════════════════════════════════════════════════════════════════
//  PLAYER
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::renderPlayerTexture() {
    SDL_SetRenderTarget(renderer, playerTexture);
    SDL_SetRenderDrawColor(renderer,0,0,0,0);
    SDL_RenderClear(renderer);

    bool springOutfit=levelData.isSpring||state==GameState::Market;
    bool chargeFace=player.charging;
    bool rightClosed=chargeFace||(player.blinkTimer>0&&player.blinkRight);
    bool leftClosed=player.blinkTimer>0&&!player.blinkRight;

    // Flash red when damaged
    SDL_Color playerAccent = (player.damageFlashTimer>0 && (player.damageFlashTimer/3)%2==0)
                             ? SDL_Color{255,50,50,255} : currentTheme.playerAccent;
    SDL_Color playerBody   = (player.invincible && (ticks/4)%2==0)
                             ? SDL_Color{200,200,255,255} : currentTheme.playerPrimary;

    fillRoundedRect(2,2,32,32,9,playerAccent);
    fillRoundedRect(4,4,28,28,8,playerBody);
    fillRoundedRect(6,6,24,24,7,alpha(blend(playerBody,{255,255,255,255},0.06f),255));

    if (springOutfit) {
        fillRoundedRect(8,18,20,10,4,{255,235,59,255});
        fillRect(16.5f,18,3,10,{211,47,47,255});
        fillRect(11,20,2,6,{255,193,7,255});
        fillRect(23,20,2,6,{255,193,7,255});
        fillRect(12,15,12,3,{255,248,225,255});
    }
    // Left eye
    if (!leftClosed) {
        fillRoundedRect(9,10,6,8,2,{255,255,255,255});
        fillCircle(12,16,1.5f,{0,0,0,255});
    } else fillRect(9,15,6,2,{255,255,255,255});
    // Right eye
    if (!rightClosed) {
        fillRoundedRect(21,10,6,8,2,{255,255,255,255});
        fillCircle(24,16,1.5f,{0,0,0,255});
    } else fillRect(21,15,6,2,{255,255,255,255});
    // Mouth
    if (chargeFace) fillCircle(18,24,2.5f,{255,255,255,255});
    else            fillRoundedRect(14,23,8,3,1.5f,{255,255,255,255});

    SDL_SetRenderTarget(renderer, nullptr);
}

void SquareJumpGame::drawAimDots() {
    if (!player.charging) return;
    int effCharge=upgrades.effectiveMaxCharge();
    float ratio=static_cast<float>(player.chargeTime)/effCharge;
    float mwx=mouseScreenX+camX, mwy=mouseScreenY+camY;
    float ang=std::atan2(mwy-(player.y+player.height*0.5f), mwx-(player.x+player.width*0.5f));
    float startX=player.x+player.width*0.5f-camX;
    float startY=player.y+player.height*0.5f-camY;
    float dist=80+ratio*250;
    SDL_Color dotC=player.chargeTime>=effCharge ? currentTheme.gate : SDL_Color{255,255,255,255};
    for (int i=1;i<=12;i++) {
        float t=static_cast<float>(i)/12.0f;
        float px=startX+std::cos(ang)*dist*t;
        float py=startY+std::sin(ang)*dist*t;
        fillCircle(px,py,2.5f-t,alpha(dotC,static_cast<Uint8>((1-t)*255)));
    }
}

void SquareJumpGame::drawPlayer() {
    // Charge glow ring
    if (player.charging) {
        float pulse=0.5f+0.5f*std::sin(ticks*0.24f);
        fillCircle(player.x-camX+player.width*0.5f,
                   player.y-camY+player.height*0.5f,
                   24+pulse*6,
                   alpha(player.chargeTime>=upgrades.effectiveMaxCharge() ? currentTheme.gate : currentTheme.playerAccent, 50));
    }
    // Invincibility glow
    if (player.invincible) {
        fillCircle(player.x-camX+player.width*0.5f,
                   player.y-camY+player.height*0.5f,
                   22+std::sin(ticks*0.3f)*4,
                   alpha({200,200,255,255},60));
    }

    if (spriteSheet) {
        int frame=player.charging ? player.chargeAnimFrame : 0;
        float sw=0,sh=0;
        SDL_GetTextureSize(spriteSheet,&sw,&sh);
        float fw=sw*0.5f, fh=sh*0.5f;
        int col=frame%2, row=frame/2;
        SDL_FRect src={col*fw,row*fh,fw,fh};
        float ang=clampf(player.vx*2,-22,22);
        SDL_FRect dst={player.x-camX-4,player.y-camY-4,
                       static_cast<float>(PLAYER_TEXTURE_SIZE)+2,
                       static_cast<float>(PLAYER_TEXTURE_SIZE)+2};
        SDL_RenderTextureRotated(renderer,spriteSheet,&src,&dst,ang,nullptr,SDL_FLIP_NONE);
    } else {
        renderPlayerTexture();
        float ang=clampf(player.vx*2,-22,22);
        SDL_FRect dst={player.x-camX-4,player.y-camY-4,
                       static_cast<float>(PLAYER_TEXTURE_SIZE)+2,
                       static_cast<float>(PLAYER_TEXTURE_SIZE)+2};
        SDL_RenderTextureRotated(renderer,playerTexture,nullptr,&dst,ang,nullptr,SDL_FLIP_NONE);
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
//  HUD
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawHealthBar() {
    float ratio=static_cast<float>(player.health)/static_cast<float>(player.maxHealth);
    SDL_Color hc = ratio>0.5f ? SDL_Color{76,175,80,255} :
                   ratio>0.25f ? SDL_Color{255,193,7,255} : SDL_Color{244,67,54,255};
    // Background
    fillRoundedRect(screenW-180,8,160,14,7,alpha({0,0,0,255},120));
    // Fill
    fillRoundedRect(screenW-180,8,160*ratio,14,7,hc);
    // HP text
    drawText(screenW-176,10,std::to_string(player.health)+"/"+std::to_string(player.maxHealth),
             {255,255,255,255},1.0f);
}

void SquareJumpGame::drawStatBars() {
    float panelX=8.0f, panelY=60.0f;
    int season=getSeasonFromLevel(currentLevel);

    auto drawBar=[&](float y,float val,float maxVal,const SDL_Color& fillC,const std::string& label){
        fillRoundedRect(panelX,y,130,12,6,alpha({0,0,0,255},110));
        fillRoundedRect(panelX,y,130*(val/maxVal),12,6,fillC);
        drawText(panelX+4,y+2,label,{255,255,255,255},1.0f);
    };

    if (season==SEASON_SUMMER) {
        drawBar(panelY,   HEAT_MAX-player.heat, HEAT_MAX, {30,150,255,255},"COOL");
        drawBar(panelY+18,100-player.waterSubmergedTicks*(100.0f/WATER_SUBMERSION_MAX),100,
                {30,200,100,255},"BREATH");
    }
    if (season==SEASON_AUTUMN) {
        drawBar(panelY,   player.hunger, HUNGER_MAX, {255,152,0,255},"HUNGER");
        if (player.invincible) {
            Uint64 elapsed=ticks-player.lastInvincibleTick;
            float remain=1.0f-static_cast<float>(elapsed)/INVINCIBLE_DURATION;
            drawBar(panelY+18,remain*100,100,{200,200,255,255},"SHIELD");
        }
    }
    if (season==SEASON_DESERT) {
        drawBar(panelY,   player.thirst,THIRST_MAX,{30,150,255,255},"THIRST");
        drawBar(panelY+18,static_cast<float>(player.jumpsLeft),DESERT_JUMP_MAX,{255,235,59,255},"JUMPS");
        if (player.camelOasisWater>0)
            drawBar(panelY+36,player.camelOasisWater,100,{100,200,255,255},"WATER");
    }
}

void SquareJumpGame::drawHud() {
    // Top bar
    fillRect(0,0,static_cast<float>(screenW),52,alpha({0,0,0,255},128));

    // Season / level label
    const char* seasonLabel="";
    switch (getSeasonFromLevel(currentLevel)) {
        case SEASON_SPRING: seasonLabel="SPRING"; break;
        case SEASON_SUMMER: seasonLabel="SUMMER"; break;
        case SEASON_AUTUMN: seasonLabel="AUTUMN"; break;
        case SEASON_WINTER: seasonLabel="WINTER"; break;
        case SEASON_DESERT: seasonLabel="DESERT"; break;
    }
    if (state==GameState::Market) drawText(25,16,"CHO TET",{255,255,255,255},2.0f);
    else drawText(25,16,std::string(seasonLabel)+" LV"+std::to_string(currentLevel),{255,255,255,255},2.0f);

    // Li xi
    drawCenteredText(screenW*0.5f,16,"LIXI: "+std::to_string(lixiCount),{255,235,59,255},2.0f);

    // Air jump cooldown
    float cdRatio=clampf(static_cast<float>(ticks-player.lastAirJumpTick)/AIR_JUMP_COOLDOWN,0,1);
    fillRoundedRect(screenW-146,19,108,12,6,alpha({255,255,255,255},50));
    fillRoundedRect(screenW-146,19,108*cdRatio,12,6,
                    cdRatio>=1.0f ? currentTheme.playerAccent : SDL_Color{255,23,68,255});

    // Charge bar
    if (player.charging) {
        int effCharge=upgrades.effectiveMaxCharge();
        float cr=static_cast<float>(player.chargeTime)/effCharge;
        float bx=player.x-camX+player.width*0.5f-40;
        float by=player.y-camY-22;
        fillRoundedRect(bx,by,80,10,5,alpha({0,0,0,255},160));
        fillRoundedRect(bx,by,80*cr,10,5,
                        cr>=1.0f ? currentTheme.gate : blend({100,200,255,255},{255,214,0,255},cr));
    }

    drawHealthBar();
    drawStatBars();

    // P = Shop hint
    drawText(screenW-150,36,"[P]=SHOP [ESC]=MENU",{200,200,200,255},1.0f);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MENUS
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawButton(const SDL_FRect& r, const std::string& label, bool hover, const SDL_Color& fill) {
    SDL_Color base=hover ? blend(fill,{255,255,255,255},0.25f) : fill;
    float expand=hover?4:0;
    drawGlowRect(r.x-expand,r.y-expand,r.w+expand*2,r.h+expand*2,base,3,8);
    fillRoundedRect(r.x-expand,r.y-expand,r.w+expand*2,r.h+expand*2,12,base);
    drawCenteredText(r.x+r.w*0.5f, r.y+18-expand, label, {0,0,0,255}, 2.0f);
}

void SquareJumpGame::drawMenu() {
    drawMenuBackground();
    drawCenteredText(screenW*0.5f, screenH*0.5f-190, "SQUARE JUMP", {255,255,255,255}, 3.0f);

    // Panel
    SDL_FRect panel={screenW*0.5f-280,screenH*0.5f-145,560,395};
    fillRoundedRect(panel.x,panel.y,panel.w,panel.h,16,alpha({255,255,255,255},10));
    drawCenteredText(screenW*0.5f,panel.y+16, "GIU SPACE + CHUOT DE NHAY",{0,230,118,255},1.5f);
    drawCenteredText(screenW*0.5f,panel.y+44, "5 MUA: SPRING  SUMMER  AUTUMN  WINTER  DESERT",{255,235,59,255},1.0f);
    drawCenteredText(screenW*0.5f,panel.y+64, "MOI MUA CO CU CHE RIENG + BOSS + SHOP",{255,255,255,255},1.0f);
    drawCenteredText(screenW*0.5f,panel.y+84, "[ESC]=MENU  [P]=SHOP  [I]=INVINCIBLE(AUTUMN)",{180,180,180,255},1.0f);

    SDL_FRect play=playButtonRect();
    SDL_FRect cont=continueButtonRect();
    SDL_FRect shop=shopButtonRect();
    SDL_FRect exit=exitButtonRect();
    drawButton(play,hasSave?"NEW GAME":"PLAY",  pointInRect(mouseScreenX,mouseScreenY,play), {0,230,118,255});
    if (hasSave)
        drawButton(cont,"CONTINUE", pointInRect(mouseScreenX,mouseScreenY,cont),{0,188,212,255});
    drawButton(shop,"SHOP",     pointInRect(mouseScreenX,mouseScreenY,shop), {255,193,7,255});
    drawButton(exit,"THOAT",    pointInRect(mouseScreenX,mouseScreenY,exit), {255,112,67,255});
}

void SquareJumpGame::drawMarketPrompt() {
    drawWorldBackground();
    drawPlatforms();
    drawSpringNpc();
    drawParticles();
    drawGate();
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({1,4,9,255},210));
    SDL_FRect box={screenW*0.5f-250,screenH*0.5f-120,500,220};
    drawGlowRect(box.x,box.y,box.w,box.h,{0,230,118,255},4,12);
    fillRoundedRect(box.x,box.y,box.w,box.h,18,alpha({1,4,9,255},236));
    drawCenteredText(screenW*0.5f,box.y+34,"VAO CHO TET?",{255,255,255,255},3.0f);
    drawCenteredText(screenW*0.5f,box.y+92,"BAN MUON DOI QUA CHO TET KHONG",{0,230,118,255},1.5f);
    drawButton(promptYesRect(),"CO",    pointInRect(mouseScreenX,mouseScreenY,promptYesRect()),{0,230,118,255});
    drawButton(promptNoRect(), "KHONG", pointInRect(mouseScreenX,mouseScreenY,promptNoRect()), {255,183,77,255});
}

void SquareJumpGame::drawShop() {
    // Dim overlay
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},200));

    float panelW=520, panelH=520;
    float panelX=screenW*0.5f-panelW*0.5f, panelY=screenH*0.5f-panelH*0.5f;
    fillRoundedRect(panelX,panelY,panelW,panelH,20,alpha({10,15,30,255},240));
    drawGlowRect(panelX,panelY,panelW,panelH,{255,235,59,255},3,12);

    drawCenteredText(screenW*0.5f,panelY+16,"SHOP - LIXI: "+std::to_string(lixiCount),{255,235,59,255},2.5f);
    drawText(panelX+16,panelY+58,"[ESC] Close  [1-5] Buy",{180,180,180,255},1.0f);

    // 5 items
    struct ShopItem { std::string name; int cost; int level; int maxLevel; };
    ShopItem items[5]={
        {"[1] JUMP POWER  +" + std::to_string(static_cast<int>(SHOP_JUMP_BONUS)),  SHOP_JUMP_COST,   upgrades.jumpLevel,   SHOP_JUMP_MAX_LEVEL},
        {"[2] FAST CHARGE  -"+std::to_string(SHOP_CHARGE_BONUS)+"ticks",          SHOP_CHARGE_COST, upgrades.chargeLevel, SHOP_CHARGE_MAX_LEVEL},
        {"[3] MAX HEALTH  +"+std::to_string(SHOP_HEALTH_BONUS)+" HP",             SHOP_HEALTH_COST, upgrades.healthLevel, SHOP_HEALTH_MAX_LEVEL},
        {"[4] HEAT SHIELD  (summer)",                                              SHOP_HEAT_COST,   upgrades.heatLevel,   SHOP_HEAT_MAX_LEVEL},
        {"[5] FULL HEAL  (instant)",                                               SHOP_FULL_HEAL_COST, -1, -1},
    };

    float itemH=64, itemW=panelW-40;
    float startY=panelY+82, startX=panelX+20;
    for (int i=0;i<5;i++) {
        bool hover=(shopHover==i);
        bool maxed=(items[i].maxLevel>0 && items[i].level>=items[i].maxLevel);
        bool afford=(lixiCount>=items[i].cost);
        SDL_Color bgC = maxed ? SDL_Color{40,50,40,255} :
                        !afford ? SDL_Color{50,30,30,255} :
                        hover   ? SDL_Color{50,60,80,255} : SDL_Color{20,30,50,255};
        SDL_FRect r={startX, startY+i*(itemH+12), itemW, itemH};
        fillRoundedRect(r.x,r.y,r.w,r.h,10,bgC);
        if (hover&&afford&&!maxed) drawGlowRect(r.x,r.y,r.w,r.h,{0,230,118,255},2,8);

        SDL_Color textC = maxed ? SDL_Color{100,200,100,255} :
                          !afford ? SDL_Color{150,100,100,255} : SDL_Color{255,255,255,255};
        drawText(r.x+14, r.y+14, items[i].name, textC, 1.5f);

        if (maxed) {
            drawText(r.x+14,r.y+36,"MAX LEVEL",{100,200,100,255},1.0f);
        } else if (items[i].maxLevel>0) {
            drawText(r.x+14,r.y+36,"Level "+std::to_string(items[i].level)+"/"+std::to_string(items[i].maxLevel)+
                     "   Cost: "+std::to_string(items[i].cost)+" LIXI",textC,1.0f);
        } else {
            drawText(r.x+14,r.y+36,"Cost: "+std::to_string(items[i].cost)+" LIXI",textC,1.0f);
        }
    }
}

void SquareJumpGame::drawDead() {
    drawWorldBackground();
    drawPlatforms();
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},180));

    drawCenteredText(screenW*0.5f,screenH*0.5f-80,"YOU DIED",{244,67,54,255},4.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f-20,"Season: "+std::to_string(getSeasonFromLevel(currentLevel)+1)+
                     "  Level: "+std::to_string(currentLevel),{255,255,255,255},1.5f);

    SDL_FRect restartBtn={screenW*0.5f-120,screenH*0.5f+60,240,60};
    SDL_FRect menuBtn   ={screenW*0.5f-120,screenH*0.5f+140,240,60};
    drawButton(restartBtn,"[R] RESTART",pointInRect(mouseScreenX,mouseScreenY,restartBtn),{244,67,54,255});
    drawButton(menuBtn,   "[ESC] MENU", pointInRect(mouseScreenX,mouseScreenY,menuBtn),  {100,100,100,255});
    drawParticles();
}

void SquareJumpGame::drawWin() {
    drawGradientVertical({1,4,9,255},{10,20,40,255});
    for (const Petal& p : petals) drawPetal(p,true);
    drawCenteredText(screenW*0.5f,screenH*0.5f-130,"CONGRATULATIONS!",{255,235,59,255},3.5f);
    drawCenteredText(screenW*0.5f,screenH*0.5f-70,"YOU COMPLETED ALL 5 SEASONS!",{0,230,118,255},2.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+10,"LIXI COLLECTED: "+std::to_string(lixiCount),{255,235,59,255},2.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+70,"[ENTER] Return to Menu",{180,180,180,255},1.5f);
    drawParticles();
}

void SquareJumpGame::drawSeasonTransition() {
    static const char* seasonNames[]={"SPRING","SUMMER","AUTUMN","WINTER","DESERT"};
    static const SDL_Color seasonColors[]={
        {0,230,118,255},{33,150,243,255},{255,152,0,255},{135,206,235,255},{255,193,7,255}
    };
    int s=clampf(static_cast<float>(transitionSeason),0,4);
    float alpha_f=clampf(static_cast<float>(transitionTimer)/200.0f*2,0,1);
    if (transitionTimer<100) alpha_f=clampf(static_cast<float>(transitionTimer)/100.0f,0,1);

    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),
             alpha({0,0,0,255},static_cast<Uint8>(alpha_f*220)));
    SDL_Color sc=seasonColors[s];
    drawCenteredText(screenW*0.5f,screenH*0.5f-60,
                     "ENTERING "+std::string(seasonNames[s]),alpha(sc,static_cast<Uint8>(alpha_f*255)),3.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+20,
                     "Press any key to continue",alpha({200,200,200,255},static_cast<Uint8>(alpha_f*200)),1.5f);
}

// ═══════════════════════════════════════════════════════════════════════════════
//  MAIN DRAW
// ═══════════════════════════════════════════════════════════════════════════════
void SquareJumpGame::drawWorld(bool showPlayer) {
    drawWorldBackground();
    // Season-specific elements
    if (levelData.season==SEASON_SUMMER) drawWaterZones();
    if (levelData.season==SEASON_WINTER) drawSnowZones();
    if (levelData.season==SEASON_DESERT) { drawDesertHeat(); drawOases(); drawCamels(); }
    drawPlatforms();
    drawCheckpoints();
    drawSpringNpc();
    drawChildNpcs();
    drawStalls();
    if (levelData.hasBoss) { drawBoss(); drawFireballs(); drawSpinyLeaves(); }
    drawParticles();
    drawGate();
    if (showPlayer) { drawAimDots(); drawPlayer(); }
    // Winter: blizzard overlay AFTER player so it partially obscures everything
    if (levelData.season==SEASON_WINTER) drawWinterEffect();
    drawHud();
}

void SquareJumpGame::draw() {
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);

    switch (state) {
        case GameState::Menu:             drawMenu();           break;
        case GameState::MarketPrompt:     drawMarketPrompt();   break;
        case GameState::Market:           drawWorld(true);      break;
        case GameState::Playing:          drawWorld(true);      break;
        case GameState::Shop:             drawWorld(false); drawShop(); break;
        case GameState::Dead:             drawDead();           break;
        case GameState::Win:              drawWin();            break;
        case GameState::SeasonTransition: drawWorld(true); drawSeasonTransition(); break;
    }
    SDL_RenderPresent(renderer);
}
