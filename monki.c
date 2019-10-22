#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "monki.h"

void main( void ) {
  upkeep();
  initGame();

  while( 1 ) {
    upkeep();
    switch( game_mode ) {
      case TITLE:     titleScreen();  break;
      case STARTGAME: startGame();    break;
      case PLAYING:   runGame();      break;
      case PAUSE:     pauseGame();    break;
      case GAMEOVER:  gameover();     break;
    }
  }
}

void initGame( void ) {
  pal_bg(palette_bg);
  pal_spr(palette_sp);

  is_gameover = TRUE;

  highscorers[0].initials="BEN"; highscorers[0].score=40;
  highscorers[1].initials="MJH"; highscorers[1].score=30;
  highscorers[2].initials="HAN"; highscorers[2].score=20;
  highscorers[3].initials="JIR"; highscorers[3].score=10;
  highscorers[4].initials="CAH"; highscorers[4].score=5;
}

// game modes
void titleScreen( void ) {
  if ( player_one & PAD_START ) {
    game_mode = STARTGAME;
    return;
  }

  if ( is_gameover ) {
    is_gameover = FALSE;

    ppu_off();
    clear_bg();

    drawStaticPoles( 8*8, 8*23 );
    set_music_speed( 6 );
    music_play( 0 );

    multi_vram_buffer_horz( GAMENAME_TEXT, sizeof( GAMENAME_TEXT ), NTADR_A(11,14) );
    multi_vram_buffer_horz( "PRESS", sizeof( "PRESS" ), NTADR_A(2,14));
    multi_vram_buffer_horz( "START", sizeof( "START" ), NTADR_A(25,14));

    drawLeaderboard();
    ppu_on_all();
  }
  y =  10; for ( j = 6; j > 0; ) oam_meta_spr( 88, y+=14, titles[ --j ] );
  y = 112; for ( j = 0; j < 1; ) oam_meta_spr( 88, y+=14, titles[ j++ ] );
}

void startGame( void ) {
  // reset game elements:
  setupObjects();

  pal_fade_to(4,0);
  set_music_speed( 6 );

  drawPlayfield();

  delay(10);
  pal_bright(4);

  // set game mode for next frame:
  game_mode = PLAYING;
}

void runGame( void ) {
  if ( player_one & PAD_START ) {
    game_mode = PAUSE;
    return;
  }

  if ( is_paused || is_gameover ) {
    is_paused = FALSE;
    is_gameover = FALSE;
    drawPlayfield();
  }

  updateMovement();
  updateScroll();
  updateTimer();
  updateMonkiState();

  drawScoreboard();
  drawGaps();
  drawObjects();
  drawMonki();

  // gray_line();
}

void gameover( void ) {
  if ( !is_gameover ) {
    is_gameover = TRUE;

    music_stop();
    sfx_play( SFX_GAMEOVER, 0 );

    clear_bg();
    pal_col( 0, 0x0F );
    multi_vram_buffer_horz( GAMEOVER_TEXT, sizeof( GAMEOVER_TEXT ), NTADR_A( 11, 14 ) );
    multi_vram_buffer_horz( SCORE_TEXT, sizeof( SCORE_TEXT ), NTADR_A( 11, 16 ) );

    hasHighscore();

    hs_pos = 0;
    key_down_frame = 0;
  }

  // show final score:
  drawNumbers( 160, 128, score );

  drawEnterInitials();

  if ( player_one & PAD_START ) {
    game_mode = TITLE;
    delay( 10 );
    return;
  }
}

void pauseGame( void ) {
  if ( player_one & PAD_START ) {
    game_mode = PLAYING;
    delay( 10 );
    return;
  }

  if ( !is_paused ) {
    is_paused = TRUE;
    clear_bg();
    multi_vram_buffer_horz(GAMEPAUSED_TEXT, sizeof(GAMEPAUSED_TEXT), NTADR_A(13,14));
    delay( 6 );
  }
}

// screen elements
void setupObjects( void ) {
  seed_rng();

  lives = START_LIVES;
  score = START_SCORE;
  timer = START_TIMER;

  active_object = 0;

  // generate random objects:
  for ( i = 0; i < 64; ++i ){
    objects[ i ].grabbed  = FALSE;
    objects[ i ].type     = randRange( 0, 10 );
    objects[ i ].x        = i % 2 ? LEFT_POLE - 16 : RIGHT_POLE + 16;
    objects[ i ].y        = randRange( MONKI_TOP + 16, MONKI_BOTTOM - 16 );
  }
}

void updateTimer( void ) {
  if ( game_frame % 60 == 0 ) {
    timer--;
  }

  if ( timer <= 0x30 ) {
    game_mode = GAMEOVER;
    return;
  }

  pal_col( 0, 0x0F );
  gap_color = 0x03;

  if ( timer <= 0x35 ) {
    set_music_speed( 4 );
    pal_col( 0, 0x06 );
    gap_color = 0x01;
  } else if ( timer <= 0x3A ) {
    set_music_speed( 5 );
  } else {
    set_music_speed( 6 );
  }

  if ( timer == 0x35 ) {
    objects[ active_object ].type = 10;
  }
}

void drawPlayfield( void ) {
  clear_bg();
  pal_col( 0, 0x0F );
  gap_color = 0x03;
  drawStaticPoles( LEFT_POLE, RIGHT_POLE );
  multi_vram_buffer_horz( TIMER_TEXT, sizeof( TIMER_TEXT ), NTADR_A(1,1) );
  multi_vram_buffer_horz( LIVES_TEXT, sizeof( LIVES_TEXT ), NTADR_A(1,2) );
  multi_vram_buffer_horz( SCORE_TEXT, sizeof( SCORE_TEXT ), NTADR_A(1,3) );
}

void drawScoreboard( void ) {
  drawNumbers( 80, 6, timer );
  drawNumbers( 80, 16, lives );
  drawNumbers( 80, 26, score );
}

void drawNumbers( char x, char y, char nr ) {
  nr = ( nr - 0x30 );

  // 1
  oam_spr( x -= 8, y, (( nr / 1 ) % 10 ) + 0x30, 0x03);

  // 10
  if ( nr > 9 ) oam_spr( x -= 8, y, (( nr / 10 ) % 10 ) + 0x30, 0x03 );

  // 100
  if ( nr > 99 ) oam_spr( x -= 8, y, (( nr / 100 ) % 10) + 0x30, 0x03 );
}

void drawNumbersToBg( int x, int y, int nr ) {
  //   1
  temp1 = ( ( nr / 1 ) % 10 ) + 0x30;
  temp2 = 6;
  one_vram_buffer( temp1, NTADR_A( x--, y ) );

  //  10
  if ( nr > ( 10 - 1 ) ) {
    temp1 = ( ( nr / 10 ) % 10 ) + 0x30;
    temp2 = 5;
    one_vram_buffer( temp1, NTADR_A( x--, y ) );
  }

  // 100
  if ( nr > ( 100 - 1 ) ) {
    temp1 = ( ( nr / 100 ) % 10 ) + 0x30;
    temp2 = 4;
    one_vram_buffer( temp1, NTADR_A( x--, y ) );
  }

  multi_vram_buffer_horz( "......", temp2, NTADR_A( x-=(temp2-1), y ) );
}

void drawStaticPoles( int left, int right ) {
  pal_col( 0, 0x0F );

  left_gap_y = 0x10;
  right_gap_y = 0x78;

  multi_vram_buffer_vert( Poles[0], 30, get_ppu_addr( 0, left, 0 ) );
  multi_vram_buffer_vert( Poles[1], 30, get_ppu_addr( 0, right, 0 ) );
}

void drawGaps( void ) {
  oam_spr( LEFT_POLE, left_gap_y-8,  0x7F, gap_color );
  oam_spr( LEFT_POLE, left_gap_y,    0x7F, gap_color );
  oam_spr( LEFT_POLE, left_gap_y+8,  0x7F, gap_color );

  oam_spr( RIGHT_POLE, right_gap_y-8, 0x7F, gap_color );
  oam_spr( RIGHT_POLE, right_gap_y,   0x7F, gap_color );
  oam_spr( RIGHT_POLE, right_gap_y+8, 0x7F, gap_color );
}

void drawObjects( void ) {
  // show next object at random intervals
  if ( game_frame % 75 == 0 ) {
    active_object = randRange( 0, 63 );
    objects[ active_object ].grabbed = FALSE;
    objects[ active_object ].type = randRange( 0, 10 );
  }

  if ( objects[ active_object ].grabbed == TRUE )
    return;

  temp1 = object_types[ objects[ active_object ].type ][ 0 ]; // sprite
  temp2 = object_types[ objects[ active_object ].type ][ 1 ]; // attribute

  oam_spr( objects[ active_object ].x, objects[ active_object ].y, temp1, temp2 );
}

void drawLeaderboard( void ) {
  qsort( &highscorers, 5, sizeof(Highscorer), compareScore );
  y = 16;
  for ( i = 0; i<5; i++ ) {
    y+=2;
    multi_vram_buffer_horz( highscorers[ i ].initials, 3, NTADR_A(11,y));
    drawNumbersToBg( 20, y, highscorers[ i ].score );
  }
}

void drawEnterInitials( void ) {
  if ( leaderboard_pos < 6 ) {
    // enter high score initials:
    if ( key_down_frame == 0 || game_frame - key_down_frame > 3 )
      key_down = FALSE;

    if ( !key_down ) {
      if ( ( player_one & PAD_A || player_one & PAD_RIGHT ) && hs_pos < 2 )
        hs_pos++;

      if ( ( player_one & PAD_B || player_one & PAD_LEFT ) && hs_pos > 0 )
        hs_pos--;

      if ( player_one & PAD_UP && initials[ hs_pos ] < 0x5A )
        initials[ hs_pos ]++;

      if ( player_one & PAD_DOWN && initials[ hs_pos ] > 0x41 )
        initials[ hs_pos ]--;

      key_down = TRUE;
      key_down_frame = game_frame;
    }

    // display initials:
    for ( i = 0; i<3; i++ ) {
      if ( hs_pos == i && game_frame % 6 == 0 )
        continue;
      oam_spr( 88 + ( i * 8 ), 140, initials[ i ], hs_pos == i ? 2 : 3 );
    }

    highscorers[ leaderboard_pos ].score = score;
    highscorers[ leaderboard_pos ].initials = initials;
  }
}

// monki
void updateMonkiState( void ) {
  // monki is jumping if he's right of the left pole, or left of the right
  if( ( monki_x > ( LEFT_POLE + 8 ) && on_left_pole ) ||
      ( monki_x < ( RIGHT_POLE - 8 ) && !on_left_pole ) ) {
    is_jumping = TRUE;
  }

  if ( is_jumping && !is_reaching ) {
    // tells drawMonki which animation to use:
    monki_state = on_left_pole ? JUMPING_RIGHT : JUMPING_LEFT;

    if ( on_left_pole ) {
      // move monki over to other pole:
      if ( monki_x > 0 ) monkiMoves( RIGHT, 8 );

      // simulate gravity:
      if ( monki_x > MID_POINT ) monkiMoves( UP, 2 );
      else monkiMoves( DOWN, 4 );

      // done jumping:
      if ( monki_x < LEFT_POLE + 1 ) {
        sfx_play( SFX_IMPACT, 0 );
        is_jumping = FALSE;
      }
    } else {
      // move monki over to other pole:
      if ( monki_x > 0 ) monkiMoves( LEFT, 8 );

      // simulate gravity:
      if ( monki_x < MID_POINT ) monkiMoves( UP, 2 );
      else monkiMoves( DOWN, 4 );

      // done jumping:
      if ( monki_x > RIGHT_POLE - 1 ) {
        sfx_play( SFX_IMPACT, 0 );
        monki_frame = 0;
        is_jumping = FALSE;
      }
    }
  } else if ( is_reaching ) {
    // tells drawMonki which animation to use:
    monki_state = on_left_pole ? REACHING_LEFT : REACHING_RIGHT;
    monki_frame = 0;
    is_reaching = FALSE;
    monkiGrabs();
  } else {
    // tells drawMonki which animation to use:
    monki_state = on_left_pole ? CLIMBING_LEFT : CLIMBING_RIGHT;
  }
}

void drawMonki( void ) {
  if ( monki_state == REACHING_LEFT ) monki_x = LEFT_POLE - 16;
  if ( monki_state == CLIMBING_LEFT ) monki_x = LEFT_POLE - 8;
  if ( monki_state == CLIMBING_RIGHT ) monki_x = RIGHT_POLE - 1;
  oam_meta_spr(monki_x, monki_y, monki_states[ monki_state ][ monki_frame ] );
}

void monkiMoves( int direction, int amount ) {
  switch ( direction ) {
    case UP:      if ( monki_y <= MONKI_TOP ) return;
                  monki_y-=amount;
                  break;

    case DOWN:    if ( monki_y >= MONKI_BOTTOM ) monkiDies();
                  monki_y+=amount;
                  break;

    case LEFT:    monki_x+=amount;
                  break;

    case RIGHT:   monki_x-=amount;
                  break;

    case TOP:     on_left_pole = TRUE;
                  monki_x = LEFT_POLE;
                  monki_y = MONKI_TOP;
                  break;
  }
}

void monkiGrabs( void ) {
  if ( objects[ active_object ].grabbed ) return;

  x = objects[ active_object ].x;
  y = objects[ active_object ].y;

  // monki on same Y as object
  temp1 = monki_y < ( y + 8 ) && monki_y > ( y - 24 );

  // monki on same pole as object
  temp2 = ( on_left_pole  && x <= LEFT_POLE ) ||
          ( !on_left_pole && x >= RIGHT_POLE );

  // both true, hit!
  if ( temp1 && temp2 ) {
    oam_spr( x, y, 0x4F, 0 ); // draw hit sprite

    is_reaching = FALSE;
    objects[ active_object ].grabbed = TRUE;

    switch( object_types[ objects[ active_object ].type ][ 0 ] ) {
      case ITEM_1UP:      sfx_play( SFX_1UP, 0 );
                          lives++;
                          timer+=3;
                          break;

      case ITEM_DEATH:    monkiDies();
                          break;

      case ITEM_TIMEEXT:  sfx_play( SFX_DING, 0 );
                          timer+=5;
                          score++;
                          break;

      default:            sfx_play( SFX_DING, 0 );
                          score++;
    }
  }
}

void monkiReaches( void ) {
  if ( is_reaching ) return;
  is_reaching = TRUE;
}

void monkiJumps( int direction ) {
  if ( is_jumping ) return;

  sfx_play( SFX_JUMP, 0 );

  monki_frame = 0;

  switch ( direction ) {
    case LEFT:  monki_x = RIGHT_POLE - 8;
                on_left_pole = TRUE;
                break;

    case RIGHT: monki_x = LEFT_POLE + 16;
                on_left_pole = FALSE;
                break;
  }
}

void monkiDies( void ) {
  lives--;
  monkiMoves( TOP, 0 );
  pal_col( 0, 0x06 );
  delay( 15 );

  if ( lives < ZERO_LIVES ) {
    game_mode = GAMEOVER;
    return;
  }

  sfx_play( SFX_DEATH, 0 );
}

void hasHighscore( void ) {
  leaderboard_pos = 6;

  for( i=0; i<5; i++ ) {
    if( score > highscorers[ i ].score ) {
      leaderboard_pos = i;
    }
  }
}

// house keeping
void upkeep( void ) {
  ppu_wait_nmi();

  game_frame++;
  monki_moving = FALSE;
  player_one = pad_poll( 0 ); // read the first controller

  clear_vram_buffer();
  set_vram_buffer();
  oam_clear();
}

void updateMovement( void ) {
  if ( is_jumping ) {
    monki_moving = TRUE;
    frame( UP );
    return;
  }

  if ( on_left_pole ) {
    if ( player_one & PAD_B ) monkiReaches();
    if ( player_one & PAD_RIGHT && monkiCanMove( RIGHT ) ) {
      monkiJumps( RIGHT );
    }
  } else {
    if ( player_one & PAD_A ) monkiReaches();
    if ( player_one & PAD_LEFT && monkiCanMove( LEFT ) ) {
      monkiJumps( LEFT );
    }
  }

  if ( is_reaching ) return;

  if (player_one & PAD_UP && monkiCanMove( UP )) {
    monki_moving = TRUE;
    if ( game_frame % GAME_SPEED == 0 ) {
      frame( UP );
      monkiMoves( UP, 6 );
    }
  }

  if (player_one & PAD_DOWN && monkiCanMove( DOWN )) {
    monki_moving = TRUE;
    if ( game_frame % GAME_SPEED == 0 ) {
      frame( DOWN );
      monkiMoves( DOWN, 10 );
    }
  }
}

void frame( int direction ) {
  if ( game_frame - animation_frame < ANIMATION_SPEED )
    return;

  animation_frame = game_frame;

  switch ( direction ) {
    case UP:    monki_frame++;
                break;

    case DOWN:  monki_frame--;
                break;
  }

  if ( monki_frame > 5 ) monki_frame = 0;
  if ( monki_frame < 0 ) monki_frame = 5;
}

void updateScroll( void ) {
  ++left_gap_y;
  ++right_gap_y;
  if ( !monki_moving ) monkiMoves( DOWN, 1 );
}

void clear_bg( void ) {
  ppu_off();
  vram_adr(NAMETABLE_A);
  vram_fill(0,1024); // blank the screen
  ppu_on_all();
}

// utility

int randRange( int low, int high ){
  return rand8() % (high + 1 - low) + low;
}

int monkiCanMove( int direction ) {
  if ( direction == UP || direction == DOWN )
    temp1 = on_left_pole ? left_gap_y : right_gap_y;
  else
    temp2 = on_left_pole ? right_gap_y : left_gap_y;

  switch ( direction ) {
    case UP:    return monki_y < temp1 + 8 || monki_y > temp1 + 16;
    case DOWN:  return monki_y > temp1 + 8 || monki_y < temp1 - 40;
    case LEFT:  return !( monki_y > temp2 - 32 && monki_y < temp2 );
    case RIGHT: return !( monki_y > temp2 - 48 && monki_y < temp2 - 8 );
  }

  return TRUE;
}

int compareScore(const void *a, const void *b) {
  const Highscorer *highscoreA = (Highscorer*)a;
  const Highscorer *highscoreB = (Highscorer*)b;
  return -(highscoreA->score - highscoreB->score);
}