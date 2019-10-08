#include "HERO/monki_climb.c"
#include "HERO/monki_jump.c"
#include "HERO/monki_reach.c"
#include "MAPS/poles_01.c"
#include "MAPS/poles_02.c"

#pragma bss-name(push, "ZEROPAGE")

// LIVES / SCORE / TIMER:
#define START_TIMER     0x58
#define START_LIVES     0x32
#define START_SCORE     0x30
#define ZERO_LIVES      0x30
#define ITEM_1UP        0x90
#define ITEM_DEATH      0x91
#define ITEM_TIMEEXT    0xA1

// COORDINATES:
#define LEFT_POLE       0x48
#define RIGHT_POLE      0xA0
#define MID_POINT       0x70
#define MONKI_TOP       0x20
#define MONKI_BOTTOM    0xD0

// TEXTS:
const unsigned char GAMEOVER_TEXT[]     =    "GAME OVER";     // x=11
const unsigned char GAMENAME_TEXT[]     =   "MONKI KONG!";    // x=10
const unsigned char GAMEPAUSED_TEXT[]   =     "PAUSED";       // x=12
const unsigned char PRESSSTART_TEXT[]   =   "press start";    // x=10

// CONSTANTS:
const int  GAME_SPEED      = 3;
const int  ANIMATION_SPEED = 6;
const int  MAX_MONKIFRAME  = 6;

// color palettes
const unsigned char palette_sp[]={
  0x0F, 0x02, 0x21, 0x27, // blues
  0x0F, 0x06, 0x36, 0x16, // reds
  0x0F, 0x19, 0x29, 0x16, // greens
  0x0F, 0x0F, 0x06, 0x30  // black and yellow
};

const unsigned char palette_bg[]={
  0x0F, 0x20, 0x10, 0x30
};

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
const unsigned char object_types[11][2]={
  { 0x91, 0x01 }, // Adam
  { 0x90, 0x01 }, // 1up
  { 0xA0, 0x01 }, // casette
  { 0xB0, 0x01 }, // elleboog
  { 0xB1, 0x01 }, // fiets
  { 0xB4, 0x00 }, // lamp
  { 0xC3, 0x00 }, // LP
  { 0xC4, 0x02 }, // rups
  { 0xC5, 0x01 }, // TENT
  { 0xC6, 0x00 }, // LAMP2
  { 0xA1, 0x02 }, // diabolo
};

#pragma bss-name(push, "BSS")

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

unsigned char pad1;
unsigned char temp1;
unsigned char temp2;
unsigned char left_gap_y;
unsigned char right_gap_y;
unsigned char lives=START_LIVES;
unsigned char score=START_SCORE;
unsigned char timer=START_TIMER;
unsigned char monki_x=LEFT_POLE;
unsigned char monki_y=MONKI_TOP;
unsigned char gap_color=0x01;

unsigned int x;
unsigned int y;
unsigned int i;
unsigned int game_mode=TITLE;
unsigned int monki_state=CLIMBING_LEFT;
unsigned int on_left_pole=TRUE;
unsigned int is_gameover=FALSE;
unsigned int is_jumping=FALSE;
unsigned int is_paused=FALSE;
unsigned int is_reaching=FALSE;
unsigned int active_object=0;

int game_frame=0;
int monki_frame=0;

// OBJECTS
struct object {
  unsigned int x;
  unsigned int y;
  unsigned int type;
  unsigned int grabbed;
};

struct object objects[64];

// PROTOTYPES
void clear_bg( void );
void controllers( void );
void drawGaps( void );
void drawMonki( void );
void drawNumbers( char x, char y, char nr );
void drawObjects( void );
void drawScoreboard( void );
void drawStaticPoles( void );
void frame( int direction );
void gameover( void );
void monkiDies( void );
void monkiJumps( int direction );
void monkiGrabs( void );
void monkiMoves( int direction, int amount );
void movement( void );
void pauseGame( void );
void runGame( void );
void scrolling( void );
void setupObjects( void );
void startGame( void );
void titleScreen( void );
void updateMonkiState( void );
void updateTimer( void );
void upkeep( void );
int monkiCanMove( int direction );
int randRange( int low, int high );