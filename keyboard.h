#ifndef __KEYBOARD_C__
#define __KEYBOARD_C__

#define KEY_TIME (50)
#define KEYS_NUM (0xF)

#define KEY_QUIT ('q')
#define KEY_USPEED ('+')
#define KEY_DSPEED ('-')

#define KEY_0    ('r')
#define KEY_1    (KEY_UP)
#define KEY_2    (KEY_DOWN)
#define KEY_3    ('t')
#define KEY_4    (KEY_LEFT)
#define KEY_5    ('u')
#define KEY_6    (KEY_RIGHT)
#define KEY_7    ('y')
#define KEY_8    ('f')
#define KEY_9    ('g')
#define KEY_A    ('h')
#define KEY_B    ('j')
#define KEY_C    ('v')
#define KEY_D    ('b')
#define KEY_E    ('n')
#define KEY_16   (',')

struct keyboard_s {
  uint8_t map[KEYS_NUM];
  unsigned int timeout;
  unsigned char pressed; 
};

struct keyboard_s keyboard;

extern int keyboard_manager();


#endif
