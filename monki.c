#include <string.h>
#include <stdio.h>
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "monki.h"

void main( void ) {
  upkeep();
  ppu_off();
  pal_bg(palette_bg);
  pal_spr(palette_sp);
  clear_vram_buffer();
  ppu_on_all();

  is_gameover = TRUE;

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

// game modes
void titleScreen( void ) {
  if ( is_gameover ) {
    is_gameover = FALSE;
    clear_bg();
    music_play( 0 );
    multi_vram_buffer_horz( GAMENAME_TEXT, sizeof( GAMENAME_TEXT ), NTADR_A(10,12) );
    multi_vram_buffer_horz( PRESSSTART_TEXT, sizeof( PRESSSTART_TEXT ), NTADR_A(10,14));
  }
}

void startGame( void ) {
  // reset game elements:
  setupObjects();

  pal_fade_to(4,0);
  clear_bg();
  drawStaticPoles();
  delay(10);

  pal_bright(4);
  // set game mode for next frame:
  game_mode = PLAYING;
}

void runGame( void ) {
  game_frame++;

  if ( is_paused || is_gameover ) {
    is_paused = FALSE;
    is_gameover = FALSE;
    clear_bg();
    drawStaticPoles();
  }

  movement();
  scrolling();
  updateMonkiState();

  drawGaps();
  drawScoreboard();
  drawObjects();
  drawMonki();
}

void gameover( void ) {
  if ( !is_gameover ) {
    is_gameover = TRUE;
    music_stop();
    sfx_play( SFX_GAMEOVER, 0 );
    clear_bg();
    pal_col(0,0x31);
    multi_vram_buffer_horz(GAMEOVER_TEXT, sizeof(GAMEOVER_TEXT), NTADR_A(11,14));
  }

  oam_meta_spr( 88, 128, score_text );
  drawNumbers( 160, 128, score );
}

void pauseGame( void ) {
  if ( !is_paused ) {
    is_paused = TRUE;
    clear_bg();
    multi_vram_buffer_horz(GAMEPAUSED_TEXT, sizeof(GAMEPAUSED_TEXT), NTADR_A(13,14));
  }
}

// screen elements
void setupObjects( void ) {
  seed_rng();

  lives = START_LIVES;
  score = START_SCORE;
  timer = START_TIMER;

  active_object = 0;

  for ( i = 0; i < 64; ++i ){
    // generate random object:
    objects[ i ].grabbed  = FALSE;
    objects[ i ].type     = randRange( 0, 9 );
    objects[ i ].x        = i % 2
                              ? LEFT_POLE - 8
                              : RIGHT_POLE + 16;
    objects[ i ].y        = randRange( MONKI_TOP + 16, MONKI_BOTTOM - 16 );
  }
}

void drawScoreboard( void ) {
  if ( game_frame % 60 == 0 ) {
    timer--;
  }

  if ( timer <= 0x3A ) {
    pal_col(0,0x36);
    set_music_speed( 5 );
  } else {
    set_music_speed( 6 );
  }

  if ( timer <= 0x30 ) {
    game_mode = GAMEOVER;
    return;
  }

  // score
  oam_meta_spr( 8, 6, timer_text );
  drawNumbers( 80, 6, timer );

  // lives
  oam_meta_spr( 8, 16, lives_text );
  drawNumbers( 80, 16, lives );

  // score
  oam_meta_spr( 8, 26, score_text );
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

void drawStaticPoles( void ) {
  left_gap_y = 0x10;
  x = LEFT_POLE + 8;
  y = 0;

  multi_vram_buffer_vert( Poles[0], 30, get_ppu_addr( 0, x, y ) );
  multi_vram_buffer_vert( Poles[0], 30, get_ppu_addr( 2, x, y ) );

  right_gap_y = 0x78;
  x = RIGHT_POLE;
  y = 0;
  multi_vram_buffer_vert( Poles[1], 30, get_ppu_addr( 0, x, y ) );
  multi_vram_buffer_vert( Poles[1], 30, get_ppu_addr( 2, x, y ) );
}

void drawGaps( void ) {
  oam_spr( LEFT_POLE + 8, left_gap_y-8,  0x7F, 0x03 );
  oam_spr( LEFT_POLE + 8, left_gap_y,    0x7F, 0x03 );
  oam_spr( LEFT_POLE + 8, left_gap_y+8,  0x7F, 0x03 );

  oam_spr( RIGHT_POLE, right_gap_y-8, 0x7F, 0x03 );
  oam_spr( RIGHT_POLE, right_gap_y,   0x7F, 0x03 );
  oam_spr( RIGHT_POLE, right_gap_y+8, 0x7F, 0x03 );
}

void drawObjects( void ) {
  // show next object at random intervals
  if ( game_frame % 75 == 0 ) {
    // find an object that hasn't been grabbed yet
    do {
      active_object++;
      if ( active_object > 63 ) active_object = 0;
    } while( objects[ active_object ].grabbed );
  }

  if ( objects[ active_object ].grabbed == TRUE )
    return;

  temp1 = object_types[ objects[ active_object ].type ][ 0 ]; // sprite
  temp2 = object_types[ objects[ active_object ].type ][ 1 ]; // attribute

  oam_spr( objects[ active_object ].x, objects[ active_object ].y, temp1, temp2 );
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
      if ( monki_x > MID_POINT ) monkiMoves( UP, 1 );
      else monkiMoves( DOWN, monki_frame );

      // done jumping:
      if ( monki_x < LEFT_POLE + 1 ) {
        sfx_play( SFX_IMPACT, 0 );
        is_jumping = FALSE;
      }
    } else {
      // move monki over to other pole:
      if ( monki_x > 0 ) monkiMoves( LEFT, 8 );

      // simulate gravity:
      if ( monki_x < MID_POINT ) monkiMoves( UP, 1 );
      else monkiMoves( DOWN, monki_frame );

      // done jumping:
      if ( monki_x > RIGHT_POLE - 1 ) {
        sfx_play( SFX_IMPACT, 0 );
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
  if ( monki_state == REACHING_LEFT ) monki_x = LEFT_POLE - 8;
  if ( monki_state == CLIMBING_LEFT ) monki_x = LEFT_POLE + 1;

  oam_meta_spr(monki_x, monki_y, monki_states[ monki_state ][ monki_frame ] );
}

void monkiMoves( int direction, int amount ) {
  switch ( direction ) {
    case UP:
      if ( monki_y <= MONKI_TOP ) return;
      monki_y-=amount;
      break;

    case DOWN:
      if ( monki_y >= MONKI_BOTTOM ) {
        monkiDies();
      };
      monki_y+=amount;
      break;

    case LEFT:
      monki_x+=amount;
      break;

    case RIGHT:
      monki_x-=amount;
      break;

    case TOP:
      on_left_pole = TRUE;
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
    switch ( objects[ active_object ].type ) {
      case 0:
        lives++;
        sfx_play( SFX_1UP, 0 );
        break;

      case 1:
        monkiDies();
        return;

      case 3:
        timer+=10;
        score++;
        sfx_play( SFX_DING, 0 );
        break;

      default:
        score++;
        sfx_play( SFX_DING, 0 );
    }

    oam_spr( x, y, 0x4F, 0 ); // draw hit sprite

    is_reaching = FALSE;
    objects[ active_object ].grabbed = TRUE;
  }
}

void monkiReaches( void ) {
  if ( is_reaching ) return;
  is_reaching = TRUE;
}

void monkiJumps( int direction ) {
  if ( is_jumping ) return;

  sfx_play( SFX_JUMP, 0 );

  switch ( direction ) {
    case LEFT:
      monki_x = RIGHT_POLE - 16;
      on_left_pole = TRUE;
      break;
    case RIGHT:
      monki_x = LEFT_POLE + 16;
      on_left_pole = FALSE;
      break;
  }
}

void monkiDies( void ) {
  lives--;
  monkiMoves( TOP, 0 );

  if ( lives < ZERO_LIVES ) {
    game_mode = GAMEOVER;
    return;
  }

  sfx_play( SFX_DEATH, 0 );
}

// house keeping
void controllers( void ) {
  pad1 = pad_poll(0); // read the first controller

  if ( pad1 & PAD_START ) {
    switch ( game_mode ) {
      case PLAYING:
        game_mode = PAUSE;
        color_emphasis(COL_EMP_DARK);
        delay( 10 );
        return;

      case PAUSE:
        game_mode = PLAYING;
        color_emphasis( COL_EMP_NORMAL );
        delay( 10 );
        return;

      case TITLE:
      case GAMEOVER:
        game_mode = STARTGAME;
        return;
    }
  }
}

void upkeep( void ) {
  ppu_wait_nmi();
  clear_vram_buffer();
  oam_clear();
  set_vram_buffer();
  controllers();
}

void movement( void ) {
  if ( is_jumping ) {
    frame( UP );
    return;
  }

  if ( on_left_pole ) {
    if ( pad1 & PAD_LEFT ) monkiReaches();
    if ( pad1 & PAD_RIGHT && monkiCanMove( RIGHT ) ) {
      monki_frame = 0;
      monkiJumps( RIGHT );
    }
  } else {
    if ( pad1 & PAD_RIGHT ) monkiReaches();
    if ( pad1 & PAD_LEFT && monkiCanMove( LEFT ) ) {
      monki_frame = 0;
      monkiJumps( LEFT );
    }
  }

  if ( is_reaching ) return;

  if (pad1 & PAD_UP && monkiCanMove( UP )) {
    frame( UP );
    if ( game_frame % GAME_SPEED == 0 ) monkiMoves( UP, 8 );
  }

  if (pad1 & PAD_DOWN && monkiCanMove( DOWN )) {
    frame( DOWN );
    if ( game_frame % GAME_SPEED == 0 ) monkiMoves( DOWN, 8 );
  }
}

void frame( int direction ) {
  if ( game_frame % ANIMATION_SPEED == 0 ) {
    switch ( direction ) {
      case UP:
        monki_frame++;
        break;
      case DOWN:
        monki_frame--;
        break;
    }

    if ( monki_frame >= MAX_MONKIFRAME )
      monki_frame = 0;
    else if ( monki_frame < 0 )
      monki_frame = MAX_MONKIFRAME - 1;
  }
}

void scrolling( void ) {
  ++left_gap_y;
  ++right_gap_y;

  monkiMoves( DOWN, 1 );
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
  temp1 = on_left_pole ? left_gap_y : right_gap_y;
  temp2 = on_left_pole ? right_gap_y : left_gap_y;

  switch ( direction ) {
    case UP:
      return monki_y < temp1 || monki_y > temp1 + 16;
    case DOWN:
      return monki_y > temp1 || monki_y < temp1 - 40;
    case RIGHT:
    case LEFT:
      return !( monki_y > ( temp2 - 42 ) && monki_y < temp2 + 0 );
  }

  return TRUE;
}
