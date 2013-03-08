#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>

#include "display.h"
#include "keyboard.h"

void init_keyboard(void)
{
  keyboard.timeout = 0;
  keyboard.pressed = 0;
  memset(keyboard.map,0,KEYS_NUM);
}

static int get_key(void)
{
  int current = getch();

  if (current == KEY_QUIT)
    return 0;

  if (current != ERR){
    switch (current){
    case KEY_0: keyboard.map[0x0] = 1; break; 
    case KEY_1: keyboard.map[0x1] = 1; break;
    case KEY_2: keyboard.map[0x2] = 1; break;
    case KEY_3: keyboard.map[0x3] = 1; break;
    case KEY_4: keyboard.map[0x4] = 1; break;
    case KEY_5: keyboard.map[0x5] = 1; break;
    case KEY_6: keyboard.map[0x6] = 1; break;
    case KEY_7: keyboard.map[0x7] = 1; break;
    case KEY_8: keyboard.map[0x8] = 1; break;
    case KEY_9: keyboard.map[0x9] = 1; break;
    case KEY_A: keyboard.map[0xA] = 1; break;
    case KEY_B: keyboard.map[0xB] = 1; break;
    case KEY_C: keyboard.map[0xC] = 1; break;
    case KEY_D: keyboard.map[0xD] = 1; break;
    case KEY_E: keyboard.map[0xE] = 1; break;
    case KEY_16: keyboard.map[0xF] = 1; break;
    default: break;
    }
    keyboard.timeout = KEY_TIME;
  }

  return current;
}

static void key_pressed(void)
{
  int i;
  
  for (i=0;i<KEYS_NUM;i++)
    if (*(keyboard.map+i))
      keyboard.pressed = 1;
}

static void key_timeout(void)
{  
  if (keyboard.timeout > 0)
    keyboard.timeout--;
  
  if (keyboard.timeout == 0){
    memset(keyboard.map,0,KEYS_NUM);
    keyboard.pressed = 0;
  }
}

int keyboard_manager(void)
{
  int res = get_key();
  key_pressed();
  key_timeout();
  
  return res;  
}
