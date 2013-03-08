#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define NCURSES_DRIVER 0
#define CACA_DRIVER 1

#define CACA_CANV_ERR (-127)
#define CACA_DISP_ERR (-126)

extern void screen_init(void);
extern void screen_end(void);
extern void refresh_screen(uint8_t *disp);
extern void keyboard_sync(void);
extern void keyboard_async(void); 
extern void change_disp_mode(void);
extern void change_driver(void);



#endif
