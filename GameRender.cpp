#include "Game.h"
#include <algorithm>
#include <cmath>

void SquareJumpGame::setColor(const SDL_Color& c) { SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a); }

void SquareJumpGame::fillRect(float x,float y,float w,float h,const SDL_Color& c) {
    SDL_FRect r={x,y,w,h}; setColor(c); SDL_RenderFillRect(renderer,&r);
}

void SquareJumpGame::fillCircle(float cx,float cy,float radius,const SDL_Color& c) {
    if(radius<=0) return;
    setColor(c);
    int ir=static_cast<int>(std::ceil(radius));
    for(int iy=-ir;iy<=ir;iy++) {
        float dy=static_cast<float>(iy);
        float dx=std::sqrt(std::max(0.0f,radius*radius-dy*dy));
        SDL_RenderLine(renderer,cx-dx,cy+dy,cx+dx,cy+dy);
    }
}

void SquareJumpGame::fillRoundedRect(float x,float y,float w,float h,float r,const SDL_Color& c) {
    r=clampf(r,0,std::min(w,h)*0.5f);
    if(r<=0.5f){fillRect(x,y,w,h,c);return;}
    fillRect(x+r,y,w-r*2,h,c);
    fillRect(x,y+r,r,h-r*2,c);
    fillRect(x+w-r,y+r,r,h-r*2,c);
    fillCircle(x+r,y+r,r,c); fillCircle(x+w-r,y+r,r,c);
    fillCircle(x+r,y+h-r,r,c); fillCircle(x+w-r,y+h-r,r,c);
}

void SquareJumpGame::drawGlowRect(float x,float y,float w,float h,const SDL_Color& c,int layers,float spread) {
    for(int i=layers;i>=1;i--) {
        float t=static_cast<float>(i)/layers, pad=spread*t;
        fillRoundedRect(x-pad,y-pad,w+pad*2,h+pad*2,14+pad,alpha(c,static_cast<Uint8>(22*t)));
    }
}

void SquareJumpGame::drawThickLine(float x1,float y1,float x2,float y2,float th,const SDL_Color& c) {
    float dx=x2-x1,dy=y2-y1,len=std::sqrt(dx*dx+dy*dy);
    if(len<=0.001f){fillCircle(x1,y1,th*0.5f,c);return;}
    float nx=-dy/len,ny=dx/len;
    int bands=std::max(1,static_cast<int>(std::ceil(th)));
    for(int i=0;i<bands;i++) {
        float off=i-(bands-1)*0.5f;
        setColor(c);
        SDL_RenderLine(renderer,x1+nx*off,y1+ny*off,x2+nx*off,y2+ny*off);
    }
}

void SquareJumpGame::drawGradientVertical(const SDL_Color& top,const SDL_Color& bot) {
    const int bands=80;
    for(int i=0;i<bands;i++) {
        float t0=static_cast<float>(i)/bands, t1=static_cast<float>(i+1)/bands;
        SDL_Color c=blend(top,bot,(t0+t1)*0.5f);
        fillRect(0,screenH*t0,static_cast<float>(screenW),std::ceil(screenH*(t1-t0))+1,c);
    }
}

void SquareJumpGame::drawText(float x,float y,const std::string& s,const SDL_Color& c,float scale) {
    if(s.empty()) return;
    TTF_Font* font=pickFont(scale);
    if(!font) {
        setColor(c);
        SDL_SetRenderScale(renderer,scale,scale);
        SDL_RenderDebugText(renderer,x/scale,y/scale,s.c_str());
        SDL_SetRenderScale(renderer,1.0f,1.0f);
        return;
    }
    SDL_Surface* surf=TTF_RenderText_Blended(font,s.c_str(),0,c);
    if(!surf) return;
    SDL_Texture* tex=SDL_CreateTextureFromSurface(renderer,surf);
    SDL_DestroySurface(surf);
    if(!tex) return;
    float tw=0,th=0;
    SDL_GetTextureSize(tex,&tw,&th);
    SDL_FRect dst={x,y,tw,th};
    SDL_RenderTexture(renderer,tex,nullptr,&dst);
    SDL_DestroyTexture(tex);
}

float SquareJumpGame::textWidth(const std::string& s,float scale) const {
    TTF_Font* font=pickFont(scale);
    if(!font) return static_cast<float>(s.size())*DEBUG_TEXT_CHAR_W*scale;
    int w=0,h=0;
    TTF_GetStringSize(font,s.c_str(),0,&w,&h);
    return static_cast<float>(w);
}

void SquareJumpGame::drawCenteredText(float cx,float y,const std::string& s,const SDL_Color& c,float scale) {
    drawText(cx-textWidth(s,scale)*0.5f,y,s,c,scale);
}

void SquareJumpGame::drawDialogueBubble(float x,float y,const std::string& text,const SDL_Color& bg) {
    float tw=textWidth(text,1.5f);
    float bw=tw+24, bh=34;
    float bx=x-bw*0.5f, by=y-bh-16;
    fillRoundedRect(bx,by,bw,bh,8,alpha(bg,230));
    fillRoundedRect(bx+1,by+1,bw-2,bh-2,7,alpha({255,255,255,255},30));
    float tx=bx+bw*0.5f-6, ty2=by+bh;
    SDL_FRect tri1={tx,ty2,12,10}; setColor(alpha(bg,230)); SDL_RenderFillRect(renderer,&tri1);
    drawCenteredText(x,by+9,text,{20,20,20,255},1.5f);
}

void SquareJumpGame::drawParallaxStars(const std::vector<Star>& stars,float wW,float wH) {
    for(const Star& s:stars) {
        float sx=std::fmod(s.x-camX*0.3f,wW), sy=std::fmod(s.y-camY*0.2f,wH);
        if(sx<0)sx+=wW; if(sy<0)sy+=wH;
        float pulse=0.7f+0.3f*std::sin(ticks*0.03f+s.phase);
        fillRect(sx,sy,s.size,s.size,alpha({255,255,255,255},static_cast<Uint8>(255*s.alpha*pulse)));
    }
}

void SquareJumpGame::drawMenuBackground() {
    drawGradientVertical({1,4,9,255},{13,17,23,255});
    fillCircle(screenW*0.25f,screenH*0.28f,170,alpha({0,212,255,255},20));
    fillCircle(screenW*0.78f,screenH*0.22f,190,alpha({255,214,0,255},16));
    for(const Star& s:menuStars) {
        float pulse=0.65f+0.35f*std::sin(ticks*0.025f+s.phase);
        fillRect(s.x,s.y,s.size,s.size,alpha({255,255,255,255},static_cast<Uint8>(255*s.alpha*pulse)));
    }
    for(const Petal& p:petals) drawPetal(p,true);
}

void SquareJumpGame::drawSpringBackdrop() {
    drawGradientVertical(currentTheme.bg1,currentTheme.bg2);
    fillCircle(screenW*0.82f,screenH*0.20f,110,alpha({255,241,118,255},70));
    fillCircle(screenW*0.82f,screenH*0.20f,70,alpha({255,255,255,255},30));
    fillCircle(screenW*0.18f,screenH*0.78f,220,alpha({129,199,132,255},28));
    for(const Petal& p:petals) drawPetal(p,true);

    float groundWorldY = levelData.worldH - 44.0f;
    float treeScreenY  = groundWorldY - camY;
    float parallaxX    = std::fmod(camX * 0.6f, 240.0f);
    float step = std::max(220.0f, screenW / 5.0f);
    for(float fx = -parallaxX; fx < screenW + step; fx += step) {
        if(treeScreenY > -200 && treeScreenY < screenH + 100) {
            float sc = 0.9f + std::sin((fx + camX) * 0.01f) * 0.12f;
            drawTree(fx, treeScreenY, 82*sc, 0, 8*sc, 0);
        }
    }
}

void SquareJumpGame::drawSummerBackground() {
    float waterScreenY = levelData.waterLineY - camY;
    float sandScreenY  = (levelData.worldH - SUMMER_SAND_HEIGHT) - camY;

    drawGradientVertical({64,164,223,255},{120,200,240,255});

    float sunX=screenW*0.80f, sunY=screenH*0.12f;
    if(sunY < waterScreenY) {
        for(int i=4;i>=0;i--) {
            float r=40+i*14+std::sin(ticks*0.05f)*4;
            fillCircle(sunX,sunY,r,alpha({255,230,100,255},static_cast<Uint8>(30-i*5)));
        }
        fillCircle(sunX,sunY,36,alpha({255,250,200,255},240));
    }

    float seaTop = waterScreenY < 0 ? 0 : waterScreenY;
    float seaBot = sandScreenY > screenH ? static_cast<float>(screenH) : sandScreenY;
    if(seaTop < seaBot) {
        for(int i=0;i<8;i++) {
            float t=static_cast<float>(i)/8.0f;
            SDL_Color wc=blend({14,90,190,255},{25,120,220,255},t);
            float wy=seaTop+t*(seaBot-seaTop);
            fillRect(0,wy,static_cast<float>(screenW),(seaBot-seaTop)/8.0f+1,wc);
        }
        for(int i=0;i<6;i++) {
            float wavX=std::fmod(i*180.0f+ticks*0.8f-camX*0.2f,static_cast<float>(screenW)+200)-100;
            float wavY=seaTop+20+i*12+std::sin(ticks*0.04f+i)*5;
            if(wavY>seaTop&&wavY<seaBot)
                drawThickLine(wavX,wavY,wavX+140,wavY+3,2,alpha({200,230,255,255},60));
        }
    }

    if(sandScreenY < screenH && sandScreenY+SUMMER_SAND_HEIGHT > 0) {
        float sy=std::max(sandScreenY,0.0f);
        float sh=std::min(sandScreenY+SUMMER_SAND_HEIGHT,static_cast<float>(screenH))-sy;
        if(sh>0) {
            fillRect(0,sy,static_cast<float>(screenW),sh,{210,180,100,255});
            fillRect(0,sy,static_cast<float>(screenW),8,{225,200,130,255});
            for(int i=0;i<14;i++) {
                float gx=std::fmod(i*110.0f-camX*0.12f,screenW+120)-60;
                fillRect(gx,sy+sh*0.4f,60,7,alpha({190,155,75,255},80));
            }
        }
    }
}

void SquareJumpGame::drawAutumnBackground() {
    drawGradientVertical(currentTheme.bg1,currentTheme.bg2);
    fillCircle(screenW*0.75f,screenH*0.12f,55,alpha({255,235,59,255},200));
    fillCircle(screenW*0.75f,screenH*0.12f,42,alpha({255,255,200,255},220));
    for(int i=0;i<3;i++) {
        float r=65+i*20+std::sin(ticks*0.04f+i)*5;
        fillCircle(screenW*0.75f,screenH*0.12f,r,alpha({255,193,7,255},static_cast<Uint8>(15-i*4)));
    }
    for(int i=0;i<6;i++) {
        float tx=i*screenW*0.18f-std::fmod(camX*0.05f,screenW*0.18f), ty=static_cast<float>(screenH)-80;
        fillRect(tx-4,ty-80,8,80,alpha({30,15,5,255},180));
        fillCircle(tx,ty-80,35,alpha({60,30,10,255},160));
    }
    drawParallaxStars(levelData.stars,std::max(levelData.worldW,static_cast<float>(screenW)),std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawWinterBackground() {
    drawGradientVertical(currentTheme.bg1,currentTheme.bg2);
    fillCircle(screenW*0.5f,screenH*0.10f,40,alpha({200,220,255,255},160));
    for(int i=0;i<5;i++) {
        float hx=i*screenW*0.24f-std::fmod(camX*0.04f,screenW*0.24f);
        fillCircle(hx,screenH*0.88f,140+i*15,alpha({200,215,235,255},60));
    }
    drawParallaxStars(levelData.stars,std::max(levelData.worldW,static_cast<float>(screenW)),std::max(levelData.worldH,static_cast<float>(screenH)));
}

void SquareJumpGame::drawDesertBackground() {
    drawGradientVertical(currentTheme.bg1,currentTheme.bg2);
    float sunX=screenW*0.85f, sunY=screenH*0.12f;
    for(int i=4;i>=0;i--) {
        float r=50+i*18+std::sin(ticks*0.05f)*4;
        fillCircle(sunX,sunY,r,alpha({255,200,50,255},static_cast<Uint8>(40-i*7)));
    }
    fillCircle(sunX,sunY,48,alpha({255,240,120,255},240));
    for(int i=0;i<5;i++) {
        float dx=i*screenW*0.25f-std::fmod(camX*0.07f,screenW*0.25f);
        fillCircle(dx,static_cast<float>(screenH),220+i*30,alpha({180,140,70,255},static_cast<Uint8>(80+i*10)));
    }
}

void SquareJumpGame::drawWorldBackground() {
    if(levelData.isSpring||state==GameState::Market) { drawSpringBackdrop(); return; }
    switch(levelData.season) {
        case SEASON_SUMMER: drawSummerBackground(); break;
        case SEASON_AUTUMN: drawAutumnBackground(); break;
        case SEASON_WINTER: drawWinterBackground(); break;
        case SEASON_DESERT: drawDesertBackground(); break;
        default:
            drawGradientVertical(currentTheme.bg1,currentTheme.bg2);
            drawParallaxStars(levelData.stars,std::max(levelData.worldW,static_cast<float>(screenW)),std::max(levelData.worldH,static_cast<float>(screenH)));
            break;
    }
}

void SquareJumpGame::drawWinterEffect() {
    float pulse=0.5f+0.5f*std::sin(ticks*0.03f);
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),
             alpha({200,220,255,255},static_cast<Uint8>(WINTER_VISIBILITY*255*(0.6f+0.4f*pulse))));
}

void SquareJumpGame::drawDesertHeat() {
    float groundScreenY=(levelData.worldH-44)-camY;
    for(int i=0;i<6;i++) {
        float shimY=groundScreenY-i*12-std::sin(ticks*0.08f+i)*4;
        if(shimY<0||shimY>screenH) continue;
        fillRect(0,shimY,static_cast<float>(screenW),2,alpha({255,200,100,255},static_cast<Uint8>(20-i*3)));
    }
}

void SquareJumpGame::drawTree(float x,float y,float len,float angleDeg,float branchW,int depth) {
    float rad=angleDeg*PI/180, x2=x+std::sin(rad)*len, y2=y-std::cos(rad)*len;
    drawThickLine(x,y,x2,y2,branchW,{93,64,55,255});
    if(depth>=6||len<12) {
        fillCircle(x2,y2,4.5f,currentTheme.petalA);
        fillCircle(x2+4,y2+1,3.5f,currentTheme.petalB);
        return;
    }
    drawTree(x2,y2,len*0.75f,angleDeg-20,branchW*0.72f,depth+1);
    drawTree(x2,y2,len*0.75f,angleDeg+20,branchW*0.72f,depth+1);
}

void SquareJumpGame::drawPetal(const Petal& p,bool screenSpace) {
    float depth=lerpf(0.45f,1.35f,p.z), x=p.x, y=p.y;
    if(!screenSpace){x-=camX*depth*0.05f;y-=camY*depth*0.05f;}
    SDL_Color c=blend(currentTheme.petalA,currentTheme.petalB,std::sin((ticks+p.angle)*0.02f)*0.5f+0.5f);
    c.a=static_cast<Uint8>(120+p.z*110);
    float r=p.size*depth;
    fillCircle(x,y,r,alpha(c,c.a));
    fillCircle(x+r*0.6f,y-r*0.2f,r*0.55f,alpha(currentTheme.petalB,static_cast<Uint8>(c.a*0.8f)));
}

void SquareJumpGame::drawMetalPlatform(const Platform& pf,float x,float y,float w,float h) {
    fillRect(x,y,w,h,currentTheme.platformBody);
    fillRect(x,y,w,2,currentTheme.platformTop); fillRect(x,y,2,h,currentTheme.platformTop);
    fillRect(x,y+h-2,w,2,currentTheme.platformShadow); fillRect(x+w-2,y,2,h,currentTheme.platformShadow);
    int ss=static_cast<int>(std::max(20.0f,std::floor((camX-pf.x)/35)*35+20));
    for(int local=ss;local<pf.w-10;local+=35) {
        float sx=x+local;
        fillRect(sx,y,2,h,{26,32,44,255});
        fillCircle(sx+1,y+5,1.5f,{113,128,150,255});
        fillCircle(sx+1,y+h-5,1.5f,{113,128,150,255});
    }
}

void SquareJumpGame::drawSpringPlatform(float x,float y,float w,float h,bool isGround) {
    if(isGround) {
        fillRect(x,y,w,h,{93,64,55,255}); fillRect(x,y,w,12,{76,175,80,255});
        for(float stripe=12;stripe<w;stripe+=22) fillRect(x+stripe,y+8,2,h-8,alpha({121,85,72,255},90));
    } else {
        fillRect(x,y,w,h,currentTheme.platformBody);
        fillRect(x,y,w,3,currentTheme.platformTop);
        fillRect(x+w*0.33f,y,2,h,currentTheme.platformTop);
        fillRect(x+w*0.66f,y,2,h,currentTheme.platformTop);
        fillRect(x,y+h*0.5f-1,w,2,alpha(currentTheme.platformTop,160));
    }
}

void SquareJumpGame::drawSandGround(float sx,float sy,float w,float h) {
    fillRect(sx,sy,w,h,{210,180,100,255});
    fillRect(sx,sy,w,8,{225,200,130,255});
    for(int i=0;i<static_cast<int>(w/45)+1;i++) {
        float gx=sx+i*45+std::sin(static_cast<float>(i)*2.1f)*9;
        fillCircle(gx,sy+h*0.45f,3+std::sin(static_cast<float>(i))*1.5f,alpha({185,148,70,255},110));
    }
}

void SquareJumpGame::drawBuoyPlatform(const Platform& pf,float sx,float sy) {
    if(pf.buoyGone) return;
    float cx=sx+pf.w*0.5f;
    float bob=pf.buoyActivated ? 0.0f : std::sin(ticks*0.06f+pf.buoyBobOffset)*4.0f;
    float topY=sy-bob;

    for(int i=3;i>=0;i--)
        fillCircle(cx,sy+pf.h*0.5f,pf.w*0.5f+i*2,alpha({0,80,160,255},static_cast<Uint8>(25-i*7)));
    fillCircle(cx,sy+pf.h*0.5f,pf.w*0.5f,{255,140,0,255});
    fillCircle(cx,sy+pf.h*0.5f,pf.w*0.42f,{255,170,30,255});

    fillRect(sx+3,sy+pf.h*0.5f-3,pf.w-6,6,{220,30,30,255});
    fillRect(cx-3,topY,6,pf.h,{220,30,30,255});

    fillRoundedRect(sx+5,topY,pf.w-10,10,5,alpha({255,255,255,255},200));

    if(pf.buoyActivated) {
        drawGlowRect(sx,topY,pf.w,pf.h,{255,200,50,255},4,12);
        return;
    }

    float pdx=(player.x+player.width*0.5f)-(pf.x+pf.w*0.5f);
    float pdy=(player.y+player.height*0.5f)-(pf.y+pf.h*0.5f);
    bool playerOnThis=(player.ridingBuoyIndex>=0);
    if(!playerOnThis && pdx*pdx+pdy*pdy<200*200)
        drawDialogueBubble(cx,topY,"Bấm Q để tháo van",{255,255,220,255});
}

void SquareJumpGame::drawMooncakePlatform(const Platform& pf,float sx,float sy) {
    if(!pf.mooncakeActive) {
        fillRect(sx,sy,pf.w,pf.h,alpha({121,85,72,255},120));
        drawText(sx+pf.w*0.5f-24,sy-20,"Đã ăn hết",{150,100,50,255},1.5f);
        return;
    }
    fillRect(sx,sy,pf.w,pf.h,{183,128,64,255});
    fillRect(sx+2,sy,pf.w-4,5,{230,180,90,255});
    for(float lx=10;lx<pf.w-5;lx+=14) fillRect(sx+lx,sy,2,pf.h,alpha({150,100,40,255},100));
    drawText(sx+4,sy+4,std::to_string(pf.mooncakeBites)+" miếng",{255,235,59,255},1.5f);
    float pdx=(player.x+player.width*0.5f)-(pf.x+pf.w*0.5f);
    float pdy=(player.y+player.height*0.5f)-(pf.y+pf.h*0.5f);
    if(pdx*pdx+pdy*pdy<130*130) drawDialogueBubble(sx+pf.w*0.5f,sy,"[E] Ăn bánh",{255,235,59,255});
}

void SquareJumpGame::drawPlatforms() {
    for(const Platform& pf:levelData.platforms) {
        if(pf.buoyGone) continue;
        float x=pf.x-camX, y=pf.y-camY;
        if(x+pf.w<0||x>screenW||y+pf.h<0||y>screenH) continue;

        if(pf.isMooncake)                               drawMooncakePlatform(pf,x,y);
        else if(pf.isBuoy)                              drawBuoyPlatform(pf,x,y);
        else if(pf.isGround&&levelData.season==SEASON_SUMMER) drawSandGround(x,y,pf.w,pf.h);
        else if(levelData.isSpring||state==GameState::Market)  drawSpringPlatform(x,y,pf.w,pf.h,pf.isGround);
        else if(levelData.season==SEASON_WINTER) {
            drawMetalPlatform(pf,x,y,pf.w,pf.h);
            fillRect(x,y,pf.w,3,alpha({200,230,255,255},180));
        } else drawMetalPlatform(pf,x,y,pf.w,pf.h);
    }
}

void SquareJumpGame::drawWaterZones() {
    for(const WaterZone& wz:levelData.waterZones) {
        float x=wz.x-camX, y=wz.y-camY;
        if(x+wz.w<0||x>screenW||y+wz.h<0||y>screenH) continue;
        SDL_Color wc=wz.isDangerous?SDL_Color{180,30,30,255}:SDL_Color{30,100,200,255};
        fillRect(x,y,wz.w,wz.h,alpha(wc,160));
        for(int i=0;i<4;i++) {
            float wy=y+i*16+std::sin(ticks*0.05f+i)*4;
            if(wy<y||wy>y+wz.h) continue;
            fillRect(x,wy,wz.w,3,alpha({255,255,255,255},40));
        }
        if(wz.isDangerous) drawText(x+10,y+8,"Nguy hiểm!",{255,100,100,255},1.5f);
    }
}

void SquareJumpGame::drawSnowZones() {
    for(const SnowZone& sz:levelData.snowZones) {
        float x=sz.x-camX, y=sz.y-camY;
        if(x+sz.w<0||x>screenW||y+sz.h<0||y>screenH) continue;
        SDL_Color sc=sz.isDamaging?SDL_Color{150,100,200,255}:SDL_Color{200,220,255,255};
        fillRect(x,y,sz.w,sz.h,alpha(sc,60));
        setColor(alpha(sc,120)); SDL_FRect br={x,y,sz.w,sz.h}; SDL_RenderRect(renderer,&br);
        drawText(x+4,y+4,sz.isDamaging?"Bão tuyết":"Tuyết chậm",sz.isDamaging?SDL_Color{200,150,255,255}:SDL_Color{180,210,255,255},1.5f);
    }
}

void SquareJumpGame::drawSpringNpc() {
    if(!levelData.isSpring||!levelData.hasSpringNpc) return;
    const SpringNpc& npc=levelData.springNpc;
    float x=npc.x-camX, y=npc.y-camY;
    float bob=std::sin(ticks*0.06f)*2.5f;
    y+=bob;

    fillRect(x+10,y+2,12,4,{139,90,43,255});
    fillRect(x+8,y,16,2,{180,120,50,255});
    fillCircle(x+16,y+10,10,{255,224,178,255});
    fillRect(x+10,y+18,14,4,{255,235,59,255});
    fillRoundedRect(x+6,y+22,20,24,6,{211,47,47,255});

    bool giving=(npc.animFrame==1||npc.animFrame==3);
    if(!giving) {
        fillRect(x+2,y+24,5,18,{255,224,178,255});
        fillRect(x+23,y+24,5,18,{255,224,178,255});
    } else {
        fillRect(x+2,y+24,5,14,{255,224,178,255});
        float armAng=std::sin(ticks*0.1f)*10.0f;
        float ax=x+28+std::cos((armAng+40)*PI/180)*15;
        float ay=y+26+std::sin((armAng+40)*PI/180)*15;
        drawThickLine(x+26,y+26,ax,ay,5,{255,224,178,255});
        if(!npc.gifted) {
            fillRoundedRect(ax-4,ay-5,10,12,3,{255,193,7,255});
            fillRect(ax-1,ay-8,4,4,{255,152,0,255});
        }
    }
    fillRect(x+10,y+46,5,8,{46,125,50,255});
    fillRect(x+19,y+46,5,8,{46,125,50,255});
    fillRoundedRect(x+8,y+10,5,7,2,{255,255,255,255}); fillCircle(x+11,y+15,1.5f,{0,0,0,255});
    fillRoundedRect(x+19,y+10,5,7,2,{255,255,255,255}); fillCircle(x+22,y+15,1.5f,{0,0,0,255});
    fillRoundedRect(x+12,y+20,8,3,1,{255,255,255,255});

    static const char* NPC_DLGS[]={"Biết chú là ai không?","Có bạn gái chưa?","Học hành thế nào?","Làm ở đâu vậy?","Lương bao nhiêu?","Chúc mừng năm mới!","Bao giờ cưới?","Tết về quê không?",nullptr};
    if(npc.talkActive&&NPC_DLGS[npc.dialogueIndex])
        drawDialogueBubble(x+16,y,NPC_DLGS[npc.dialogueIndex],{255,235,59,255});
    else if(!npc.gifted)
        drawDialogueBubble(x+16,y,"Lì xì +"+std::to_string(npc.reward)+"k",{255,235,200,255});
    else
        drawDialogueBubble(x+16,y,"Chúc mừng!",{180,255,180,255});
}

void SquareJumpGame::drawChildNpcs() {
    static const char* CHILD_DLGS[]={"Cho con xin lì xì!","Chú ơi lì xì đi!","Con chúc chú năm mới!","Lì xì đi chú ơi!",nullptr};
    static const char* ANGRY_DLGS[]={"Không có à?!","Lấy tiền ra đây!","Đá cho biết tay!",nullptr};

    for(const ChildNpc& child:levelData.childNpcs) {
        if(!child.active) continue;
        float x=child.x-camX, y=child.y-camY;
        float bob=child.angered?0.0f:std::sin(ticks*0.08f+child.x*0.01f)*1.5f;
        y+=bob;

        fillCircle(x+14,y+9,8,{255,224,178,255});
        SDL_Color bodyC=child.angered?SDL_Color{220,50,50,255}:SDL_Color{100,181,246,255};
        fillRoundedRect(x+6,y+17,16,20,5,bodyC);

        if(!child.angered) {
            if(child.animFrame==1) {
                fillRect(x+2,y+20,5,14,{255,224,178,255});
                float pa=std::sin(ticks*0.12f)*15.0f;
                float px=x+26+std::cos((-30+pa)*PI/180)*12;
                float py=y+22+std::sin((-30+pa)*PI/180)*12;
                drawThickLine(x+24,y+22,px,py,4,{255,224,178,255});
            } else {
                fillRect(x+2,y+20,5,14,{255,224,178,255});
                fillRect(x+21,y+20,5,14,{255,224,178,255});
            }
            fillRect(x+8,y+37,5,8,{33,33,120,255});
            fillRect(x+15,y+37,5,8,{33,33,120,255});
        } else {
            fillRect(x+2,y+20,5,14,{255,224,178,255});
            float kickAng=(child.kickFrame*28.0f)-20.0f;
            float legEndX=x+14+std::cos(kickAng*PI/180)*20;
            float legEndY=y+37+std::sin(kickAng*PI/180)*20;
            drawThickLine(x+14,y+37,legEndX,legEndY,7,{33,33,120,255});
            fillCircle(legEndX,legEndY,5,{200,100,50,255});
            fillRect(x+15,y+37,5,8,{33,33,120,255});
        }

        if(child.angered) {
            fillRect(x+7,y+5,5,2,{0,0,0,255}); fillRect(x+14,y+5,5,2,{0,0,0,255});
            fillRect(x+7,y+13,10,2,{0,0,0,255});
        } else {
            fillCircle(x+9,y+8,2.5f,{0,0,0,255}); fillCircle(x+18,y+8,2.5f,{0,0,0,255});
            fillRoundedRect(x+9,y+13,9,2,1,{0,0,0,255});
        }

        const char* dlg=child.angered
            ? ANGRY_DLGS[child.dialogueIndex%3]
            : CHILD_DLGS[child.dialogueIndex%4];
        if(dlg) drawDialogueBubble(x+14,y,dlg,child.angered?SDL_Color{255,180,180,255}:SDL_Color{220,240,255,255});
    }
}

void SquareJumpGame::drawStalls() {
    if(state!=GameState::Market) return;
    for(const Stall& st:levelData.stalls) {
        float x=st.x-camX, y=st.y-camY;
        fillRect(x,y,st.w,st.h,{141,110,99,255});
        fillRect(x-10,y-20,st.w+20,20,{216,67,21,255});
        drawGlowRect(x-6,y-12,st.w+12,st.h+12,{255,235,59,255},2,4);
        if(st.bought) drawText(x+8,y+18,"Đã mua",{255,255,255,255},1.5f);
        else {
            drawText(x+8,y+10,std::to_string(st.cost)+" lì xì",{255,255,255,255},1.5f);
            drawText(x+8,y+30,st.name,{255,235,59,255},1.5f);
        }
    }
}

void SquareJumpGame::drawBoss() {
    if(!levelData.hasBoss||!levelData.boss.active) return;
    const Boss& b=levelData.boss;
    float x=b.x-camX, y=b.y-camY;
    if(x+b.w<-20||x>screenW+20||y+b.h<-20||y>screenH+20) return;
    SDL_Color gc=(b.state==BossState::Rage)?SDL_Color{255,23,68,255}
               :(b.state==BossState::Attacking)?SDL_Color{255,152,0,255}
               :(b.state==BossState::Waking)?SDL_Color{255,235,59,255}
               :SDL_Color{100,200,100,255};
    drawGlowRect(x,y,b.w,b.h,gc,4,12);
    fillRoundedRect(x+4,y+20,b.w-8,b.h-20,10,{50,20,10,255});
    float hx=x+b.w*0.5f, hy=y+18;
    fillCircle(hx,hy,26,{200,100,20,255});
    for(int i=0;i<8;i++) {
        float ma=i*PI*2/8+ticks*0.02f;
        fillCircle(hx+std::cos(ma)*26,hy+std::sin(ma)*24,10,
                   blend({200,100,20,255},{255,152,0,255},std::sin(ticks*0.05f+i)*0.5f+0.5f));
    }
    fillCircle(hx,hy,18,{230,140,40,255});
    bool sl=(b.state==BossState::Sleeping);
    if(sl) { fillRect(hx-10,hy-4,9,2,{0,0,0,255}); fillRect(hx+2,hy-4,9,2,{0,0,0,255}); }
    else {
        fillCircle(hx-7,hy-4,4,{255,255,255,255}); fillCircle(hx+7,hy-4,4,{255,255,255,255});
        SDL_Color pupil=(b.state==BossState::Rage)?SDL_Color{255,23,68,255}:SDL_Color{0,0,0,255};
        fillCircle(hx-7,hy-4,2,pupil); fillCircle(hx+7,hy-4,2,pupil);
    }
    fillRoundedRect(hx-8,hy+6,16,5,2,{50,10,5,255});
    if(b.state==BossState::Attacking||b.state==BossState::Rage) {
        float fs=0.6f+0.4f*std::sin(ticks*0.2f);
        for(int fi=0;fi<4;fi++) {
            float fr=8+fi*5+fs*4, fx=hx+(b.facingRight?fr*2:-fr*2);
            fillCircle(fx,hy+6,fr,alpha(blend({255,152,0,255},{255,50,0,255},static_cast<float>(fi)/4),static_cast<Uint8>(180-fi*35)));
        }
    }
    const char* sn="Zzz";
    if(b.state==BossState::Waking)    sn="!";
    if(b.state==BossState::Attacking) sn="LỬA!";
    if(b.state==BossState::Rage)      sn="ĐIÊN!";
    drawText(x+b.w*0.5f-20,y-24,sn,b.state==BossState::Rage?SDL_Color{255,23,68,255}:SDL_Color{255,235,59,255},1.5f);
}

void SquareJumpGame::drawFireballs() {
    for(const Fireball& f:levelData.fireballs) {
        float sx=f.x-camX, sy=f.y-camY;
        for(int i=3;i>=0;i--) {
            float r=10-i*2+std::sin(ticks*0.3f)*1.5f;
            fillCircle(sx,sy,r,alpha(blend({255,152,0,255},{255,50,0,255},static_cast<float>(i)/3),static_cast<Uint8>(200-i*40)));
        }
    }
}

void SquareJumpGame::drawSpinyLeaves() {
    for(const SpinyLeaf& sl:levelData.spinyLeaves) {
        float sx=sl.x-camX, sy=sl.y-camY;
        if(sx<-20||sx>screenW+20||sy<-20||sy>screenH+20) continue;
        SDL_Color lc=blend({180,100,20,255},{220,60,20,255},std::sin(sl.angle*0.05f)*0.5f+0.5f);
        fillCircle(sx,sy,6,lc);
        for(int i=0;i<4;i++) {
            float sa=(sl.angle+i*90)*PI/180;
            fillCircle(sx+std::cos(sa)*8,sy+std::sin(sa)*8,3,alpha({255,100,30,255},200));
        }
    }
}

void SquareJumpGame::drawCamels() {
    for(const Camel& c:levelData.camels) {
        if(!c.active) continue;
        float x=c.x-camX, y=c.y-camY;
        bool flip=!c.facingRight;
        fillRoundedRect(x+8,y-40,50,30,12,{180,140,80,255});
        fillCircle(x+20,y-50,14,{160,120,60,255}); fillCircle(x+38,y-50,14,{160,120,60,255});
        float hx=flip?x+10:x+48;
        fillCircle(hx,y-42,12,{200,160,90,255}); fillCircle(hx+(flip?-5:5),y-46,3,{0,0,0,255});
        for(int i=0;i<4;i++) {
            float lx=x+12+i*12, bob=std::sin(ticks*0.12f+i*0.7f)*3;
            fillRect(lx,y-14+bob,6,18,{150,110,55,255});
            fillRect(lx-1,y+4+bob,8,5,{120,90,40,255});
        }
        if(c.waterLevel>0) {
            fillRoundedRect(x+8,y-60,50,5,2,alpha({100,180,255,255},100));
            fillRoundedRect(x+8,y-60,50*(c.waterLevel/100.0f),5,2,{30,150,255,255});
        }
        if(c.waterLevel>=CAMEL_WATER_NEED) drawGlowRect(x+8,y-40,50,30,{30,150,255,255},2,6);
        float pdx=(player.x+player.width*0.5f)-(c.x+30);
        float pdy=(player.y+player.height*0.5f)-(c.y-20);
        if(pdx*pdx+pdy*pdy<130*130&&c.waterLevel<CAMEL_WATER_NEED)
            drawDialogueBubble(x+33,y-42,"Cần nước",{100,200,255,255});
    }
}

void SquareJumpGame::drawOases() {
    for(const Oasis& o:levelData.oases) {
        float x=o.x-camX, y=o.y-camY;
        if(o.depleted) {
            fillRect(x,y,o.w,o.h,alpha({120,90,50,255},100));
            drawText(x+4,y-20,"Khô cạn",{150,100,50,255},1.5f);
            continue;
        }
        fillRect(x,y,o.w,o.h,alpha({30,120,200,255},200));
        fillRect(x+2,y+2,o.w*0.6f,4,alpha({255,255,255,255},60));
        if(o.beingDug) {
            float prog=static_cast<float>(o.digTimer)/OASIS_DIG_TIME;
            fillRoundedRect(x,y-10,o.w,6,3,alpha({255,255,255,255},80));
            fillRoundedRect(x,y-10,o.w*prog,6,3,{30,200,255,255});
        }
        float pdx=(player.x+player.width*0.5f)-(o.x+o.w*0.5f);
        float pdy=(player.y+player.height*0.5f)-(o.y+o.h*0.5f);
        if(pdx*pdx+pdy*pdy<120*120)
            drawDialogueBubble(x+o.w*0.5f,y,upgrades.hasShovel?"[E] Đào nước":"Cần xẻng",{30,200,255,255});
    }
}

void SquareJumpGame::drawCheckpoints() {
    for(const Checkpoint& cp:levelData.checkpoints) {
        float x=cp.x-camX, y=cp.y-camY;
        if(x<-30||x>screenW+30||y<-60||y>screenH+30) continue;
        SDL_Color fc=cp.reached?SDL_Color{255,235,59,255}:SDL_Color{255,255,255,180};
        drawThickLine(x,y,x,y-40,2,fc);
        for(int fy=0;fy<3;fy++) {
            float wave=std::sin(ticks*0.1f+fy*0.5f)*3;
            fillRect(x+1,y-40+fy*7+wave,18,6,fc);
        }
        if(cp.reached) drawGlowRect(x-8,y-48,20,48,{255,235,59,255},3,8);
    }
}

void SquareJumpGame::drawParticles() {
    for(const Particle& p:particles) {
        float ar=p.maxLife>0?p.life/p.maxLife:0;
        SDL_Color c=alpha(p.color,static_cast<Uint8>(255*clampf(ar,0,1)));
        fillCircle(p.x-camX,p.y-camY,p.size,c);
    }
}

void SquareJumpGame::drawGate() {
    float x=levelData.gate.x-camX, y=levelData.gate.y-camY;
    float glow=18+std::sin(ticks*0.1f)*8;
    drawGlowRect(x-6,y-6,levelData.gate.w+12,levelData.gate.h+12,currentTheme.gate,5,glow*0.35f);
    fillRoundedRect(x+10,y+10,levelData.gate.w-20,levelData.gate.h-20,10,
                    alpha(currentTheme.gate,static_cast<Uint8>(180+std::sin(ticks*0.1f)*35)));
    fillRoundedRect(x+15,y+15,levelData.gate.w-30,levelData.gate.h-30,8,{255,255,255,235});
    float aY=y+levelData.gate.h*0.5f-8;
    fillRect(x+levelData.gate.w*0.5f-2,aY,4,16,{0,0,0,200});
    fillRect(x+levelData.gate.w*0.5f-8,aY-8,17,4,{0,0,0,200});
}

void SquareJumpGame::renderPlayerTexture() {
    SDL_SetRenderTarget(renderer,playerTexture);
    SDL_SetRenderDrawColor(renderer,0,0,0,0); SDL_RenderClear(renderer);
    bool springOutfit=levelData.isSpring||state==GameState::Market;
    bool chargeFace=player.charging;
    bool rightClosed=chargeFace||(player.blinkTimer>0&&player.blinkRight);
    bool leftClosed=player.blinkTimer>0&&!player.blinkRight;
    SDL_Color pAcc=(player.damageFlashTimer>0&&(player.damageFlashTimer/3)%2==0)?SDL_Color{255,50,50,255}:currentTheme.playerAccent;
    SDL_Color pBody=(player.invincible&&(ticks/4)%2==0)?SDL_Color{200,200,255,255}:currentTheme.playerPrimary;
    fillRoundedRect(2,2,32,32,9,pAcc); fillRoundedRect(4,4,28,28,8,pBody);
    fillRoundedRect(6,6,24,24,7,alpha(blend(pBody,{255,255,255,255},0.06f),255));
    if(springOutfit) {
        fillRoundedRect(8,18,20,10,4,{255,235,59,255}); fillRect(16.5f,18,3,10,{211,47,47,255});
        fillRect(11,20,2,6,{255,193,7,255}); fillRect(23,20,2,6,{255,193,7,255}); fillRect(12,15,12,3,{255,248,225,255});
    }
    if(!leftClosed){fillRoundedRect(9,10,6,8,2,{255,255,255,255});fillCircle(12,16,1.5f,{0,0,0,255});}
    else fillRect(9,15,6,2,{255,255,255,255});
    if(!rightClosed){fillRoundedRect(21,10,6,8,2,{255,255,255,255});fillCircle(24,16,1.5f,{0,0,0,255});}
    else fillRect(21,15,6,2,{255,255,255,255});
    if(chargeFace) fillCircle(18,24,2.5f,{255,255,255,255});
    else fillRoundedRect(14,23,8,3,1.5f,{255,255,255,255});
    SDL_SetRenderTarget(renderer,nullptr);
}

void SquareJumpGame::drawAimDots() {
    if(!player.charging) return;
    int effCharge=upgrades.effectiveMaxCharge();
    float ratio=static_cast<float>(player.chargeTime)/effCharge;
    float mwx=mouseScreenX+camX, mwy=mouseScreenY+camY;
    float ang=std::atan2(mwy-(player.y+player.height*0.5f),mwx-(player.x+player.width*0.5f));
    float stX=player.x+player.width*0.5f-camX, stY=player.y+player.height*0.5f-camY;
    float dist=80+ratio*250;
    SDL_Color dc=player.chargeTime>=effCharge?currentTheme.gate:SDL_Color{255,255,255,255};
    for(int i=1;i<=12;i++) {
        float t=static_cast<float>(i)/12.0f;
        float px=stX+std::cos(ang)*dist*t, py=stY+std::sin(ang)*dist*t;
        fillCircle(px,py,2.5f-t,alpha(dc,static_cast<Uint8>((1-t)*255)));
    }
}

void SquareJumpGame::drawPlayer() {
    if(player.charging) {
        float pulse=0.5f+0.5f*std::sin(ticks*0.24f);
        fillCircle(player.x-camX+player.width*0.5f,player.y-camY+player.height*0.5f,24+pulse*6,
                   alpha(player.chargeTime>=upgrades.effectiveMaxCharge()?currentTheme.gate:currentTheme.playerAccent,50));
    }
    if(player.invincible)
        fillCircle(player.x-camX+player.width*0.5f,player.y-camY+player.height*0.5f,22+std::sin(ticks*0.3f)*4,alpha({200,200,255,255},60));

    if(spriteSheet) {
        int frame=player.charging?player.chargeAnimFrame:0;
        float sw=0,sh=0; SDL_GetTextureSize(spriteSheet,&sw,&sh);
        float fw=sw*0.5f, fh=sh*0.5f;
        int col=frame%2, row=frame/2;
        SDL_FRect src={col*fw,row*fh,fw,fh};
        float ang=clampf(player.vx*2,-22,22);
        SDL_FRect dst={player.x-camX-4,player.y-camY-4,static_cast<float>(PLAYER_TEXTURE_SIZE)+2,static_cast<float>(PLAYER_TEXTURE_SIZE)+2};
        SDL_RenderTextureRotated(renderer,spriteSheet,&src,&dst,ang,nullptr,SDL_FLIP_NONE);
    } else {
        renderPlayerTexture();
        float ang=clampf(player.vx*2,-22,22);
        SDL_FRect dst={player.x-camX-4,player.y-camY-4,static_cast<float>(PLAYER_TEXTURE_SIZE)+2,static_cast<float>(PLAYER_TEXTURE_SIZE)+2};
        SDL_RenderTextureRotated(renderer,playerTexture,nullptr,&dst,ang,nullptr,SDL_FLIP_NONE);
    }
}

void SquareJumpGame::drawHealthBar() {
    float ratio=static_cast<float>(player.health)/player.maxHealth;
    SDL_Color hc=ratio>0.5f?SDL_Color{76,175,80,255}:ratio>0.25f?SDL_Color{255,193,7,255}:SDL_Color{244,67,54,255};
    fillRoundedRect(screenW-185,8,164,14,7,alpha({0,0,0,255},120));
    fillRoundedRect(screenW-185,8,164*ratio,14,7,hc);
    drawText(screenW-182,10,std::to_string(player.health)+"/"+std::to_string(player.maxHealth),{255,255,255,255},1.5f);
}

void SquareJumpGame::drawStatBars() {
    float panelX=8, panelY=60;
    int season=getSeasonFromLevel(currentLevel);
    auto drawBar=[&](float by,float val,float maxVal,const SDL_Color& fc,const std::string& label) {
        fillRoundedRect(panelX,by,150,14,7,alpha({0,0,0,255},110));
        fillRoundedRect(panelX,by,150*(val/maxVal),14,7,fc);
        drawText(panelX+4,by+2,label,{255,255,255,255},1.5f);
    };
    if(season==SEASON_SUMMER) {
        drawBar(panelY,HEAT_MAX-player.heat,HEAT_MAX,{30,150,255,255},"Mát mẻ");
        drawBar(panelY+20,100-player.waterSubmergedTicks*(100.0f/WATER_SUBMERSION_MAX),100,{30,200,100,255},"Hơi thở");
    }
    if(season==SEASON_AUTUMN) {
        drawBar(panelY,player.hunger,HUNGER_MAX,{255,152,0,255},"No bụng");
        if(player.invincible) {
            Uint64 el=ticks-player.lastInvincibleTick;
            float rem=1.0f-static_cast<float>(el)/INVINCIBLE_DURATION;
            drawBar(panelY+20,rem*100,100,{200,200,255,255},"Khiên");
        }
    }
    if(season==SEASON_DESERT) {
        drawBar(panelY,player.thirst,THIRST_MAX,{30,150,255,255},"Khát");
        drawBar(panelY+20,static_cast<float>(player.jumpsLeft),DESERT_JUMP_MAX,{255,235,59,255},"Lần nhảy");
        if(player.camelOasisWater>0) drawBar(panelY+40,player.camelOasisWater,100,{100,200,255,255},"Nước mang");
    }
}

void SquareJumpGame::drawHud() {
    fillRect(0,0,static_cast<float>(screenW),52,alpha({0,0,0,255},128));
    const char* seasonLabel="";
    switch(getSeasonFromLevel(currentLevel)) {
        case SEASON_SPRING: seasonLabel="Mùa Xuân"; break;
        case SEASON_SUMMER: seasonLabel="Mùa Hè";   break;
        case SEASON_AUTUMN: seasonLabel="Mùa Thu";  break;
        case SEASON_WINTER: seasonLabel="Mùa Đông"; break;
        case SEASON_DESERT: seasonLabel="Sa Mạc";   break;
    }
    if(state==GameState::Market) drawText(20,16,"Chợ Tết",{255,255,255,255},2.0f);
    else drawText(20,16,std::string(seasonLabel)+" - Lv."+std::to_string(currentLevel),{255,255,255,255},2.0f);

    drawCenteredText(screenW*0.5f,16,"Lì xì: "+std::to_string(lixiCount),{255,235,59,255},2.0f);

    if(player.charging) {
        int effCharge=upgrades.effectiveMaxCharge();
        float cr=static_cast<float>(player.chargeTime)/effCharge;
        float bx=player.x-camX+player.width*0.5f-44, by=player.y-camY-22;
        fillRoundedRect(bx,by,88,10,5,alpha({0,0,0,255},160));
        fillRoundedRect(bx,by,88*cr,10,5,cr>=1.0f?currentTheme.gate:blend({100,200,255,255},{255,214,0,255},cr));
    }

    drawHealthBar();
    drawStatBars();
    drawText(screenW-220,36,"[P]=Cửa hàng  [ESC]=Tạm dừng",{200,200,200,255},1.5f);
}

void SquareJumpGame::drawButton(const SDL_FRect& r,const std::string& label,bool hover,const SDL_Color& fill) {
    SDL_Color base=hover?blend(fill,{255,255,255,255},0.22f):fill;
    float expand=hover?4.0f:0.0f;
    drawGlowRect(r.x-expand,r.y-expand,r.w+expand*2,r.h+expand*2,base,3,8);
    fillRoundedRect(r.x-expand,r.y-expand,r.w+expand*2,r.h+expand*2,12,base);
    drawCenteredText(r.x+r.w*0.5f,r.y+16-expand,label,{0,0,0,255},2.0f);
}

void SquareJumpGame::drawMenu() {
    drawMenuBackground();
    drawCenteredText(screenW*0.5f,screenH*0.5f-170,"SQUARE JUMP",{255,255,255,255},4.0f);

    SDL_FRect primary=menuPrimaryBtnRect();
    SDL_FRect ng=menuNewGameBtnRect();
    SDL_FRect sh=menuShopBtnRect();
    SDL_FRect lv=menuLevelBtnRect();
    SDL_FRect ex=menuExitBtnRect();

    if(hasSave) {
        drawButton(primary,"Tiếp tục",  pointInRect(mouseScreenX,mouseScreenY,primary),{0,188,212,255});
        drawButton(ng,     "Chơi mới",  pointInRect(mouseScreenX,mouseScreenY,ng),     {0,230,118,255});
    } else {
        drawButton(primary,"Bắt đầu",   pointInRect(mouseScreenX,mouseScreenY,primary),{0,230,118,255});
    }
    drawButton(sh,"Cửa hàng",        pointInRect(mouseScreenX,mouseScreenY,sh),{255,193,7,255});
    drawButton(lv,"Chọn màn",        pointInRect(mouseScreenX,mouseScreenY,lv),{142,45,226,255});
    drawButton(ex,"Thoát",           pointInRect(mouseScreenX,mouseScreenY,ex),{255,112,67,255});
}

void SquareJumpGame::drawPauseMenu() {
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},185));
    float pw=420, ph=320;
    float px=screenW*0.5f-pw*0.5f, py=screenH*0.5f-ph*0.5f-20;
    fillRoundedRect(px,py,pw,ph,20,alpha({10,15,30,255},235));
    drawGlowRect(px,py,pw,ph,{0,230,118,255},3,12);
    drawCenteredText(screenW*0.5f,py+22,"Tạm Dừng",{255,255,255,255},3.0f);
    drawButton(pauseResumeBtnRect(), "Tiếp tục",  pointInRect(mouseScreenX,mouseScreenY,pauseResumeBtnRect()),  {0,230,118,255});
    drawButton(pauseRestartBtnRect(),"Làm mới",   pointInRect(mouseScreenX,mouseScreenY,pauseRestartBtnRect()), {255,193,7,255});
    drawButton(pauseExitBtnRect(),   "Menu chính",pointInRect(mouseScreenX,mouseScreenY,pauseExitBtnRect()),    {255,112,67,255});
}

void SquareJumpGame::drawLevelSelect() {
    drawMenuBackground();
    drawCenteredText(screenW*0.5f,30,"Chọn Màn Chơi",{255,255,255,255},3.0f);
    drawText(screenW*0.5f-60,70,"Cuộn chuột để xem thêm  |  [ESC] Quay lại",{180,180,180,255},1.5f);

    float cellW=54, cellH=44;
    float marginX=(screenW-LEVEL_SELECT_COLS*cellW)*0.5f;
    float startY=130+static_cast<float>(levelSelectScroll);
    int rows=(LEVEL_MAX+LEVEL_SELECT_COLS-1)/LEVEL_SELECT_COLS;

    static const SDL_Color SEASON_COLORS[]={
        {0,200,100,255},{30,140,220,255},{220,130,30,255},{130,180,220,255},{210,160,60,255}
    };

    for(int r=0;r<rows;r++) {
        for(int c=0;c<LEVEL_SELECT_COLS;c++) {
            int lvl=r*LEVEL_SELECT_COLS+c+1;
            if(lvl>LEVEL_MAX) break;
            float bx=marginX+c*cellW, by=startY+r*cellH;
            if(by+cellH<0||by>screenH) continue;
            int season=getSeasonFromLevel(lvl);
            SDL_Color sColor=SEASON_COLORS[season];
            SDL_FRect btn={bx,by,cellW-4,cellH-4};
            bool hover=pointInRect(mouseScreenX,mouseScreenY,btn);
            fillRoundedRect(bx,by,cellW-4,cellH-4,6,hover?blend(sColor,{255,255,255,255},0.3f):alpha(sColor,180));
            if(hover) drawGlowRect(bx,by,cellW-4,cellH-4,sColor,2,8);
            drawCenteredText(bx+(cellW-4)*0.5f,by+10,std::to_string(lvl),{255,255,255,255},1.5f);
        }
    }

    static const char* sNames[]={"Xuân","Hè","Thu","Đông","Sa Mạc"};
    float legendX=marginX, legendY=startY+rows*cellH+12;
    if(legendY<screenH-30) {
        for(int i=0;i<5;i++) {
            fillRoundedRect(legendX+i*100,legendY,12,12,3,SEASON_COLORS[i]);
            drawText(legendX+i*100+16,legendY,sNames[i],{220,220,220,255},1.5f);
        }
    }
}

void SquareJumpGame::drawMarketPrompt() {
    drawWorldBackground(); drawPlatforms(); drawSpringNpc(); drawParticles(); drawGate();
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({1,4,9,255},210));
    SDL_FRect box={screenW*0.5f-260,screenH*0.5f-120,520,230};
    drawGlowRect(box.x,box.y,box.w,box.h,{0,230,118,255},4,12);
    fillRoundedRect(box.x,box.y,box.w,box.h,18,alpha({1,4,9,255},236));
    drawCenteredText(screenW*0.5f,box.y+32,"Vào Chợ Tết?",{255,255,255,255},3.0f);
    drawCenteredText(screenW*0.5f,box.y+88,"Muốn ghé chợ đổi quà không?",{0,230,118,255},1.5f);
    drawButton(promptYesRect(),"Có",  pointInRect(mouseScreenX,mouseScreenY,promptYesRect()),{0,230,118,255});
    drawButton(promptNoRect(), "Thôi",pointInRect(mouseScreenX,mouseScreenY,promptNoRect()),{255,183,77,255});
}

void SquareJumpGame::drawShop() {
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},200));
    float pw=540, ph=530;
    float px=screenW*0.5f-pw*0.5f, py=screenH*0.5f-ph*0.5f;
    fillRoundedRect(px,py,pw,ph,20,alpha({10,15,30,255},240));
    drawGlowRect(px,py,pw,ph,{255,235,59,255},3,12);
    drawCenteredText(screenW*0.5f,py+18,"Cửa Hàng  –  Lì xì: "+std::to_string(lixiCount),{255,235,59,255},2.5f);
    drawText(px+18,py+56,"[ESC] Đóng   [1-5] Mua",{180,180,180,255},1.5f);

    struct Item { std::string name; int cost,lv,maxLv; };
    Item items[5]={
        {"[1] Lực nhảy   +" +std::to_string(static_cast<int>(SHOP_JUMP_BONUS)), SHOP_JUMP_COST,   upgrades.jumpLevel,   SHOP_JUMP_MAX_LEVEL},
        {"[2] Sạc nhanh  -" +std::to_string(SHOP_CHARGE_BONUS)+" tick",         SHOP_CHARGE_COST, upgrades.chargeLevel, SHOP_CHARGE_MAX_LEVEL},
        {"[3] Máu tối đa +" +std::to_string(SHOP_HEALTH_BONUS)+" HP",           SHOP_HEALTH_COST, upgrades.healthLevel, SHOP_HEALTH_MAX_LEVEL},
        {"[4] Giảm nóng   (mùa hè)",                                            SHOP_HEAT_COST,   upgrades.heatLevel,   SHOP_HEAT_MAX_LEVEL},
        {"[5] Hồi máu đầy (ngay lập tức)",                                      SHOP_FULL_HEAL_COST,-1,-1},
    };
    float ih=64, iw=pw-40, isy=py+78, isx=px+20;
    for(int i=0;i<5;i++) {
        bool hover=(shopHover==i);
        bool maxed=(items[i].maxLv>0&&items[i].lv>=items[i].maxLv);
        bool afford=(lixiCount>=items[i].cost);
        SDL_Color bg=maxed?SDL_Color{40,50,40,255}:!afford?SDL_Color{50,30,30,255}:hover?SDL_Color{50,60,80,255}:SDL_Color{20,30,50,255};
        SDL_FRect r={isx,isy+i*(ih+10),iw,ih};
        fillRoundedRect(r.x,r.y,r.w,r.h,10,bg);
        if(hover&&afford&&!maxed) drawGlowRect(r.x,r.y,r.w,r.h,{0,230,118,255},2,8);
        SDL_Color tc=maxed?SDL_Color{100,200,100,255}:!afford?SDL_Color{150,100,100,255}:SDL_Color{255,255,255,255};
        drawText(r.x+14,r.y+12,items[i].name,tc,1.5f);
        if(maxed) drawText(r.x+14,r.y+34,"Tối đa",{100,200,100,255},1.5f);
        else if(items[i].maxLv>0)
            drawText(r.x+14,r.y+34,"Cấp "+std::to_string(items[i].lv)+"/"+std::to_string(items[i].maxLv)+"   Giá: "+std::to_string(items[i].cost)+" lì xì",tc,1.5f);
        else
            drawText(r.x+14,r.y+34,"Giá: "+std::to_string(items[i].cost)+" lì xì",tc,1.5f);
    }
}

void SquareJumpGame::drawDead() {
    drawWorldBackground(); drawPlatforms();
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},180));
    drawCenteredText(screenW*0.5f,screenH*0.5f-80,"Bạn đã chết",{244,67,54,255},4.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f-20,"Mùa "+std::to_string(getSeasonFromLevel(currentLevel)+1)+"  Màn: "+std::to_string(currentLevel),{255,255,255,255},1.5f);
    SDL_FRect rb={screenW*0.5f-130,screenH*0.5f+60,260,60};
    SDL_FRect mb={screenW*0.5f-130,screenH*0.5f+140,260,60};
    drawButton(rb,"[R] Chơi lại",pointInRect(mouseScreenX,mouseScreenY,rb),{244,67,54,255});
    drawButton(mb,"[ESC] Menu",  pointInRect(mouseScreenX,mouseScreenY,mb),{100,100,100,255});
    drawParticles();
}

void SquareJumpGame::drawWin() {
    drawGradientVertical({1,4,9,255},{10,20,40,255});
    for(const Petal& p:petals) drawPetal(p,true);
    drawCenteredText(screenW*0.5f,screenH*0.5f-130,"Chúc Mừng!",{255,235,59,255},3.5f);
    drawCenteredText(screenW*0.5f,screenH*0.5f-70,"Bạn đã hoàn thành cả 5 mùa!",{0,230,118,255},2.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+10,"Lì xì thu được: "+std::to_string(lixiCount),{255,235,59,255},2.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+70,"[ENTER] Về Menu Chính",{180,180,180,255},1.5f);
    drawParticles();
}

void SquareJumpGame::drawSeasonTransition() {
    static const char* names[]={"Mùa Xuân","Mùa Hè","Mùa Thu","Mùa Đông","Sa Mạc"};
    static const SDL_Color colors[]={{0,230,118,255},{33,150,243,255},{255,152,0,255},{135,206,235,255},{255,193,7,255}};
    int s=static_cast<int>(clampf(static_cast<float>(transitionSeason),0,4));
    float af=clampf(static_cast<float>(transitionTimer)/200.0f*2,0,1);
    if(transitionTimer<100) af=clampf(static_cast<float>(transitionTimer)/100.0f,0,1);
    fillRect(0,0,static_cast<float>(screenW),static_cast<float>(screenH),alpha({0,0,0,255},static_cast<Uint8>(af*220)));
    drawCenteredText(screenW*0.5f,screenH*0.5f-60,"Bắt đầu "+std::string(names[s]),alpha(colors[s],static_cast<Uint8>(af*255)),3.0f);
    drawCenteredText(screenW*0.5f,screenH*0.5f+20,"Nhấn phím bất kỳ để tiếp tục",alpha({200,200,200,255},static_cast<Uint8>(af*200)),1.5f);
}

void SquareJumpGame::drawWorld(bool showPlayer) {
    drawWorldBackground();
    if(levelData.season==SEASON_SUMMER) drawWaterZones();
    if(levelData.season==SEASON_WINTER) drawSnowZones();
    if(levelData.season==SEASON_DESERT) { drawDesertHeat(); drawOases(); drawCamels(); }
    drawPlatforms();
    drawCheckpoints();
    drawSpringNpc(); drawChildNpcs();
    drawStalls();
    if(levelData.hasBoss) { drawBoss(); drawFireballs(); drawSpinyLeaves(); }
    drawParticles();
    drawGate();
    if(showPlayer) { drawAimDots(); drawPlayer(); }
    if(levelData.season==SEASON_WINTER) drawWinterEffect();
    drawHud();
}

void SquareJumpGame::draw() {
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    switch(state) {
        case GameState::Menu:             drawMenu();          break;
        case GameState::LevelSelect:      drawLevelSelect();   break;
        case GameState::MarketPrompt:     drawMarketPrompt();  break;
        case GameState::Market:           drawWorld(true);     break;
        case GameState::Playing:          drawWorld(true);     break;
        case GameState::PauseMenu:        drawWorld(false); drawPauseMenu(); break;
        case GameState::Shop:             drawWorld(false); drawShop();      break;
        case GameState::Dead:             drawDead();          break;
        case GameState::Win:              drawWin();           break;
        case GameState::SeasonTransition: drawWorld(true); drawSeasonTransition(); break;
    }
    SDL_RenderPresent(renderer);
}
