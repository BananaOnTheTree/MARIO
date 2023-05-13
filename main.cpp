#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include "CONSTANTS.h"
#include "INIT.h"
#include <SDL_mixer.h>
#define pos(_i, _j) 32 * (_i), SCREEN_HEIGHT - (_j) * 32
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
        SDL_Texture* mTexture;
    private:
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
        void setPos(double x, double y);

        double ePosX, ePosY, eVelX, eVelY, eAccX, eAccY, FadeTime;
        int eWidth, eHeight, TICK, pre, Fading;
        int numFrame[10], curFrame[10], id;
        int stateY, holdLeft, holdRight, eType, originY;
        LTimer eTime;
        Mix_Chunk *eChunk[10];
        bool Falling, Dead, used;
        SDL_RendererFlip eFlip;
        LTexture eTexture[10][10];
};
Entity::Entity()
{
    eVelX = eVelY = eAccX = eAccY = originY = 0;
    ePosX = ePosY = used = 0;
    eWidth = eHeight = FadeTime = 0;
    pre = -1; TICK = 50; eType = MOB;
    eTime = LTimer();
    Falling = 0, Fading = -1; Dead = 0;
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
    used = 1;
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
            case SDLK_ESCAPE:
            {
                gamePause = 1 - gamePause;
                //if (!gamePause) pauseRender();
                break;
            }
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
                if (stateY == STAND)
                {
                    Mix_PlayChannel(-1, eChunk[JUMP], 0);
                    Falling = 0;
                    eVelY = -2.7;
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
    if (ePosX + 32 <= mRenArea.x || ePosX >= mRenArea.x + mRenArea.w) return;
    eVelY += eAccY; ePosY += eVelY;
    if (eType == FLAG)
    {
        if (ePosY >= SCREEN_HEIGHT - 4 * 32) ePosY = SCREEN_HEIGHT - 4 * 32;
        return;
    }
    if (ePosY > LEVEL_HEIGHT)
    {
        Dead = 1;
        if (eType == MAIN)
        {
            Mix_HaltChannel(1);
            Mix_PlayChannel(-1, eChunk[DEAD], 0);
        }
    }
    eVelX += eAccX;
    if (abs(eVelX) > XLIMIT) eVelX = XLIMIT * abs(eVelX) / eVelX;
    ePosX += eVelX;
    if (ePosX < 0 || ePosX + eWidth > LEVEL_WIDTH)
    {
        ePosX -= eVelX;
        if (eType == MOB) eVelX *= -1;
    }
    if ((eType == LOOT || eType == BRICK) && originY)
    {
        if (holdLeft == -1)
        {
            if (ePosY > originY - 3) ePosY -= 0.3;
            else holdLeft = 0;
        }
        else if (ePosY < originY)
        {
            ePosY += 0.3;
        }
        else originY = 0;
    }
}
void Entity::setPos(double x, double y)
{
    ePosX = x; ePosY = y;
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
    if (stateY)
    {
        eTexture[JUMP][curFrame[JUMP]].render(ePosX, ePosY, NULL, eFlip, fade);
    }
    else if ((holdLeft || holdRight) && !originY)
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
//==========================================================================//
//==========================================================================//
// ================================Font=====================================//
//==========================================================================//
//==========================================================================//
class Font
{
    public:
        Font();
        ~Font();
        void loadimage(string path);
        void render(int pos, int x, int y, int nX, int nY);
    private:
        LTexture font;
};
Font::Font()
{
    font = LTexture();
}
Font::~Font()
{
    font.free();
}
void Font::loadimage(string path)
{
    font.imgLoad(path);
}
void Font::render(int pos, int x, int y, int nX = 18, int nY = 18)
{
    SDL_Rect Clip = {pos * 8, 0, 8, 8};
    SDL_Rect renderQuad = {x, y, nX, nY};
    SDL_RenderCopyEx(renderer, font.mTexture, &Clip, &renderQuad, 0, NULL, RIGHT);
}
struct fPos
{
    int pos, x, y;
};
Entity Object[MAX_OBJECT], Mario;
Font FONT;
vector <fPos> Score, PlayerName, pauseText, pauseText2, gameOver;
bool stuck;
//==========================================================================//
//==========================================================================//
// ========================Handling Collision===============================//
//==========================================================================//
//==========================================================================//
bool collision(Entity A, Entity B)
{
    double leftX_A, leftX_B, rightX_A, rightX_B;
    double upY_A, upY_B, downY_A, downY_B;
    leftX_A = A.ePosX;  leftX_B = B.ePosX;
    upY_A = A.ePosY;    upY_B = B.ePosY;
    rightX_A = leftX_A + A.eWidth;
    rightX_B = leftX_B + B.eWidth;
    downY_A = upY_A + 32;
    downY_B = upY_B + 32;
    if (leftX_B + 9.5 >= rightX_A) return 0;
    if (leftX_A + 9.5 >= rightX_B) return 0;
    if (upY_A > downY_B) return 0;
    if (upY_B > downY_A) return 0;
    return 1;
}
void addPoint(int val);
void Collide(Entity &chara, Entity &other)
{
    if (!collision(chara, other) || other.Dead || chara.Dead || other.eTexture == NULL || other.eType == BG || other.eType == COLLECTABLE) return;
    /*if (other.eType == COLLECTABLE)
    {
        other.Dead = 1;
        other.eAccY = 0.155;
        other.eVelY = -5.5;
        other.Fading = 255;
        other.eTime.start();
        Mix_PlayChannel(-1, other.eChunk[JUMP], 0);
    }*/
    else if (other.eType == FLAG)
    {
        Object[flag_id].eVelY = 1.5;
    }
    else if (other.eType == MOB)
    {
        if (chara.ePosY + 20 < 1.03 * other.ePosY)
                {
                    chara.eVelY = -1.5;
                    other.eVelY = -1;
                    other.Dead = 1;
                    addPoint(1);
                }
        else
        {
            chara.Dead = 1;
            chara.eVelY = -4;
            Mix_HaltChannel(1);
            Mix_PlayChannel(-1, chara.eChunk[DEAD], 0);
        }
    }
    else
    {
        if (chara.ePosY + 32 >= other.ePosY
            && chara.ePosY + 32 <= other.ePosY + 4
            && chara.eVelY >= 0)
            {
                chara.ePosY = other.ePosY - 31;
                chara.Falling = 0;
                chara.eVelY = chara.stateY = STAND;
            }
        else if (chara.ePosY >= other.ePosY + 30.2)
        {
            chara.Falling = 1;
            chara.stateY = 1;
            chara.eVelY = 0;
            chara.ePosY = other.ePosY + 32;
            if (other.eType == LOOT || other.eType == BRICK)
            {
                int id = other.holdLeft;
                other.originY = other.ePosY;
                other.holdLeft = -1;
                if (id != -1)
                {
                    if (Object[id].eType == COLLECTABLE) addPoint(1);
                    Object[id].Dead = 1;
                    Object[id].eAccY = 0.155;
                    Object[id].eVelY = -5.5;
                    Object[id].Fading = 255;
                    Mix_PlayChannel(-1, Object[id].eChunk[JUMP], 0);
                }
            }
        }
        else if (chara.ePosX + 19 >= other.ePosX && chara.eFlip == RIGHT)
        {
            if (chara.eType == MAIN)
            {
                chara.ePosX = other.ePosX - 19;
                chara.eVelX = 0;
                chara.eAccX = 0.02;
            }
            else
            {
                chara.eFlip = LEFT;
                chara.eVelX *= -1;
            }
        }
        else if (chara.ePosX < other.ePosX + 27 && chara.eFlip == LEFT)
        {
            if (chara.eType == MAIN)
            {
                chara.ePosX = other.ePosX + 27;
                chara.eVelX = 0;
                chara.eAccX = -0.02;
            }
            else
            {
                chara.eFlip = RIGHT;
                chara.eVelX *= -1;
            }
        }
    }
}
//==========================================================================//
//==========================================================================//
// ===============================Loading===================================//
//==========================================================================//
//==========================================================================//
void loadMario(double x, double y)
{
    Mario = Entity();
    Mario.eLoad("images/mario/mario_move", MOVE, 3);
    Mario.eLoad("images/mario/mario_jump", JUMP, 1);
    Mario.eLoad("images/mario/mario_death", DEAD, 1);
    Mario.eLoad("images/mario/mario", STAND, 1);
    Mario.eMusic("sounds/jump.wav", JUMP);
    Mario.eMusic("sounds/death.wav", DEAD);
    Mario.setPos(x, y);
    Mario.eAccY = GRAVITY; Mario.stateY = MOVE;
    Mario.TICK = 9.5;
    Mario.eType = MAIN;
}
void loadCoin(double x, double y)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/coin_an", MOVE, 4);
    Object[total].eMusic("sounds/coin.wav", JUMP);
    Object[total].TICK = 15; Object[total].FadeTime = FADE_TIME * 1.03;
    Object[total].setPos(x, y);
    Object[total].eType = COLLECTABLE;
    Object[total].holdLeft = 1;
    Object[total].render();
}
void loadBush(double x, double y, int l, int& temp)
{
    ++temp;
    Object[++total] = Entity();
    Object[total].eLoad("images/bush_left", STAND, 1);
    Object[total].eType = BG;
    Object[total].setPos(x, y);
    for (int i = 1; i <= l; i++)
    {
        Object[++total] = Entity(); ++temp;
        if (temp % 2) Object[total].eLoad("images/bush_center_1", STAND, 1);
        else Object[total].eLoad("images/bush_center_0", STAND, 1);
        Object[total].eType = BG;
        Object[total].setPos(x + 32 * i, y);
    }
    ++temp;
    Object[++total] = Entity();
    Object[total].eLoad("images/bush_right", STAND, 1);
    Object[total].eType = BG;
    Object[total].setPos(x + 32 * (l + 1), y);
}
void multiBush(double x, double y, int h)
{
    h -= 2;
    int temp = 0;
    for (int i = 0; 2 * i <= h; i++) loadBush(x + 32 * i, y - 32 * i, h - 2 * i, temp);
    Object[++total] = Entity();
    Object[total].eLoad("images/bush_top", STAND, 1);
    Object[total].eType = BG;
    h = (h / 2) + 1;
    Object[total].setPos(x + 32 * h, y - 32 * h);
}
void loadGoombas(double x, double y)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/goombas_", MOVE, 2);
    Object[total].eLoad("images/goombas_ded", DEAD, 1);
    Object[total].setPos(x, y);
    Object[total].eVelX = -0.5;
    Object[total].eAccY = GRAVITY;
    Object[total].holdLeft = 1;
}
void loadTerrain(double &x, double &y, bool endOfRow, bool brick, int i)
{
    Object[++total] = Entity();
    if (!brick) Object[total].eLoad("images/gnd_red_1", STAND, 1);
    else
    {
        Object[total].eLoad("images/brickred", MOVE, 1);
        Object[total].eLoad("images/brickred", STAND, 1);
    }
    Object[total].setPos(x, y);
    Object[total].eType = (brick ? BRICK : BLOCK);
    Object[total].holdLeft = (brick ? -1 : 0);
    if (endOfRow)
    {
        y += 32;
        x -= 32 * i;
    }
    else x += 32;
}
void mulTerrain(double x, double y, int _i, int _j, bool brick)
{
    double posX = x, posY = y;
    for (int j = 0; j < _j; j++)
    {
        for (int i = 0; i < _i; i++)
        {
            bool eor = (i + 1 == _i);
            loadTerrain(posX, posY, eor, brick, i);
        }
    }
}
void loadPipe(double x, double y, int h)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/pipe_left_top", STAND, 1);
    Object[total].setPos(x, y); Object[total].eType = BLOCK;
    Object[++total] = Entity();
    Object[total].eLoad("images/pipe_right_top", STAND, 1);
    Object[total].setPos(x + 32, y); Object[total].eType = BLOCK;
    for (int i = 1; i <= h; i++)
    {
        Object[++total] = Entity();
        Object[total].eLoad("images/pipe_left_bot", STAND, 1);
        Object[total].setPos(x, y + i * 32); Object[total].eType = BLOCK;
        Object[++total] = Entity();
        Object[total].eLoad("images/pipe_right_bot", STAND, 1);
        Object[total].setPos(x + 32, y + i * 32); Object[total].eType = BLOCK;
    }
}
void loadLoot(double x, double y)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/blockq_", MOVE, 3);
    Object[total].eLoad("images/blockq_used", STAND, 1);
    Object[total].eType = LOOT;
    Object[total].setPos(x, y);
    Object[total].holdLeft = total + 1;
    Object[total].TICK = 30;
    loadCoin(x + 32 / 2 - 8, y + 1.72);
    Object[total].Fading = 0;
}
void loadBrick2(double x, double y)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/gnd_red2", STAND, 1);
    Object[total].setPos(x, y);
    Object[total].eType = BLOCK;
}
void mulBrick2(double x, double y, int h)
{
    for (int i = 0; i < h; i++) loadBrick2(x, y - 32 * i);
}
void loadGrass(double x, double y, int h)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/grass_left", STAND, 1);
    Object[total].setPos(x, y);
    Object[total].eType = BG;
    for (int i = 1; i <= h - 2; i++)
    {
        Object[++total] = Entity();
        Object[total].eLoad("images/grass_center", STAND, 1);
        Object[total].setPos(x + 32 * i, y);
        Object[total].eType = BG;
    }
    Object[++total] = Entity();
    Object[total].eLoad("images/grass_right", STAND, 1);
    Object[total].eType = BG;
    Object[total].setPos(x + (h - 1) * 32, y);
}
void loadFlag(double x, double y, int h)
{
    Object[++total] = Entity();
    Object[total].eLoad("images/end0_flag", STAND, 1);
    Object[total].setPos(x - 16, y - (h - 1) * 32);
    Object[total].eType = FLAG;
    flag_id = total;
    for (int i = 1; i <= h; i++)
    {
        Object[++total] = Entity();
        Object[total].eLoad("images/end0_l", STAND, 1);
        Object[total].setPos(x, y);
        Object[total].eType = FLAG;
        y -= 32;
    }
    Object[++total] = Entity();
    Object[total].eLoad("images/end0_dot", STAND, 1);
    Object[total].setPos(x, y);
    Object[total].eType = FLAG;
}
void loadCastle()
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            Object[++total] = Entity();
            Object[total].eLoad("images/castle0_brick", STAND, 1);
            Object[total].setPos(pos(194 + i, 3 + j));
            Object[total].eType = BG;
        }
    }
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            Object[++total] = Entity();
            Object[total].eLoad("images/castle0_brick", STAND, 1);
            Object[total].setPos(pos(197 + i, 3 + j));
            Object[total].eType = BG;
        }
    }
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_center_center", STAND, 1);
    Object[total].setPos(pos(196, 3));
    Object[total].eType = BG;
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_center_center_top", STAND, 1);
    Object[total].setPos(pos(196, 4));
    Object[total].eType = BG;
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_top1", STAND, 1);
    Object[total].setPos(pos(194, 5));
    Object[total].eType = BG;
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_top1", STAND, 1);
    Object[total].setPos(pos(198, 5));
    Object[total].eType = BG;
    for (int i = 0; i < 3; i++)
    {
        Object[++total] = Entity();
        Object[total].eLoad("images/castle0_top0", STAND, 1);
        Object[total].setPos(pos(195 + i, 5));
        Object[total].eType = BG;
    }
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_center_left", STAND, 1);
    Object[total].setPos(pos(195, 6));
    Object[total].eType = BG;
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_brick", STAND, 1);
    Object[total].setPos(pos(196, 6));
    Object[total].eType = BG;
    Object[++total] = Entity();
    Object[total].eLoad("images/castle0_center_right", STAND, 1);
    Object[total].setPos(pos(197, 6));
    Object[total].eType = BG;
    for (int i = 0; i < 3; i++)
    {
        Object[++total] = Entity();
        Object[total].eLoad("images/castle0_top1", STAND, 1);
        Object[total].setPos(pos(195 + i, 7));
        Object[total].eType = BG;
    }

}


//End of loading stuff

//==========================================================================//
//==========================================================================//
// =========================Camera and Font=================================//
//==========================================================================//
//==========================================================================//
void recenter(SDL_Rect &Cam)
{
    Cam.x = (Mario.ePosX + Mario.eWidth / 2) - SCREEN_WIDTH / 2;
    Cam.y = (Mario.ePosY + Mario.eHeight / 2) - SCREEN_HEIGHT / 2;
    if (Cam.x < 0) Cam.x = 0;
    if (Cam.y < 0) Cam.y = 0;
    if (Cam.x > LEVEL_WIDTH - Cam.w) Cam.x = LEVEL_WIDTH - Cam.w;
    if (Cam.y > LEVEL_HEIGHT - Cam.h) Cam.y = LEVEL_HEIGHT - Cam.h;
}
int letter(char c)
{
    return c - 'A' + 23;
}
void statInit(string NAME)
{
    Score.clear();
    intScore.clear();
    PlayerName.clear();
    for (int i = 3; i < 9; i++)
    {
        Score.push_back({6, i * 18, 36});
        intScore.push_back(0);
    }
    int nLen = NAME.length();
    for (int i = 0; i < nLen; i++) PlayerName.push_back({letter(toupper(NAME[i])), (3 + i) * 18, 18});
}
void pauseInit()
{
    pauseText.clear();
    string pText = "PAUSED";
    int nLen = pText.length();
    for (int i = 0; i < nLen; i++) pauseText.push_back({letter(pText[i]), (i + 10) * 30 + 10, 170});
    vector <string> text = {"PRESS", "ESC", "TO", "CONTINUE"};
    int posX = 190;
    for (string pText : text)
    {
        nLen = pText.length();
        for (int i = 0; i < nLen; i++)
        {
            pauseText2.push_back({letter(pText[i]), posX, 230});
            posX += 20;
        }
        posX += 20;
    }
}
void statRender()
{
    for (auto i : PlayerName) FONT.render(i.pos, i.x, i.y);
    for (auto i : Score) FONT.render(i.pos, i.x, i.y);
}
void pauseRender()
{
    for (auto i : pauseText) FONT.render(i.pos, i.x, i.y, 30, 30);
    for (auto i : pauseText2) FONT.render(i.pos, i.x, i.y, 20, 20);
}
void loadGameOver()
{
    string pText = "GAME";
    int nLen = pText.length();
    for (int i = 0; i < nLen; i++) gameOver.push_back({letter(pText[i]), (i + 10) * 27 + 10, 190});
    pText = "OVER";
    int nLen1 = pText.length();
    for (int i = 0; i < nLen1; i++) gameOver.push_back({letter(pText[i]), (i + nLen + 11) * 27 + 10, 190});
}
void renderGameOver()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    statRender();
    for (auto i : gameOver) FONT.render(i.pos, i.x, i.y, 30, 30);
}
void addPoint(int val)
{
    for (int i = 1; i <= val; i++)
    {
        int j = 3;
        while (++intScore[j] == 10 && j) intScore[j--] = 0;
        for (j = 0; j < 6; j++)
        {
            Score[j].pos = intScore[j] + 6;
        }
    }
}
//==========================================================================//
//==========================================================================//
// ============================PRESS START==================================//
//==========================================================================//
//==========================================================================//
void start()
{
    Mix_HaltChannel(-1);
    gameOverPlayed = 0;
    Mix_Chunk *music = Mix_LoadWAV("sounds/overworld.wav");
    if (music == NULL)
    {
        logSDLError(cout, "Fail to load music!", 1);
        return;
    }
    else Mix_PlayChannel(1, music, 0);
    total = 0;
    camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    renArea = {0, 0, int(SCREEN_WIDTH * 1.12), SCREEN_HEIGHT}; preArea = renArea;
    mRenArea = {0, 0, int(SCREEN_WIDTH * 1.06), SCREEN_HEIGHT}; mPreArea = mRenArea;
    ground.clear(); mobVec.clear();
    statInit(NAME); pauseInit();
    loadMario(pos(3.5, 3));
    multiBush(pos(0, 3), 5); multiBush(pos(11, 3), 3); multiBush(pos(43, 3), 5); multiBush(pos(59, 3), 3);
    multiBush(pos(90, 3),5); multiBush(pos(106, 3), 3); multiBush(pos(138, 3), 3); multiBush(pos(154, 3), 3);
    multiBush(pos(183, 3), 5); multiBush(pos(200, 3), 3);
    loadGrass(pos(6, 3), 5); loadGrass(pos(18, 3), 3); loadGrass(pos(36, 3), 4); loadGrass(pos(54, 3), 5);
    loadGrass(pos(65, 3), 3); loadGrass(pos(83, 3), 4); loadGrass(pos(101, 3), 5); loadGrass(pos(113, 3), 3);
    loadGrass(pos(131, 3), 4); loadGrass(pos(152, 3), 2); loadGrass(pos(198, 3), 2);
    mulTerrain(pos(0, 2), 63, 2, 0);
    mulTerrain(pos(65, 2), 15, 2, 0);
    FONT.loadimage("images/font.bmp");
    loadGoombas(pos(17, 3)); loadGoombas(pos(46, 3)); loadGoombas(pos(75, 11)); loadGoombas(pos(108, 3));
    loadLoot(pos(11, 6)); mulTerrain(pos(15,6), 1, 1, 1); loadLoot(pos(16,6));
    mulTerrain(pos(17, 6), 1, 1, 1); loadLoot(pos(17,10)); loadLoot(pos(18, 6));
    mulTerrain(pos(19, 6), 1, 1, 1);
    loadPipe(pos(23, 4), 1); loadPipe(pos(33, 5), 2);
    loadPipe(pos(41, 6), 3); loadPipe(pos(52, 6), 3);
    mulTerrain(pos(71, 6), 1, 1, 1); mulTerrain(pos(73, 6), 1, 1, 1);
    loadLoot(pos(72, 6)); mulTerrain(pos(74, 10), 8, 1, 1);
    mulTerrain(pos(83, 2), 64, 2, 0); mulTerrain(pos(85, 10), 3, 1, 1);
    loadLoot(pos(88, 10)); mulTerrain(pos(88, 6), 1, 1, 1); mulTerrain(pos(94, 6), 2, 1, 1);
    loadLoot(pos(100, 6)); loadLoot(pos(103, 6)); loadLoot(pos(103, 10)); loadLoot(pos(106, 6));
    mulTerrain(pos(112, 6), 1, 1, 1); mulTerrain(pos(115, 10), 3, 1, 1);
    mulTerrain(pos(122, 10), 1, 1, 1); mulTerrain(pos(125, 10), 1, 1, 1); mulTerrain(pos(123, 6), 2, 1, 2);
    loadLoot(pos(123, 10)); loadLoot(pos(124, 10));
    for (int i = 0; i < 4; i++) mulBrick2(pos(128 + i, 3), i + 1);
    for (int i = 0; i < 4; i++) mulBrick2(pos(134 + i, 3), 4 - i);
    for (int i = 0; i < 4; i++) mulBrick2(pos(142 + i, 3), i + 1);
    mulBrick2(pos(146, 3), 4);
    mulTerrain(pos(149, 2), 63, 2, 0);
    for (int i = 0; i < 4; i++) mulBrick2(pos(149 + i, 3), 4 - i);
    loadPipe(pos(157, 4), 1);
    mulTerrain(pos(162, 6), 2, 1, 1); mulTerrain(pos(165, 6), 1, 1, 1); loadLoot(pos(164, 6));
    loadPipe(pos(173, 4), 1);
    for (int i = 0; i < 5; i++) mulBrick2(pos(175 + i, 3), i + 1);
    mulBrick2(pos(180, 3), 5); mulBrick2(pos(190, 3), 1); loadFlag(pos(190, 4), 7);
    loadCastle(); loadGameOver();
    SDL_RenderPresent(renderer);
    for (int i = 0; i < 7200; i++) renPos[i].clear();
    for (int i = 0; i < MAX_OBJECT - 10; i++)
    {
        if (!Object[i].used) continue;
        Object[i].id = i;
        int k = Object[i].ePosX + Object[i].eWidth;
        if (Object[i].eType == MOB)
        {
            renPos[k].push_back(i);
            if (k <= mRenArea.w) mobVec.insert(i);
        }
        else if (Object[i].eType == BLOCK || Object[i].eType == BG)
        {
            renPos[k].push_back(i);
            if (k <= renArea.w) ground.insert(i);
        }
        else stuff.insert(i);
    }
}
void Move()
{
    Mario.Move();
    preArea = renArea; mPreArea = mRenArea;
    recenter(camera); recenter(renArea); recenter(mRenArea);
    if (preArea.x < renArea.x)
    {
        for (int x = preArea.x; x <= renArea.x; x++)
        {
            for (int j : renPos[x + preArea.w]) if (Object[j].eType != MOB) ground.insert(j);
            for (int j : renPos[x]) if (Object[j].eType != MOB) ground.erase(j);
        }
    }
    else if (preArea.x > renArea.x)
    {
        for (int x = preArea.x; x >= renArea.x; x--)
        {
            for (int j : renPos[x + preArea.w]) if (Object[j].eType != MOB) ground.erase(j);
            for (int j : renPos[x]) if (Object[j].eType != MOB) ground.insert(j);

        }
    }

    if (mPreArea.x < mRenArea.x)
    {
        for (int x = mPreArea.x; x <= mRenArea.x; x++)
        {
            for (int j : renPos[x + mPreArea.w]) if (Object[j].eType == MOB) mobVec.insert(j);
            for (int j : renPos[x]) if (Object[j].eType == MOB) mobVec.erase(j);
        }
    }
    else if (mPreArea.x > mRenArea.x)
    {
        for (int x = mPreArea.x; x >= mRenArea.x; x--)
        {
            for (int j : renPos[x + mPreArea.w]) if (Object[j].eType == MOB) mobVec.erase(j);
            for (int j : renPos[x]) if (Object[j].eType == MOB) mobVec.insert(j);
        }
    }
    for (int i : stuff) Object[i].Move();
    for (int i : mobVec) Object[i].Move();
}
void allCollide()
{
    Mario.stateY = 1;
    for (int i : ground)
    {
        for (int j : mobVec) Collide(Object[j], Object[i]);
        Collide(Mario, Object[i]);
    }
    for (int i : stuff)
    {
        for (int j : mobVec) Collide(Object[j], Object[i]);
        Collide(Mario, Object[i]);
    }
    for (int i : mobVec) Collide(Mario, Object[i]);
}
void allRender()
{
    for (int i : stuff) Object[i].render();
    for (int i : ground) Object[i].render();
    for (int i : mobVec)
    {
        if (Object[i].ePosY > LEVEL_HEIGHT + 3 || Object[i].eTime.getTicks() > 700) mobVec.erase(i);
        else Object[i].render();
    }
    statRender();
    Mario.render();
    //cout << mobVec.size() << '\n';
}
void PressStart()
{
    start();
    LTimer temp;
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
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN && !Mario.Dead && !gamePause) start();
            else Mario.handleEvent(e);
        }
        if (Mario.Dead && !temp.isStarted()) temp.start();
        if (temp.isStarted())
        {
            if (temp.getTicks() > 6600)
            {
                temp.stop();
                SDL_SetRenderDrawColor(renderer, 70, 140, 235, 0);
                start();
            }
            else if (temp.getTicks() > 2800)
            {
                if (!gameOverPlayed)
                {
                    Mix_Chunk *music = Mix_LoadWAV("sounds/gameover.wav");
                    Mix_PlayChannel(-1, music, 0);
                    gameOverPlayed = 1;
                }
                SDL_RenderClear(renderer);
                renderGameOver();
                SDL_RenderPresent(renderer);
                continue;
            }
        }
        if (!gamePause)
        {
            Move();
            allCollide();
            SDL_RenderClear(renderer);
            allRender();
        }
        else pauseRender();
        SDL_RenderPresent(renderer);
    }
}
int main(int argc, char* argv[])
{
    //cout << "ENTER YOUR NAME (No spacing): ";
    //cin >> NAME;
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
    initSDL();
    PressStart();
    quitSDL();
    return 0;
}
