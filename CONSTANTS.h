#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED
using namespace std;
const int SCREEN_WIDTH              = 800;
const int SCREEN_HEIGHT             = 460;
const int LEVEL_WIDTH               = 7168;
const int LEVEL_HEIGHT              = 460;
const string WINDOW_TITLE           = "Mario";
const int WALKING_ANIMATION_FRAMES  = 3;
const int SCALE                     = 1;
const int STAND                     = 0;
const int MOVE                      = 1;
const int JUMP                      = 2;
const int HITBOX                    = 2;
const int STOP                      = 3;
const int DEAD                      = 4;
const int MAIN                      = 0;
const int MOB                       = 1;
const int COLLECTABLE               = 2;
const int BLOCK                     = 3;
const int BG                        = 4;
const int LOOT                      = 5;
const int BRICK                     = 6;
const int MAX_OBJECT                = 800;
const int MAIN_CHAR                 = 0;
const int TERRAIN                   = 1;
const int COIN                      = 600;
const int GOOMB                     = 700;
const double FADE_TIME              = 3.05;
const double XLIMIT                 = 1.1;
const double GRAVITY                = 0.025;
const SDL_RendererFlip LEFT         = SDL_FLIP_HORIZONTAL;
const SDL_RendererFlip RIGHT        = SDL_FLIP_NONE;
int total;
vector <int> renPos[10000];
unordered_set <int> ground, stuff;
vector <int> mobVec;
#endif // CONSTANTS_H_INCLUDED
