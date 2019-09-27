// on left pole
const unsigned char monki_f1[]={
  0,  0,0x82,0,  8,  0,0x83,0,
  0,  8,0x92,0,  8,  8,0x93,0,
  0, 16,0xA2,0,  8, 16,0xA3,0,
  0, 24,0xB2,0,  8, 24,0xB3,0,
  0x80
};

const unsigned char monki_f2[]={
  0,  0,0x84,0,  8,  0,0x85,0,
  0,  8,0x94,0,  8,  8,0x95,0,
  0, 16,0xA4,0,  8, 16,0xA5,0,
  0, 24,0x00,0,  8, 24,0xB5,0,
  0x80
};

const unsigned char monki_f3[]={
  0,  0,0x88,0,  8,  0,0x89,0,
  0,  8,0x98,0,  8,  8,0x99,0,
  0, 16,0xA8,0,  8, 16,0xA9,0,
  0, 24,0xB8,0,  8, 24,0xB9,0,
  0x80
};

const unsigned char monki_f4[]={
  0,  0,0x86,0,  8,  0,0x87,0,
  0,  8,0x96,0,  8,  8,0x97,0,
  0, 16,0xA6,0,  8, 16,0xA7,0,
  0, 24,0xB6,0,  8, 24,0xB7,0,
  0x80
};

// on right pole
const unsigned char monki_f5[]={
  0,  0,0x83,0|OAM_FLIP_H,  8,  0,0x82,0|OAM_FLIP_H,
  0,  8,0x93,0|OAM_FLIP_H,  8,  8,0x92,0|OAM_FLIP_H,
  0, 16,0xA3,0|OAM_FLIP_H,  8, 16,0xA2,0|OAM_FLIP_H,
  0, 24,0xB3,0|OAM_FLIP_H,  8, 24,0xB2,0|OAM_FLIP_H,
  0x80
};

const unsigned char monki_f6[]={
  0,  0,0x85,0|OAM_FLIP_H,  8,  0,0x84,0|OAM_FLIP_H,
  0,  8,0x95,0|OAM_FLIP_H,  8,  8,0x94,0|OAM_FLIP_H,
  0, 16,0xA5,0|OAM_FLIP_H,  8, 16,0xA4,0|OAM_FLIP_H,
  0, 24,0xB5,0|OAM_FLIP_H,  8, 24,0x00,0|OAM_FLIP_H,
  0x80
};

const unsigned char monki_f7[]={
  0,  0,0x89,0|OAM_FLIP_H,  8,  0,0x88,0|OAM_FLIP_H,
  0,  8,0x99,0|OAM_FLIP_H,  8,  8,0x98,0|OAM_FLIP_H,
  0, 16,0xA9,0|OAM_FLIP_H,  8, 16,0xA8,0|OAM_FLIP_H,
  0, 24,0xB9,0|OAM_FLIP_H,  8, 24,0xB8,0|OAM_FLIP_H,
  0x80
};

const unsigned char monki_f8[]={
  0,  0,0x87,0|OAM_FLIP_H,  8,  0,0x86,0|OAM_FLIP_H,
  0,  8,0x97,0|OAM_FLIP_H,  8,  8,0x96,0|OAM_FLIP_H,
  0, 16,0xA7,0|OAM_FLIP_H,  8, 16,0xA6,0|OAM_FLIP_H,
  0, 24,0xB7,0|OAM_FLIP_H,  8, 24,0xB6,0|OAM_FLIP_H,
  0x80
};
