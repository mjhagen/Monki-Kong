#include <string.h>
#include <stdio.h>
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "monki.h"

#define NES_MIRRORING 0

void main( void ) {
  ppu_off();
  pal_bg(palette_bg);
  pal_spr(palette_sp);
  set_vram_buffer();
  clear_vram_buffer();
  ppu_on_all();

  while( 1 ) {
    while ( game_mode == TITLE ) {
      titleScreen();
    }

    while ( game_mode == STARTGAME ) {
      startGame();
    }

    while ( game_mode == PLAYING ) {
      runGame();
    }

    while ( game_mode == PAUSE ) {
      pauseGame();
    }

    while ( game_mode == GAMEOVER ) {
      gameover();
    }
  }
}

// game modes
void titleScreen( void ) {
  upkeep();
  multi_vram_buffer_horz( GAMENAME_TEXT, sizeof( GAMENAME_TEXT ), NTADR_A(10,14));
  multi_vram_buffer_horz( PRESSSTART_TEXT, sizeof( PRESSSTART_TEXT ), NTADR_A(8,16));
  set_scroll_x( 0x00 );
}

void startGame( void ) {
  seed_rng();
  upkeep();
  clear_bg();
  pal_fade_to(4,0);
  set_scroll_x( 0xFF );
  lives = START_LIVES;
  score = START_SCORE;
  delay(10);
  pal_fade_to(0,4);
  setupObjects();
  game_mode = PLAYING;
}

void runGame( void ) {
  upkeep();
  scrolling();
  game_frame = get_frame_count();
  movement();
  updateMonkiState();
  drawScoreboard();
  drawSprites();
  gray_line();
}

void gameover( void ) {
  upkeep();
  multi_vram_buffer_horz(GAMEOVER_TEXT, sizeof(GAMEOVER_TEXT), NTADR_A(11,14));
  set_scroll_x( 0x00 );
}

void pauseGame( void ) {
  upkeep();
  multi_vram_buffer_horz(GAMEPAUSED_TEXT, sizeof(GAMEPAUSED_TEXT), NTADR_A(12,14));
  set_scroll_x( 0x00 );
}



// screen elements
void setupObjects( void ) {
  for ( i = 0; i < 64; ++i ){
    // generate random object:
    objects[ i ].grabbed  = FALSE;
    objects[ i ].x        = i % 2
                              ? LEFT_POLE - 8
                              : RIGHT_POLE + 16;
    objects[ i ].y        = randRange( MONKI_TOP + 32, MONKI_BOTTOM - 32 );

    // generate random pole:
    poles[ i ].height     = randRange( 8, 24 );
    poles[ i ].y          = 0;
  }
}

void drawScoreboard( void ) {
  oam_meta_spr( 8, 8, lives_text );
  oam_spr( 56, 8, lives, 0x03 );

  oam_meta_spr( 8, 16, score_text );

  if ( score > 0x39 ) {
    temp1 = score - 0x30;
    oam_spr(56,16,(temp1/10)+0x30,0x03);
    oam_spr(64,16,(temp1%10)+0x30,0x03);
  } else {
    oam_spr(56,16,score,0x03);
  }
}

void drawPole( int side ) {
  if ( current_left_pole == NULL || current_left_pole.y > current_left_pole.height )
    current_left_pole = poles[ randRange( 0, 63 ) ];
  if ( current_right_pole == NULL || current_right_pole.y > current_right_pole.height )
    current_right_pole = poles[ randRange( 0, 63 ) ];

  vram_adr( NTADR_B( leftpole_x, current_left_pole.height - current_left_pole.y++ ) );
  vram_put( 0x80 );

  vram_adr( NTADR_B( rightpole_x, current_right_pole.height - current_right_pole.y++ ) );
  vram_put( 0x81 );
}

void drawPoles( void ) {
  // drawPole( LEFT );
  // drawPole( RIGHT );

  // OLD:
  multi_vram_buffer_vert(Poles[0], 26, NTADR_B(leftpole_x,4));
  multi_vram_buffer_vert(Poles[1], 26, NTADR_B(rightpole_x,5));
}

void drawObjects( void ) {
  if ( game_frame % 50 == 0 ) {
    object_nr = 0;

    for ( i = 0; i < 64; ++i ) {
      // object_nr = randRange(0, 63); // pick a random place
      if ( objects[ i ].grabbed == FALSE )
        object_nr = i;
    }
  }

  if ( object_nr > 0 ) {
    active_object = object_nr;

    objects[ active_object ].type = randRange( 0, sizeof( object_types ) - 1 );

    temp1 = object_types[ objects[ active_object ].type ][ 0 ]; // sprite
    temp2 = object_types[ objects[ active_object ].type ][ 1 ]; // attribute

    oam_spr( objects[ active_object ].x, objects[ active_object ].y, temp1, temp2 );
  }
}

void drawSprites( void ) {
  // drawPoles();
  drawMonki();
  drawObjects();
}


// monki
void updateMonkiState( void ) {
  // monki is jumping if he's right of the left pole, or left of the right
  if ( ( ( monki_x > LEFT_POLE && on_left_pole ) || ( monki_x < RIGHT_POLE && !on_left_pole ) ) && ( !( monki_x < LEFT_POLE ) && !( monki_x > RIGHT_POLE ) ) ) {
    is_jumping = TRUE;
  }

  if ( is_jumping ) {
    // tells drawMonki which animation to use:
    monki_state = on_left_pole ? JUMPING_RIGHT : JUMPING_LEFT;

    if ( on_left_pole ) {
      // move monki over to other pole:
      if ( monki_frame > 0 ) monkiMoves( RIGHT, 8 );

      // simulate gravity:
      if ( monki_x > MID_POINT ) monkiMoves( UP, 1 );
      else monkiMoves( DOWN, monki_frame );

      // done jumping:
      if ( monki_x < LEFT_POLE + 1 ) is_jumping = FALSE;
    } else {
      // move monki over to other pole:
      if ( monki_frame > 0 ) monkiMoves( LEFT, 8 );

      // simulate gravity:
      if ( monki_x < MID_POINT ) monkiMoves( UP, 1 );
      else monkiMoves( DOWN, monki_frame );

      // done jumping:
      if ( monki_x > RIGHT_POLE - 1 ) is_jumping = FALSE;
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
  if ( monki_state == CLIMBING_LEFT ) monki_x = LEFT_POLE;

  oam_meta_spr(monki_x, monki_y, monki_states[ monki_state ][ monki_frame ] );
}

void monkiMoves( int direction, int amount ){
  switch ( direction ) {
    case UP:
      if ( monki_y <= MONKI_TOP ) return;
      monki_y-=amount;
      break;

    case DOWN:
      if ( monki_y >= MONKI_BOTTOM ) monkiDies();
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
  temp1 = monki_y > ( y - 5 ) && monki_y < ( y + 5 );

  // monki on same pole as object
  temp2 = ( on_left_pole  && x <= LEFT_POLE ) ||
          ( !on_left_pole && x >= RIGHT_POLE );

  // both true, hit!
  if ( temp1 && temp2 ) {
    sfx_play(SFX_DING, 0);
    score++;

    oam_spr( x, y, 0x4F, 0 ); // draw hit sprite

    is_reaching = FALSE;
    objects[ active_object ].grabbed = TRUE;
    active_object = 0;
  }
}

void monkiReaches( void ) {
  if ( is_reaching ) return;
  is_reaching = TRUE;
}

void monkiJumps( int direction ) {
  if ( is_jumping ) return;

  sfx_play(SFX_JUMP, 0);

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
  }
}


// house keeping
void controllers( void ) {
  pad1 = pad_poll(0); // read the first controller
  pad1_new = get_pad_new(0);

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

void movement( void ) {
  if ( is_jumping ) {
    if ( game_frame % 4 == 0 ) {
      monki_frame = monki_frame + 1;
      if ( monki_frame >= MAX_MONKIFRAME ) monki_frame = 0;
    }
    return;
  }

  if ( on_left_pole ) {
    if ( pad1 & PAD_LEFT ) monkiReaches();
    if ( pad1 & PAD_RIGHT ) {
      monki_frame = 0;
      monkiJumps( RIGHT );
    }
  } else {
    if ( pad1 & PAD_RIGHT ) monkiReaches();
    if ( pad1 & PAD_LEFT ) {
      monki_frame = 0;
      monkiJumps( LEFT );
    }
  }

  if ( is_reaching ) return;

  if (pad1 & PAD_UP) {
    if ( game_frame % GAME_SPEED == 0 ) monkiMoves( UP, 8 );
    if ( game_frame % ANIMATION_SPEED == 0 ) monki_frame++;
  }

  if (pad1 & PAD_DOWN) {
    if ( game_frame % GAME_SPEED == 0 ) monkiMoves( DOWN, 8 );
    if ( game_frame % ANIMATION_SPEED == 0 ) monki_frame--;
  }

  if ( monki_frame >= MAX_MONKIFRAME ) monki_frame = 0;
}

void scrolling( void ) {
  scroll_y = sub_scroll_y( 1, scroll_y );
  //set_scroll_y( scroll_y );
  monkiMoves( DOWN, 1 );

  if ( ( scroll_y & 15 ) == 0 ) {
    drawPoles();
  }
}

void upkeep( void ) {
  ppu_wait_nmi();
  controllers();
  clear_vram_buffer();
  oam_clear();
}

void spritezero( void ) {
  oam_spr(0x01,0x20,0xA0,0x20);
}

void clear_bg( void ) {
}

int randRange( int low, int high ){
  return rand8() % (high + 1 - low) + low;
}