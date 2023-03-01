#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include "CONSTANTS.h"
#include "INIT.h"
#include <SDL_mixer.h>
using namespace std;
int GROUND_LEVEL;
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
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};
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
        void eAnimation();
        void handleEvent(SDL_Event &e);
        void Move();
        void render();
        double getPosX();
        double getPosY();
        int getWidth();
        int getHeight();


        double ePosX, ePosY, eVelX, eVelY, eAccX, eAccY;
        int eWidth, eHeight, TICK, pre, Fading;
        int numFrame[10], curFrame[10];
        int stateY, holdLeft, holdRight;
        LTimer eTime;
        Mix_Chunk *eChunk[10];
        bool Falling, Stopping;
        SDL_RendererFlip eFlip;
        LTexture eTexture[10][10];
};
Entity::Entity()
{
    eVelX = eVelY = eAccX = ePosX = 0; ePosY = 0;
    eWidth = eHeight = 0;
    eAccY = GRAVITY;
    pre = -1; TICK = 0;
    eTime = LTimer();
    Falling = 0, Fading = -1;
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
double Entity::getPosX()
{
    return ePosX;
}
double Entity::getPosY()
{
    return ePosY;
}
int Entity::getWidth()
{
    return eWidth;
}
int Entity::getHeight()
{
    return eHeight;
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
                holdLeft = 1;
                if (holdRight) break;
                eAccX -= 0.05;
                eFlip = LEFT;
                break;
            case SDLK_d:
                holdRight = 1;
                if (holdLeft) break;
                eAccX += 0.05;
                eFlip = RIGHT;
                break;
            case SDLK_SPACE:
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
    if (Stopping)
    {
        Uint32 TIME = eTime.getTicks();
        if (TIME > 130) {Stopping = 0; return;}
        if ((int)TIME / 20 != pre)
        {
            pre = (int)TIME / 20;
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
    if (ePosY >= GROUND_LEVEL - eHeight)
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
    if (ePosX < 0 || ePosX + eWidth > SCREEN_WIDTH) ePosX -= eVelX;
}
void Entity::render()
{
    int fade = -1;
    if (Fading > -1)
    {
        Fading = max(Fading - 3, 0);
        fade = Fading;
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
    if (!fade) free();
}
bool collision(Entity A, Entity B)
{
    double leftX_A, leftX_B, rightX_A, rightX_B;
    double upY_A, upY_B, downY_A, downY_B;
    leftX_A = A.getPosX();  leftX_B = B.getPosX();
    upY_A = A.getPosY();    upY_B = B.getPosY();
    rightX_A = leftX_A + A.getWidth();
    rightX_B = leftX_B + B.getWidth();
    downY_A = upY_A + A.getHeight();
    downY_B = upY_B + B.getHeight();
    if (leftX_B >= rightX_A) return 0;
    if (leftX_A >= rightX_B) return 0;
    if (upY_A >= downY_B) return 0;
    if (upY_B >= downY_A) return 0;
    return 1;
}
Entity Mario, Coin;
void loadMario()
{
    Mario.eLoad("images/mario/mario_move", MOVE, 3);
    Mario.eLoad("images/mario/mario_jump", JUMP, 1);
    Mario.eLoad("images/mario/mario_st", STOP, 1);
    Mario.eLoad("images/mario/mario", STAND, 1);
    Mario.eMusic("sounds/jump.wav", JUMP);
    Mario.ePosY = SCREEN_HEIGHT / 2 - Mario.eHeight;
    Mario.TICK = 10;
    Mario.render();
}
void loadCoin()
{
    Coin.eLoad("images/coin_an", MOVE, 4);
    Coin.eMusic("sounds/coin.wav", JUMP);
    Coin.TICK = 50;
    Coin.ePosX = SCREEN_WIDTH / 2;
    Coin.ePosY = SCREEN_HEIGHT / 2 - Coin.eHeight;
    Coin.eAccY = 0.155;
    Coin.holdLeft = 1;
    Coin.render();
}
void MarioTest()
{
    bool collided = 0;
    loadMario();
    loadCoin();
    SDL_RenderPresent(renderer);
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
            Mario.handleEvent(e);
        }
        Mario.Move();
        Coin.Move();
        SDL_RenderClear(renderer);
        Mario.render();
        Coin.render();
        SDL_RenderPresent(renderer);
        if (collision(Mario, Coin) && !collided)
        {
            collided = 1;
            Coin.eVelY = -5.5;
            Coin.Fading = 255;
            Mix_PlayChannel(-1, Coin.eChunk[JUMP], 0);
        }
    }
}
int main(int argc, char* argv[])
{
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
    GROUND_LEVEL = SCREEN_HEIGHT / 2;
    initSDL();
    MarioTest();
    quitSDL();
    return 0;
}
