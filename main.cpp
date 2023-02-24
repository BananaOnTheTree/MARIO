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
const int MOVEX                     = 1;
const double TICK                   = 1.35;
const int HITBOX                    = 2;
const SDL_RendererFlip LEFT         = SDL_FLIP_HORIZONTAL;
const SDL_RendererFlip RIGHT        = SDL_FLIP_NONE;
SDL_Window* window;
SDL_Renderer* renderer;
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
    renderQuad.w *= SCALE;
    renderQuad.h *= SCALE;
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
LTexture spriteClip[10];

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
        int ePosX, ePosY;
        int eVelX, eVelY, eAccX, eAccY;
        int eWidth, eHeight;
        int numFrame[10], curFrame[10];
        int state;
        SDL_RendererFlip eFlip;
        LTexture eTexture[10][10];
};
Entity::Entity()
{
    eVelX = eVelY = 0;
    eAccX = eAccY = 0;
    ePosX = 0; ePosY = 0;
    eWidth = eHeight = state = 0;
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
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
    {
        switch(e.key.keysym.sym)
        {
            /*case SDLK_UP:
                eVelY -= eVelo;
                break;
            case SDLK_DOWN:
                eVelY += eVelo;
                break;*/
            case SDLK_LEFT:
                eAccX -= 2;
                state = 1;
                eFlip = LEFT;
                break;
            case SDLK_RIGHT:
                eAccX += 2;
                state = 1;
                eFlip = RIGHT;
                break;
        }
    }
    else if (e.type == SDL_KEYUP && e.key.repeat == 0)
    {
        switch(e.key.keysym.sym)
        {
            /*case SDLK_UP: eVelY += eVelo; break;
            case SDLK_DOWN: eVelY -= eVelo; break;*/
            case SDLK_LEFT:
                eAccX = eVelX = 0;
                state = 0;
                break;
            case SDLK_RIGHT:
                eAccX = eVelX = 0;
                state = 0;
                break;
        }
    }
    Move();
    SDL_RenderClear(renderer);
    render();
    SDL_RenderPresent(renderer);
}
void Entity::Move()
{
    eVelX += eAccX;
    if (abs(eVelX) > 10) eVelX = 10 * abs(eVelX) / eVelX;
    ePosX += eVelX;
    if (ePosX < 0 || ePosX + eWidth > SCREEN_WIDTH) ePosX -= eVelX;
    /*ePosY += eVelY;
    if (ePosY < 0 || ePosY + eHeight > SCREEN_HEIGHT) ePosY -= eVelY;*/
}
void Entity::render()
{
    if (state)
    {
        eTexture[MOVEX][int(curFrame[MOVEX] / TICK)].render(ePosX, ePosY, NULL, eFlip);
        curFrame[MOVEX]++;
        if (curFrame[MOVEX] / TICK >= numFrame[MOVEX]) curFrame[MOVEX] = 0;
    }
    else
    {
        eTexture[STAND][curFrame[STAND]].render(ePosX, ePosY, NULL, eFlip);
    }
}
Entity Mario;
void keyboard()
{
    Mario.eLoad("mario/mario_move", MOVEX, 3);
    Mario.eLoad("mario/mario", STAND, 1);
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
        /*Mario.Move();
        SDL_RenderClear(renderer);
        Mario.render();
        SDL_RenderPresent(renderer);*/
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
