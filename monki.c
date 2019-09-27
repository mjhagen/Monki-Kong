#include <string.h>
#include <stdio.h>
#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "monki.h"

void main( void ) {
  ppu_off();

  pal_bg(palette_bg);
  pal_spr(palette_sp);

  set_vram_buffer();
  clear_vram_buffer();

  ppu_on_all();

  // spritezero();

  ppu_wait_nmi();

  while( 1 ) {
    while ( game_mode == TITLE ) {
      ppu_wait_nmi();

      controllers();

      clear_vram_buffer();
      oam_clear();

      drawTitleScreen();
    }

    while ( game_mode == STARTGAME ) {
      ppu_wait_nmi();

      clear_vram_buffer();
      oam_clear();

      pal_fade_to(4,0);

      delay(10);

      pal_fade_to(0,4);

      startGame();
    }

    while ( game_mode == PLAYING ) {
      ppu_wait_nmi();

      controllers();

      clear_vram_buffer();
      oam_clear();

      game_frame = get_frame_count();
      movement();
      scrolling();
      updateMonkiState();
      drawScoreboard();
      drawSprites();

      gray_line();
    }

    while ( game_mode == PAUSE ) {
      ppu_wait_nmi();

      controllers();

      clear_vram_buffer();
      oam_clear();

      // game_frame = get_frame_count();
      // movement();
      // scrolling();
      // updateMonkiState();
      drawScoreboard();
      // drawSprites();
    }

    while ( game_mode == GAMEOVER ) {
      ppu_wait_nmi();

      controllers();

      clear_vram_buffer();
      oam_clear();

      drawGameover();
    }
  }
}

// screen elements
void drawScoreboard( void ) {
  oam_meta_spr( 8, 8, lives_text );
  oam_spr(56,8,lives,3);

  oam_meta_spr( 8, 16, score_text );
  if ( score > 9 ){
    oam_spr(56,16,score/10,3);
    oam_spr(64,16,score%10,3);
  }else{
    oam_spr(56,16,score,3);
  }
}

void drawPoles( void ) {
  multi_vram_buffer_vert(Poles[0], 26, NTADR_A(leftpole_x,3));
  multi_vram_buffer_vert(Poles[1], 26, NTADR_A(rightpole_x,3));
}

void setupObjects( void ) {
  for ( i = 0; i < 64; ++i ){
    spr_x[ i ] = i % 2
      ? LEFT_POLE - 8
      : RIGHT_POLE + 16;
    spr_y[ i ] = randRange( MONKI_BOTTOM - 32, MONKI_TOP + 32 );
  }
}

void drawObjects( void ) {
  if ( game_frame % 50 == 0 ) {
    object_nr = randRange(0, 63);
    object_spr = randRange(0, sizeof(objects)-1);
  }

  active_object = object_nr;

  x = spr_x[ object_nr ];
  y = spr_y[ object_nr ];

  temp1 = objects[ object_spr ][ 0 ];
  temp2 = objects[ object_spr ][ 1 ];

  oam_spr( x, y, temp1, temp2 );
}

void drawGameover( void ) {
  multi_vram_buffer_horz(GAMEOVER_TEXT, sizeof(GAMEOVER_TEXT), NTADR_A(11,14));
}

void drawSprites( void ) {
  drawPoles();
  drawMonki();
  drawObjects();
}

void drawTitleScreen( void ) {
  multi_vram_buffer_horz(GAME_NAME, sizeof(GAME_NAME), NTADR_A(11,14));
  multi_vram_buffer_horz(PRESS_START, sizeof(PRESS_START), NTADR_A(9,16));
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
  if ( monki_y > ( spr_y[ active_object ] - 5 ) && monki_y < ( spr_y[ active_object ] + 5 ) ) {
    sfx_play(SFX_DING, 0);
    oam_spr( spr_x[ active_object ], spr_y[ active_object ], 0x4F, 0 );
    active_object = 0;
    score++;
    is_reaching = FALSE;
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
void startGame( void ) {

  seed_rng();
  setupObjects();
  game_mode = PLAYING;
}

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
  if ( monki_frame < 0 ) monki_frame = MAX_MONKIFRAME - 1;
}

void scrolling(void){
  // scroll_y = sub_scroll_y(1,scroll_y);
  // set_scroll_y(scroll_y);
  monkiMoves( DOWN, 1 );
}

void spritezero(void){
  oam_spr(0x01,0x20,0xA0,0x20);
}

int randRange( int low, int high ){
  return rand8() % (high + 1 - low) + low;
}