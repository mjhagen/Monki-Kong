const unsigned char monki_f9[]={
  0, 0,0x00,0,  8, 0,0x8B,0,
  0, 8,0x9A,0,  8, 8,0x9B,0,
  0,16,0xAA,0,  8,16,0xAB,0,
  0,24,0xBA,0,  8,24,0xBB,0,
  0x80
};

const unsigned char monki_f10[]={
  0, 0,0x8C,0,  8, 0,0x00,0,
  0, 8,0x9C,0,  8, 8,0x9D,0,
  0,16,0xAC,0,  8,16,0xAD,0,
  0,24,0xBC,0,  8,24,0xBD,0,
  0x80
};

const unsigned char monki_f11[]={
  0, 0,0xC0,0,  8, 0,0xC1,0,  16, 0,0x00,0,
  0, 8,0xD0,0,  8, 8,0xD1,0,  16, 8,0xD2,0,
  0,16,0xE0,0,  8,16,0xE1,0,  16,16,0x00,0,
  0,24,0xF0,0,  8,24,0xF1,0,  16,24,0x00,0,
  0x80
};

const unsigned char monki_f12[]={
  0, 0,0x8E,0,  8, 0,0x8F,0,  16, 0,0x8D,0,  24, 0,0x00,0,
  0, 8,0x9E,0,  8, 8,0x9F,0,  16, 8,0x9F,0|OAM_FLIP_H,  24, 8,0x9E,0|OAM_FLIP_H,
  0,16,0xAE,0,  8,16,0xAF,0,  16,16,0xAF,0|OAM_FLIP_H,  24,16,0xAE,0|OAM_FLIP_H,
  0,24,0xBE,0,  8,24,0xBF,0,  16,24,0xBF,0|OAM_FLIP_H,  24,24,0xBE,0|OAM_FLIP_H,
  0x80
};