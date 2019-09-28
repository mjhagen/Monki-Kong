;written by Mingo Hagen 2019
;v 1.0

.export _clear_nametables

;void _clear_nametables(void);
_clear_nametables:
   lda PPU_STATUS
   lda #$20
   sta PPU_ADDR
   lda #$00
   sta PPU_ADDR
   sta PPU_CTRL
   tax
   tay
   :
      sta PPU_DATA
      inx
      bne :-
      iny
      cpy #16
      bcc :-
   rts
