  // 8,24,0xBB,0|OAM_FLIP_H, 0,24,0xBA,0|OAM_FLIP_H,
const unsigned char monki_f13[]={
  0, 0,0x00,0|OAM_FLIP_H,  8, 0,0xC1,0|OAM_FLIP_H, 16, 0,0xC0,0|OAM_FLIP_H,
  0, 8,0xC2,0|OAM_FLIP_H,  8, 8,0xD1,0|OAM_FLIP_H, 16, 8,0xD0,0|OAM_FLIP_H,
  0,16,0x00,0|OAM_FLIP_H,  8,16,0xE1,0|OAM_FLIP_H, 16,16,0xE0,0|OAM_FLIP_H,
  0,24,0x00,0|OAM_FLIP_H,  8,24,0xF1,0|OAM_FLIP_H, 16,24,0xF0,0|OAM_FLIP_H,
  0x80
};

const unsigned char monki_f14[]={
  0, 0,0xC0,0,  8, 0,0xC1,0,  16, 0,0x00,0,
  0, 8,0xD0,0,  8, 8,0xD1,0,  16, 8,0xC2,0,
  0,16,0xE0,0,  8,16,0xE1,0,  16,16,0x00,0,
  0,24,0xF0,0,  8,24,0xF1,0,  16,24,0x00,0,
  0x80
};