#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED
using namespace std;
const int SCREEN_WIDTH              = 1280;
const int SCREEN_HEIGHT             = 720;
const string WINDOW_TITLE           = "Mario";
const int WALKING_ANIMATION_FRAMES  = 3;
const int SCALE                     = 1;
const int STAND                     = 0;
const int MOVE                      = 1;
const int JUMP                      = 2;
const int TICK                      = 10;
const int HITBOX                    = 2;
const int STOP                      = 3;
const double XLIMIT                 = 1.8;
const double GRAVITY                = 0.03;
const SDL_RendererFlip LEFT         = SDL_FLIP_HORIZONTAL;
const SDL_RendererFlip RIGHT        = SDL_FLIP_NONE;

#endif // CONSTANTS_H_INCLUDED
