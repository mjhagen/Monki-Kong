#pragma bss-name(push, "ZEROPAGE")

// LIVES:
#define START_LIVES     0x32
#define START_SCORE     0x30
#define ZERO_LIVES      0x30

// COORDINATES:
#define LEFT_POLE       0x48
#define RIGHT_POLE      0xA0
#define MID_POINT       0x70
#define MONKI_TOP       0x20 // TODO: should be top of a pole
#define MONKI_BOTTOM    0xE0 // TODO: should be bottom of a pole

// ANIMATION HINTS:
#define MAX_MONKIFRAME  0x06 // frames in every animation

// MOVEMENT:
enum {
  TOP,
  RIGHT,
  LEFT,
  DOWN,
  UP
};

// SOUND:
enum {
  SFX_JUMP,
  SFX_DING,
  SFX_NOISE
};

// SCENES:
enum {
  TITLE,
  STARTGAME,
  PLAYING,
  GAMEOVER,
  PAUSE
};

// ANIMATIONS:
enum {
  CLIMBING_LEFT,
  CLIMBING_RIGHT,
  JUMPING_RIGHT,
  JUMPING_LEFT,
  REACHING_LEFT,
  REACHING_RIGHT
};

// TEXTS:
const unsigned char GAMEOVER_TEXT[]   =    "GAME OVER";     // x=11
const unsigned char GAMENAME_TEXT[]   =   "MONKI KONG!";    // x=10
const unsigned char GAMEPAUSED_TEXT[] =     "PAUSED";       // x=12
const unsigned char PRESSSTART_TEXT[] = "< press start >";  // x=8

// CONSTANTS:
const unsigned int  GAME_SPEED      = 3;
const unsigned int  ANIMATION_SPEED = 3;

#pragma bss-name(push, "BSS")

// GLOBALS
unsigned char scoreboard_line_1[16];
unsigned char scoreboard_line_2[16];
unsigned char object_spr;

unsigned char pad1;
unsigned char pad1_new;

unsigned char lives=START_LIVES;
unsigned char score=START_SCORE;

// GLOBAL X/Ys
unsigned char monki_x=LEFT_POLE;
unsigned char monki_y=MONKI_TOP;
unsigned int scroll_x;
unsigned int scroll_y;

// POLES
struct pole {
  int y;
  int height;
};

struct pole poles[64];

struct pole current_pole;
struct pole current_left_pole;
struct pole current_right_pole;

// OBJECTS
struct object {
  int x;
  int y;
  int type;
  int grabbed;
};

struct object objects[64];

unsigned int object_nr;
unsigned int active_object;
// unsigned char spr_x[64];
// unsigned char spr_y[64];

// TEMPS
unsigned char temp1;
unsigned char temp2;
unsigned int x;
unsigned int y;
unsigned int i;
unsigned int t;
unsigned int l;

//
unsigned int game_mode=TITLE;
unsigned int monki_state=CLIMBING_LEFT;

unsigned int leftpole_x=(LEFT_POLE/8)+1;
unsigned int rightpole_x=(RIGHT_POLE/8);

unsigned int on_left_pole=TRUE;
unsigned int is_jumping=FALSE;
unsigned int is_reaching=FALSE;

unsigned int monki_frame=0;
unsigned int game_frame=0;

const unsigned char palette_sp[]={
  0x0F, 0x02, 0x21, 0x27, // blues
  0x0F, 0x16, 0x25, 0x30, // reds
  0x0F, 0x19, 0x29, 0x16, // greens
  0x0F, 0x27, 0x37, 0x30  // yellows
};

const unsigned char palette_bg[]={
  0x0F, 0x20, 0x10, 0x00, // black and grays
  0x0F, 0x35, 0x25, 0x15 // black and pinks
};

// HERO

#include "HERO/monki_climb.c"
#include "HERO/monki_jump.c"
#include "HERO/monki_reach.c"

const unsigned char *monki_anim_left[]   = { monki_f1, monki_f2,  monki_f3,  monki_f4,  monki_f3,  monki_f2  };
const unsigned char *monki_anim_right[]  = { monki_f5, monki_f6,  monki_f7,  monki_f8,  monki_f7,  monki_f6  };
const unsigned char *monki_anim_jump_l[] = { monki_f9, monki_f10, monki_f11, monki_f12, monki_f11, monki_f10 };
const unsigned char *monki_anim_jump_r[] = { monki_f9, monki_f10, monki_f11, monki_f12, monki_f11, monki_f10 };
const unsigned char *monki_anim_reach_l[] = { monki_f13 };
const unsigned char *monki_anim_reach_r[] = { monki_f14 };

const unsigned char **monki_states[] = {
  monki_anim_left,
  monki_anim_right,
  monki_anim_jump_l,
  monki_anim_jump_r,
  monki_anim_reach_l,
  monki_anim_reach_r
};

// MAPS

#include "MAPS/poles_01.c"
#include "MAPS/poles_02.c"

const unsigned char * const Poles[] = {
  Poles1, Poles2
};

// SCOREBOARD

const unsigned char score_text[]={
     0,  0,0x53,3, // S
     8,  0,0x63,3, // c
    16,  0,0x6F,3, // o
    24,  0,0x72,3, // r
    32,  0,0x65,3, // e
    40,  0,0x3A,3, // :
  128
};

const unsigned char lives_text[]={
     0,  0,0x4D,3, // M
     8,  0,0x4F,3, // O
    16,  0,0x4E,3, // N
    24,  0,0x4B,3, // K
    32,  0,0x49,3, // I
    40,  0,0x3A,3, // :
  128
};

// 0. blues
// 1. reds
// 2. greens
// 3. yellows

const unsigned char object_types[10][2]={
  { 0x90, 0x01 }, // 1up
  { 0x91, 0x01 }, // Adam
  { 0xA0, 0x01 }, // casette
  { 0xA1, 0x01 }, // diabolo
  { 0xB0, 0x01 }, // elleboog
  { 0xB1, 0x01 }, // fiets
  { 0xB4, 0x03 }, // lamp
  { 0xC3, 0x00 }, // LP
  { 0xC4, 0x02 }, // rups
  { 0xC5, 0x00 }  // TENT
};


// PROTOTYPES
void titleScreen( void );
void startGame( void );
void runGame( void );
void pauseGame( void );
void gameover( void );

void drawMonki( void );
void drawScoreboard( void );
void drawSprites( void );
void drawPoles( void );
void drawPole( int side );

void monkiDies( void );
void monkiGrabs( void );
void monkiMoves( int direction, int amount );

void setupObjects( void );
void movement( void );
void scrolling( void );
void updateMonkiState( void );
void upkeep( void );
void clear_bg( void );
int randRange(  int low, int high );
