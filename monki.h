#include "HERO/monki_climb.c"
#include "HERO/monki_jump.c"
#include "HERO/monki_reach.c"
#include "MAPS/poles_01.c"
#include "MAPS/poles_02.c"
#include "title.c"

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
#define LEFT_POLE       0x50
#define RIGHT_POLE      0xA8
#define MID_POINT       0x7C
#define MONKI_TOP       0x20
#define MONKI_BOTTOM    0xD0

// CONSTANTS:
#define GAME_SPEED 3
#define ANIMATION_SPEED 12
#define MAX_MONKIFRAME 6

// TEXTS:
unsigned char GAMEOVER_TEXT[] = "GAME OVER";
unsigned char GAMENAME_TEXT[] = "MONKI KONG";
unsigned char GAMEPAUSED_TEXT[] = "PAUSED";
unsigned char PRESSSTART_TEXT[] = "press start";
unsigned char TIMER_TEXT[] = "TIMER:";
unsigned char LIVES_TEXT[] = "MONKI:";
unsigned char SCORE_TEXT[] = "SCORE:";

// color palettes
unsigned char palette_sp[]={
  0x0F, 0x02, 0x21, 0x27, // blues
  0x0F, 0x06, 0x36, 0x16, // reds
  0x0F, 0x19, 0x29, 0x16, // greens
  0x0F, 0x0F, 0x06, 0x30  // black and yellow
};

unsigned char palette_bg[]={
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

const unsigned char *titles[] = {
  title_row_1,
  title_row_2,
  title_row_3,
  title_row_4,
  title_row_5,
  title_row_6
};

const unsigned char *Poles[] = {
  Poles1, Poles2
};

// first arg is object type, second arg is color:
unsigned char object_types[11][2]={
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

unsigned char initials[3]="AAA";

unsigned char player_one;
unsigned char temp1;
unsigned char temp2;

unsigned char lives=START_LIVES;
unsigned char score=START_SCORE;
unsigned char timer=START_TIMER;

unsigned char gap_color=0x01;

int monki_x=LEFT_POLE;
int monki_y=MONKI_TOP;
int left_gap_y;
int right_gap_y;

unsigned int x;
unsigned int y;
unsigned int i;
unsigned int j;
unsigned int hs_letter;
unsigned int hs_pos;
unsigned int game_mode=TITLE;
unsigned int monki_state=CLIMBING_LEFT;
unsigned int on_left_pole=TRUE;
unsigned int is_gameover=FALSE;
unsigned int is_jumping=FALSE;
unsigned int is_paused=FALSE;
unsigned int is_reaching=FALSE;
unsigned int title_screen_active=FALSE;
unsigned int key_down=FALSE;
unsigned int key_down_frame=0;
unsigned int active_object=0;
unsigned int leaderboard_pos=6;

int game_frame=0;
int animation_frame=0;
int monki_frame=0;
int monki_moving=FALSE;

#pragma bss-name(push, "BSS")

// OBJECTS
struct Object {
  unsigned int x;
  unsigned int y;
  unsigned int type;
  unsigned int grabbed;
};

struct Object objects[64];

typedef struct Highscorer {
  unsigned int score;
  unsigned char *initials;
} Highscorer;

struct Highscorer highscorers[5];

unsigned int highscores[5];

// PROTOTYPES
void clear_bg( void );
void controllers( void );
void drawEnterInitials( void );
void drawGaps( void );
void drawLeaderboard( void );
void drawMonki( void );
void drawNumbers( char x, char y, char nr );
void drawNumbersToBg( int x, int y, int nr );
void drawObjects( void );
void drawPlayfield( void );
void drawScoreboard( void );
void drawStaticPoles( int left, int right );
void frame( int direction );
void gameover( void );
void hasHighscore( void );
void initGame( void );
void monkiDies( void );
void monkiGrabs( void );
void monkiJumps( int direction );
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
int compareScore(const void *a, const void *b);