#include <iostream>
#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
using namespace std;
void logSDLError(std::ostream& os, const std::string &msg, bool fatal)
{
    os << msg << " Error: " << SDL_GetError() << std::endl;
    if (fatal) {
        SDL_Quit();
        exit(1);
    }
}
const int SCREEN_WIDTH              = 1280;
const int SCREEN_HEIGHT             = 720;
const string WINDOW_TITLE           = "Mario";
const int WALKING_ANIMATION_FRAMES  = 3;
const int SCALE                     = 1;
const int STAND                     = 0;
const int MOVE                      = 1;
const int TICK                      = 10;
const int HITBOX                    = 2;
const double GRAVITY                = 0.1983;
const SDL_RendererFlip LEFT         = SDL_FLIP_HORIZONTAL;
const SDL_RendererFlip RIGHT        = SDL_FLIP_NONE;
SDL_Window* window;
SDL_Renderer* renderer;
int GROUND_LEVEL;
void initSDL()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) logSDLError(std::cout, "SDL_Init", true);
    window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) logSDLError(std::cout, "CreateWindow", true);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == nullptr) logSDLError(std::cout, "CreateRenderer", true);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

}
void quitSDL()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void waitUntilExit()
{
    SDL_Event event;
    while (true)
    {
        while (SDL_WaitEvent(&event))
        {
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_KEYDOWN)
            {
                return;
            }
        }
        SDL_Delay(50);
    }
}
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
        void render(int x, int y, SDL_Rect *Clip = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
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
void LTexture::render(int x, int y, SDL_Rect* Clip, SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};
    if (Clip)
    {
        renderQuad.w = Clip->w;
        renderQuad.h = Clip->h;
    }
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
        void eAnimation();
        void handleEvent(SDL_Event &e);
        void Move();
        void render();
    private:
        double ePosX, ePosY;
        double eVelX, eVelY, eAccX;
        int eWidth, eHeight;
        int numFrame[10], curFrame[10];
        int stateY, holdLeft, holdRight;
        LTimer eTime;
        bool Falling;
        SDL_RendererFlip eFlip;
        LTexture eTexture[10][10];
};
Entity::Entity()
{
    eVelX = eVelY = 0;
    eAccX = 0;
    ePosX = 0; ePosY = 0;
    eWidth = eHeight = 0;
    eTime = LTimer();
    Falling = 0;
    holdLeft = holdRight = stateY = STAND;
    eFlip = RIGHT;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)eTexture[i][j] = LTexture();
        numFrame[i] = curFrame[i] = 0;
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
    ePosY = SCREEN_HEIGHT / 2;
}
void Entity::handleEvent(SDL_Event &e)
{
    if (e.type == SDL_KEYDOWN && (e.key.repeat == 0))
    {
        switch(e.key.keysym.sym)
        {
            case SDLK_a:
                holdLeft = 1;
                if (!holdRight)
                {
                    eAccX -= 4;
                    eFlip = LEFT;
                }

                break;
            case SDLK_d:
                holdRight = 1;
                if (!holdLeft)
                {
                    eAccX += 4;
                    eFlip = RIGHT;
                }
                break;
            case SDLK_SPACE:
                if (ePosY == GROUND_LEVEL)
                {
                    Falling = 0;
                    eVelY = -9;
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
                else
                {
                    eAccX *= -1;
                    eVelX *= -1;
                    eFlip = RIGHT;
                }
                break;
            case SDLK_d:
                holdRight = 0;
                if (!holdLeft) eAccX = eVelX = 0;
                else
                {
                    eAccX *= -1;
                    eVelX *= -1;
                    eFlip = LEFT;
                }
                break;
            case SDLK_SPACE:
                if (eVelY < 0)
                {
                    eVelY = 1;
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
    //Uint32 TIME = eTime.getTicks();
    eVelY += GRAVITY;
    ePosY += eVelY;
    if (ePosY >= GROUND_LEVEL)
    {
        ePosY = GROUND_LEVEL;
        eVelY = 0;
        Falling = 0;
        eTime.pause();
    }
    eVelX += eAccX;
    if (abs(eVelX) > 4) eVelX = 4 * abs(eVelX) / eVelX;
    ePosX += eVelX;
    if (ePosX < 0 || ePosX + eWidth > SCREEN_WIDTH) ePosX -= eVelX;
}
void Entity::render()
{
    if (holdLeft || holdRight)
    {
        eTexture[MOVE][curFrame[MOVE] / TICK].render(ePosX, ePosY, NULL, eFlip);
        curFrame[MOVE]++;
        if (curFrame[MOVE] / TICK >= numFrame[MOVE]) curFrame[MOVE] = 0;
    }
    else
    {
        eTexture[STAND][curFrame[STAND]].render(ePosX, ePosY, NULL, eFlip);
    }
}
Entity Mario;
void keyboard()
{
    Mario.eLoad("mario/mario_move", MOVE, 3);
    Mario.eLoad("mario/mario", STAND, 1);
    Mario.render();
    GROUND_LEVEL = SCREEN_HEIGHT / 2;
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
        SDL_RenderClear(renderer);
        Mario.render();
        SDL_RenderPresent(renderer);
    }
}
int main(int argc, char* argv[])
{
    initSDL();
    keyboard();
    //waitUntilExit();
    quitSDL();
    return 0;
}
