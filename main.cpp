#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include "CONSTANTS.h"
#include "INIT.h"
#include <SDL_mixer.h>
#define Coin(i) Object[COIN + i]
#define Goombas(i) Object[GOOMB + i]
#define Terrain(i) Object[TERRAIN + i]
using namespace std;
int GROUND_LEVEL;
SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
SDL_Rect renArea = {0, 0, int(SCREEN_WIDTH * 1.1), int(SCREEN_HEIGHT * 1.1)};
//==========================================================================//
//==========================================================================//
// ================================Texture==================================//
//==========================================================================//
//==========================================================================//

class LTexture
{
    public:
        LTexture();
        ~LTexture();
        bool imgLoad(string path);
        void free();
        void render(int x, int y, SDL_Rect *Clip = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE, int Fading = -1);
        int getWidth();
        int getHeight();
    private:
        SDL_Texture* mTexture;
        int mWidth;
        int mHeight;
};
LTexture::LTexture()
{
    mTexture = NULL;
    mWidth = mHeight = 0;
}
LTexture::~LTexture()
{
    free();
}
void LTexture::free()
{
    if (!mTexture)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = mHeight = 0;
    }
}
void LTexture::render(int x, int y, SDL_Rect* Clip, SDL_RendererFlip flip, int Fading)
{
    SDL_Rect renderQuad = {x - camera.x, y - camera.y, mWidth, mHeight};
    if (Clip)
    {
        renderQuad.w = Clip->w;
        renderQuad.h = Clip->h;
    }
    if (Fading != -1) SDL_SetTextureAlphaMod(mTexture, Fading);
    SDL_RenderCopyEx(renderer, mTexture, Clip, &renderQuad, 0, NULL, flip);
}
bool LTexture::imgLoad(string path)
{
    free();
    SDL_Texture* NewTexture = NULL;
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface)
    {
        logSDLError(cout, "Cannot load image from " + path, 1);
        return 0;
    }
    else
    {
        SDL_SetColorKey( surface, SDL_TRUE, SDL_MapRGB( surface->format, 255, 0, 255 ) );
        NewTexture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!NewTexture)
        {
            logSDLError(cout, "Cannot create texture", 1);
            return 0;
        }
        else
        {
            mWidth = surface->w;
            mHeight = surface->h;
        }
        SDL_FreeSurface(surface);
    }
    mTexture = NewTexture;
    return (mTexture != NULL);
}
int LTexture::getHeight()
{
    return mHeight;
}
int LTexture::getWidth()
{
    return mWidth;
}


//==========================================================================//
//==========================================================================//
// =================================Timer===================================//
//==========================================================================//
//==========================================================================//
class LTimer
{
    public:
        LTimer();
        void start();
        void stop();
        void pause();
        void unpause();
        Uint32 getTicks();
        bool isStarted();
        bool isPaused();
    private:
        Uint32 mStartTicks;
        Uint32 mPausedTicks;
        bool mPaused;
        bool mStarted;
};
LTimer::LTimer()
{
    mStartTicks = 0;
    mPausedTicks = 0;
    mStarted = mPaused = false;
}
void LTimer::start()
{
    mStarted = true;
    mPaused = false;
    mStartTicks = SDL_GetTicks();
    mPausedTicks = 0;
}
void LTimer::stop()
{
    mStarted = false;
    mPaused = false;
    mStartTicks = 0;
    mPausedTicks = 0;
}
void LTimer::pause()
{
    if (mStarted && !mPaused)
    {
        mPaused = true;
        mPausedTicks = SDL_GetTicks() - mStartTicks;
        mStartTicks = 0;
    }
}
void LTimer::unpause()
{
    if (mStarted && mPaused)
    {
        mPaused = false;
        mStartTicks = SDL_GetTicks() - mPausedTicks;
        mPausedTicks = 0;
    }
}
Uint32 LTimer::getTicks()
{
    Uint32 time = 0;
    if (mStarted)
    {
        if (mPaused)
        {
            time = mPausedTicks;
        }
        else
        {
            time = SDL_GetTicks() - mStartTicks;
        }
    }
    return time;
}
bool LTimer::isStarted()
{
    return mStarted;
}
bool LTimer::isPaused()
{
    return mPaused;
}
//==========================================================================//
//==========================================================================//
// ================================Entity===================================//
//==========================================================================//
//==========================================================================//
class Entity
{
    public:
        static const int eVelo = 3;
        Entity();
        ~Entity();
        void free();
        void eLoad(string path, int type, int num);
        void eMusic(string path, int type);
        void handleEvent(SDL_Event &e);
        void Move();
        void render();

        double ePosX, ePosY, eVelX, eVelY, eAccX, eAccY, FadeTime;
        int eWidth, eHeight, TICK, pre, Fading;
        int numFrame[10], curFrame[10];
        int stateY, holdLeft, holdRight, eType;
        LTimer eTime;
        Mix_Chunk *eChunk[10];
        bool Falling, Stopping, Dead;
        SDL_RendererFlip eFlip;
        LTexture eTexture[10][10];
};
Entity::Entity()
{
    eVelX = eVelY = eAccX = eAccY = 0;
    ePosX = ePosY = 0;
    eWidth = eHeight = FadeTime = 0;
    pre = -1; TICK = 0; eType = MOB;
    eTime = LTimer();
    Falling = Stopping = 0, Fading = -1; Dead = 0;
    holdLeft = holdRight = stateY = STAND;
    eFlip = RIGHT;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)eTexture[i][j] = LTexture();
        numFrame[i] = curFrame[i] = 0;
        eChunk[i] = NULL;
    }
}
Entity::~Entity()
{
    free();
}
void Entity::free()
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            eTexture[j][i].free();
        }
    }
}
void Entity::eLoad(string path, int type, int num)
{
    numFrame[type] = num;
    for (int i = 0; i < num; i++)
    {
        string des = path;
        if (num > 1) des += char(i + '0');
        des += ".bmp";
        eTexture[type][i].imgLoad(des);
    }
    eWidth = eTexture[type][0].getWidth() + HITBOX;
    eHeight = eTexture[type][0].getHeight() + HITBOX;
}
void Entity::eMusic(string path, int type)
{
    eChunk[type] = Mix_LoadWAV(path.c_str());
    if (eChunk[type] == NULL)
    {
        logSDLError(cout, "Cannot load sound file: " + path, 1);
    }
}
void Entity::handleEvent(SDL_Event &e)
{
    if (e.type == SDL_KEYDOWN && (e.key.repeat == 0))
    {
        switch(e.key.keysym.sym)
        {
            case SDLK_a:
                if (Dead) break;
                holdLeft = 1;
                if (holdRight) break;
                eAccX -= 0.05;
                eFlip = LEFT;
                break;
            case SDLK_d:
                if (Dead) break;
                holdRight = 1;
                if (holdLeft) break;
                eAccX += 0.05;
                eFlip = RIGHT;
                break;
            case SDLK_SPACE:
                if (Dead) break;
                if (ePosY == GROUND_LEVEL - eHeight)
                {
                    Mix_PlayChannel(-1, eChunk[JUMP], 0);
                    Falling = 0;
                    eVelY = -2.9;
                    stateY = MOVE;
                    eTime.start();
                }
                break;
        }
    }
    else if (e.type == SDL_KEYUP && (e.key.repeat == 0))
    {
        switch(e.key.keysym.sym)
        {
            case SDLK_a:
                holdLeft = 0;
                if (!holdRight) eAccX = eVelX = 0;
                else if (eFlip == LEFT)
                {
                    if (abs(abs(eVelX) - XLIMIT) < 0.0001 && !stateY)
                    {
                        pre = -1;
                        eTime.stop();
                        eTime.start();
                        Stopping = 1;
                    }
                    eAccX *= -1;
                    eVelX *= -1;
                    eFlip = RIGHT;
                }
                break;
            case SDLK_d:
                holdRight = 0;
                if (!holdLeft) eAccX = eVelX = 0;
                else if (eFlip == RIGHT)
                {
                    if (abs(abs(eVelX) - XLIMIT) < 0.0001 && !stateY)
                    {
                        pre = -1;
                        eTime.stop();
                        eTime.start();
                        Stopping = 1;
                    }
                    eAccX *= -1;
                    eVelX *= -1;
                    eFlip = LEFT;
                }
                break;
            case SDLK_SPACE:
                if (eVelY < 0)
                {
                    eVelY = 0.75;
                    eTime.pause();
                    Falling = 1;
                    stateY = MOVE;
                    eTime.start();
                }
                break;
        }
    }
}
void Entity::Move()
{
    if (Dead && eType != COLLECTABLE)
    {
        eVelY += 0.1;
        ePosY += eVelY;
        if (ePosY > 2 * SCREEN_HEIGHT) ePosY = 2 * SCREEN_HEIGHT;
        holdLeft = holdRight = 0;
        stateY = 1;
        return;
    }
    if (Stopping)
    {
        Uint32 TIME = eTime.getTicks();
        if (TIME > 80) {Stopping = 0; return;}
        if ((int)TIME / 25 != pre)
        {
            pre = (int)TIME / 25;
            if (eFlip == LEFT) {
                ePosX += 3;
                if (ePosX + eWidth > SCREEN_WIDTH) ePosX -= 3;
            }
            else {
                ePosX -= 3;
                if (ePosX < 0) ePosX += 3;
            }
        }
        return;
    }
    eVelY += eAccY;
    ePosY += eVelY;
    if (ePosY >= GROUND_LEVEL - eHeight && eType != BLOCK)
    {
        ePosY = GROUND_LEVEL - eHeight;
        eVelY = 0;
        stateY = 0;
        Falling = 0;
        eTime.pause();
    }
    eVelX += eAccX;
    if (abs(eVelX) > XLIMIT) eVelX = XLIMIT * abs(eVelX) / eVelX;
    ePosX += eVelX;
    if (ePosX < 0 || ePosX + eWidth > LEVEL_WIDTH)
    {
        ePosX -= eVelX;
        if (eType == MOB) eVelX *= -1;
    }
}
void Entity::render()
{
    int fade = -1;
    if (Fading > -1)
    {
        Fading = max(Fading - FadeTime, 0.0);
        fade = Fading;
    }
    if (!fade)
    {
        free();
        return;
    }
    if (Dead && eType != COLLECTABLE)
    {
        eTexture[DEAD][curFrame[DEAD]].render(ePosX, ePosY, NULL, LEFT, fade);
        return;
    }
    if (Stopping)
    {
        SDL_RendererFlip curFlip = (eFlip == LEFT ? RIGHT : LEFT);
        eTexture[STOP][curFrame[STOP]].render(ePosX, ePosY, NULL, curFlip, fade);
    }
    else if (stateY)
    {
        eTexture[JUMP][curFrame[JUMP]].render(ePosX, ePosY, NULL, eFlip, fade);
    }
    else if (holdLeft || holdRight)
    {
        eTexture[MOVE][curFrame[MOVE] / TICK].render(ePosX, ePosY, NULL, eFlip, fade);
        curFrame[MOVE]++;
        if (curFrame[MOVE] / TICK >= numFrame[MOVE]) curFrame[MOVE] = 0;
    }
    else
    {
        eTexture[STAND][curFrame[STAND]].render(ePosX, ePosY, NULL, eFlip, fade);
    }
}
bool collision(Entity A, Entity B)
{
    double leftX_A, leftX_B, rightX_A, rightX_B;
    double upY_A, upY_B, downY_A, downY_B;
    leftX_A = A.ePosX;  leftX_B = B.ePosX;
    upY_A = A.ePosY;    upY_B = B.ePosY;
    rightX_A = leftX_A + A.eWidth;
    rightX_B = leftX_B + B.eWidth;
    downY_A = upY_A + A.eHeight;
    downY_B = upY_B + B.eHeight;
    if (leftX_B >= rightX_A) return 0;
    if (leftX_A >= rightX_B) return 0;
    if (upY_A >= downY_B) return 0;
    if (upY_B >= downY_A) return 0;
    return 1;
}
Entity Object[MAX_OBJECT], Mario;
vector <int> renPos[10000];
void loadMario()
{
    Mario = Entity();
    Mario.eLoad("images/mario/mario_move", MOVE, 3);
    Mario.eLoad("images/mario/mario_jump", JUMP, 1);
    Mario.eLoad("images/mario/mario_death", DEAD, 1);
    Mario.eLoad("images/mario/mario_st", STOP, 1);
    Mario.eLoad("images/mario/mario", STAND, 1);
    Mario.eMusic("sounds/jump.wav", JUMP);
    Mario.ePosX = SCREEN_WIDTH / 15;
    Mario.ePosY = GROUND_LEVEL - Mario.eHeight;
    Mario.eAccY = GRAVITY;
    Mario.TICK = 9.5;
    Mario.eType = MAIN;
    Mario.render();
}
void loadCoin(int i)
{
    Coin(i) = Entity();
    Coin(i).eLoad("images/coin_use0", MOVE, 3);
    Coin(i).eMusic("sounds/coin.wav", JUMP);
    Coin(i).TICK = 30; Coin(i).FadeTime = FADE_TIME;
    Coin(i).ePosX = SCREEN_WIDTH / 4; Coin(i).ePosY = GROUND_LEVEL - 4 * Coin(i).eHeight;
    Coin(i).eType = COLLECTABLE;
    Coin(i).holdLeft = 1;
    Coin(i).render();
}
void loadGoombas(int i)
{
    Goombas(i) = Entity();
    Goombas(i).eLoad("images/goombas_", MOVE, 2);
    Goombas(i).eLoad("images/goombas_ded", DEAD, 1);
    Goombas(i).ePosY = GROUND_LEVEL - Goombas(1).eHeight;
    Goombas(i).ePosX = SCREEN_WIDTH / 2;
    Goombas(i).eVelX = -0.5;
    Goombas(i).TICK = 50; Goombas(i).eAccY = GRAVITY;
    Goombas(i).holdLeft = 1;
    Goombas(i).render();
}
void loadTerrain()
{
    for (int j = 0; j < 2; j++)
    {
        for (int i = 1; i <= 60; i++)
        {
            int id = j * 60 + i;
            Terrain(id).eLoad("images/gnd_red_1", STAND, 1);
            Terrain(id).ePosY = SCREEN_HEIGHT - (j + 1) * Terrain(id).eHeight;
            Terrain(id).eType = BLOCK;
            if (i == 1) Terrain(id).ePosX = 0;
            else Terrain(id).ePosX = Terrain(id - 1).ePosX + Terrain(id - 1).eWidth;
        }
    }
}
void Collide(Entity &chara, Entity &other)
{
    if (!collision(chara, other) || other.Dead || chara.Dead || other.eTexture == NULL) return;
    if (other.eType == COLLECTABLE)
    {
        other.Dead = 1;
        other.eAccY = 0.155;
        other.eVelY = -5.5;
        other.Fading = 255;
        Mix_PlayChannel(-1, other.eChunk[JUMP], 0);
    }
    else if (other.eType == MOB)
    {
        if (chara.ePosY + chara.eHeight < 1.023 * other.ePosY &&
            ((chara.ePosX + chara.eWidth >= 1.023 * other.ePosX) ||
             (1.023 * chara.ePosX <= other.ePosX + other.eWidth)))
                {
                chara.eVelY = -1.5;
                other.eVelY = -1;
                other.Dead = 1;
                }
        else
        {
            chara.Dead = 1;
            chara.eVelY = -4;
        }
    }
}
void start()
{
    loadTerrain();
    GROUND_LEVEL = SCREEN_HEIGHT - 2 * Terrain(1).eHeight + 2;
    loadMario();
    for (int i = 1; i <= 5; i++)
    {
        loadCoin(i);
        if (i == 1) continue;
        Coin(i).ePosX = Coin(i - 1).ePosX + 30;
        if (i <= 3) Coin(i).ePosY = Coin(i - 1).ePosY - 15;
        else Coin(i).ePosY = Coin(i - 1).ePosY + 15;
    }
    loadGoombas(1);
    SDL_RenderPresent(renderer);
    for (int i = 0; i < 7200; i++) renPos[i].clear();
    for (int i = 0; i < MAX_OBJECT - 10; i++)
    {
        if (Object[i].eTexture == NULL) continue;
        int k = Object[i].ePosX + Object[i].eWidth / 2;
        renPos[k].push_back(i);
    }
}
void renderMove()
{
    for (int i = 0; i <= MAX_OBJECT - 1; i++)
    {
        if (Object[i].eTexture == NULL) continue;
        Object[i].Move();
    }
    Mario.Move();
    camera.x = (Mario.ePosX + Mario.eWidth / 2) - SCREEN_WIDTH / 2;
    camera.y = (Mario.ePosY + Mario.eHeight / 2) - SCREEN_HEIGHT / 2;
    if (camera.x < 0) camera.x = 0;
    if (camera.y < 0) camera.y = 0;
    if (camera.x > LEVEL_WIDTH - camera.w) camera.x = LEVEL_WIDTH - camera.w;
    if (camera.y > LEVEL_HEIGHT - camera.h) camera.y = LEVEL_HEIGHT - camera.h;
}
void allCollide()
{
    for (int i = 1; i <= MAX_OBJECT - 1; i++)
    {
        if (Object[i].eTexture == NULL) continue;
        Collide(Mario, Object[i]);
    }
}
void Test()
{
    start();
    SDL_Event e;
    bool quit = 0;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) start();
            else Mario.handleEvent(e);
        }
        renderMove();
        allCollide();
        SDL_RenderClear(renderer);
        for (int i = 0; i <= MAX_OBJECT - 1; i++) Object[i].render();
        Mario.render();
        SDL_RenderPresent(renderer);
    }
}
int main(int argc, char* argv[])
{
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
    initSDL();
    Test();
    quitSDL();
    return 0;
}
