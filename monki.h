#pragma bss-name(push, "ZEROPAGE")

// LIVES / SCORE / TIMER:
#define START_TIMER     0x40 // 0x93
#define START_LIVES     0x32
#define START_SCORE     0x30
#define ZERO_LIVES      0x30

// COORDINATES:
#define LEFT_POLE       0x48
#define RIGHT_POLE      0xA0
#define MID_POINT       0x70
#define MONKI_TOP       0x20 // TODO: should be top of a pole
#define MONKI_BOTTOM    0xD0 // TODO: should be bottom of a pole

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
  SFX_JUMP,     // 0
  SFX_DING,     // 1
  SFX_IMPACT,   // 2
  SFX_1UP,      // 3
  SFX_DEATH,    // 4
  SFX_GAMEOVER  // 5
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
const unsigned char GAMEOVER_TEXT[]     =    "GAME OVER";     // x=11
const unsigned char GAMENAME_TEXT[]     =   "MONKI KONG!";    // x=10
const unsigned char GAMEPAUSED_TEXT[]   =     "PAUSED";       // x=12
const unsigned char PRESSSTART_TEXT[]   =   "press start";    // x=10

// CONSTANTS:
const unsigned int  GAME_SPEED      = 3;
const unsigned int  ANIMATION_SPEED = 3;

#pragma bss-name(push, "BSS")

// GLOBALS
unsigned char pad1;
unsigned char lives=START_LIVES;
unsigned char score=START_SCORE;
unsigned char timer=START_TIMER;

// GLOBAL X/Ys
unsigned char monki_x=LEFT_POLE;
unsigned char monki_y=MONKI_TOP;
unsigned char left_gap_y;
unsigned char right_gap_y;

// OBJECTS
struct object {
  unsigned int x;
  unsigned int y;
  unsigned int type;
  unsigned int grabbed;
};

struct object objects[64];

int active_object;

// TEMPS
unsigned char temp1;
unsigned char temp2;
unsigned int x;
unsigned int y;
unsigned int i;

unsigned int game_mode=TITLE;
unsigned int monki_state=CLIMBING_LEFT;

// monki positions
unsigned int on_left_pole=TRUE;
unsigned int is_jumping=FALSE;
unsigned int is_reaching=FALSE;
unsigned int is_gameover=FALSE;
unsigned int is_paused=FALSE;

// frame timers
int monki_frame=0;
unsigned int game_frame=0;

// color palettes
/*
  0f 02 21 27
  0f 16 25 30
  0f 19 29 16
  0d 27 37 30

  0f 20 10 00
  0f 35 25 15
  0d 00 00 00
*/

const unsigned char palette_sp[]={
  0x0F, 0x02, 0x21, 0x27, // blues
  0x0F, 0x16, 0x25, 0x30, // reds
  0x0F, 0x19, 0x29, 0x16, // greens
  0x0F, 0x27, 0x30, 0x0F  // black and yellow
};

const unsigned char palette_bg[]={
  0x31, 0x20, 0x10, 0x00, // black and grays
  0x0F, 0x35, 0x25, 0x15, // black and pinks
  0x0D, 0x00, 0x00, 0x00  // black
};

// HERO

#include "HERO/monki_climb.c"
#include "HERO/monki_jump.c"
#include "HERO/monki_reach.c"

const unsigned char *monki_anim_left[]    = { monki_f1, monki_f2,  monki_f3,  monki_f4,  monki_f3,  monki_f2  };
const unsigned char *monki_anim_right[]   = { monki_f5, monki_f6,  monki_f7,  monki_f8,  monki_f7,  monki_f6  };
const unsigned char *monki_anim_jump_l[]  = { monki_f9, monki_f10, monki_f11, monki_f12, monki_f11, monki_f10 };
const unsigned char *monki_anim_jump_r[]  = { monki_f9, monki_f10, monki_f11, monki_f12, monki_f11, monki_f10 };
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
    0, 0, 0x53, 0x03, // S
    8, 0, 0x63, 0x03, // c
   16, 0, 0x6F, 0x03, // o
   24, 0, 0x72, 0x03, // r
   32, 0, 0x65, 0x03, // e
   40, 0, 0x3A, 0x03, // :
  128
};

const unsigned char timer_text[]={
    0, 0, 0x54, 0x03, // T
    8, 0, 0x69, 0x03, // i
   16, 0, 0x6D, 0x03, // m
   24, 0, 0x65, 0x03, // e
   32, 0, 0x3A, 0x03, // :
  128
};

const unsigned char lives_text[]={
    0, 0, 0x4D, 0x03, // M
    8, 0, 0x4F, 0x03, // O
   16, 0, 0x4E, 0x03, // N
   24, 0, 0x4B, 0x03, // K
   32, 0, 0x49, 0x03, // I
   40, 0, 0x3A, 0x03, // :
  128
};


// first arg is object type, second arg is color:
// 0. blues
// 1. reds
// 2. greens
// 3. black
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
  { 0xC5, 0x01 }  // TENT
};

// PROTOTYPES
void clear_bg( void );
void controllers( void );
void drawGaps( void );
void drawMonki( void );
void drawNumbers( char x, char y, char nr );
void drawObjects( void );
void drawPoles( void );
void drawScoreboard( void );
void drawSprites( void );
void drawStaticPoles( void );
void frame( int direction );
void gameover( void );
void initRandomPole( int pole );
void monkiDies( void );
void monkiGrabs( void );
void monkiMoves( int direction, int amount );
void movement( void );
void pauseGame( void );
void runGame( void );
void scrolling( void );
void set_attr_entry( char x, char y, char pal );
void setupObjects( void );
void startGame( void );
void titleScreen( void );
void updateMonkiState( void );
void upkeep( void );

int monkiCanMove( int direction );
int getNametable( int coordinate );
int randRange( int low, int high );