#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ncurses.h>
#include <iconv.h>
#include <locale.h>
#include <caca.h>

#include "display.h"

/* ncurses global vars */
static WINDOW *main_window;
static WINDOW *disp_window;

/* caca gloval vars */
static caca_canvas_t *cv;
static caca_display_t *dp;

static int disp_driver = NCURSES_DRIVER;

static void ncurses_disp_screen(uint8_t *);
static void ncurses_disp_screen_uncomp(uint8_t *);
static void caca_disp_screen(uint8_t *);
static void caca_disp_screen_uncomp(uint8_t *);
/*  static void (*ncurses_disp)(uint8_t *) = ncurses_disp_screen_uncomp;
    static void (*caca_disp)(uint8_t *) = caca_disp_screen_uncomp; */
static void (*disp_s)(uint8_t *) = ncurses_disp_screen_uncomp;

static void ncurses_screen_init(void);
static void caca_screen_init(void);
static void (*disp_init[2])(void) = {ncurses_screen_init, caca_screen_init};

static void ncurses_screen_end(void);
static void caca_screen_end(void);
static void (*disp_end[2])() = {ncurses_screen_end,caca_screen_end};

static void ncurses_refresh_screen(uint8_t *);
static void caca_refresh_screen(uint8_t *);
static void (*disp_refresh[2])(uint8_t *) = {ncurses_refresh_screen,caca_refresh_screen};

static void ncurses_screen_init(void)
{
  setlocale(LC_CTYPE, "");
  main_window = initscr();
  noecho();
  cbreak();
  nodelay(main_window, TRUE);
  refresh();
  wrefresh(main_window);
  disp_window = newwin(35,67,2,2);
  box(disp_window, ACS_VLINE,ACS_HLINE);  
  keypad(stdscr, TRUE);
  curs_set(0);
}

static void caca_screen_init(void)
{
  if ((cv = caca_create_canvas(0, 0)) == NULL)
    {
      fprintf(stderr,"CACA Drivers: canvas error\n");
      exit(CACA_CANV_ERR);
    }
  if((dp = caca_create_display(cv)) == NULL)
    {
      fprintf(stderr,"CACA DRIVER: display error\n");
      exit(CACA_DISP_ERR);
    }
}

void screen_init(void)
{
  disp_init[disp_driver]();
}

/* Must be refactored */
static void caca_disp_screen(uint8_t *disp)
{
  int l=2,c=1, r=0;
  /*  char  *dis; */
  
  for (r = 0; r<64*32; r++, c++){
    if (c > 64){
      c = 1;
      l++;
      r += 64;
    }
    
    switch(*(disp+r) - ((r+64<32*64)?*(disp+r+64):0)){
    case 1: 
      caca_put_str(cv,c,l,"▀");
      break;
    case 0:
      caca_put_str(cv,c,l,(*(disp+r) == 1 ? "█":" "));
      break;
    case -1:
      caca_put_str(cv,c,l,"▄");
    }
    
    /*    caca_put_str(cv,c,l,dis);  */
  }
}

/* Must be refactored */
static void ncurses_disp_screen(uint8_t *disp)
{
  int l=2,c=1, r=0;
  int (*MODE[2])(WINDOW *, int) = {wattroff, wattron};
  char  *dis;
  int choose = 0;
  
  for (r = 0; r<64*32; r++, c++){
    if (c > 64){
      c = 1;
      l++;
      r += 64;
    }
    
    switch(*(disp+r) - *(disp+r+64)){
    case 1: 
      dis = "_";
      choose = 1;
      break;
    case 0:
      dis = " ";
      choose = (r+64<32*64)?*(disp+r) | *(disp+r+64):0;
      break;
    case -1:
      dis =  "\"" ;
      choose = 1;
    }
    /* choose = (r+64<32*64)? *(disp+r) | *(disp+r+64):0; */
    
    MODE[choose](disp_window, A_REVERSE);
    
     mvwaddch(disp_window,l,c,dis[0]); 
     /*mvwprintw(disp_window,l,c,"%s",dis);*/
  }    
}

static void ncurses_disp_screen_uncomp(uint8_t *disp)
{
  int l=2,c=1, r=0;
  int (*MODE[2])(WINDOW *, int) = {wattroff, wattron};

  for (r = 0; r<64*32; r++, c++){
    if (c > 64){
      c = 1;
      l++;
    }

    MODE[*(disp+r)](disp_window, A_REVERSE);
    mvwprintw(disp_window,l,c," ");
  }
}

static void caca_disp_screen_uncomp(uint8_t *disp)
{
  int l=2,c=1, r=0;

  for (r = 0; r<64*32; r++, c++){
    if (c > 64){
      c = 1;
      l++;
    }

    caca_put_str(cv,c,l,*(disp+r)?"█":" "); 
  }
}


void comp_change_disp_mode(void)
{
  if (disp_driver == NCURSES_DRIVER)
    disp_s = ncurses_disp_screen;
  else 
    disp_s = caca_disp_screen;
}


void uncomp_change_disp_mode(void)
{
  if (disp_driver == NCURSES_DRIVER)
    disp_s = ncurses_disp_screen_uncomp;
  else 
    disp_s = caca_disp_screen_uncomp;
}


void change_disp_mode(void)
{
  if (disp_s == ncurses_disp_screen_uncomp 
      || disp_s == caca_disp_screen_uncomp)
    comp_change_disp_mode();
  else
    uncomp_change_disp_mode();
}

void change_driver(void)
{
  disp_driver = CACA_DRIVER;
  if (disp_s == ncurses_disp_screen_uncomp 
      || disp_s == caca_disp_screen_uncomp)
    uncomp_change_disp_mode();
  else
    comp_change_disp_mode();
}

static void ncurses_refresh_screen(uint8_t *disp)
{
  disp_s(disp);
  wrefresh(disp_window);
  refresh();
}

static void caca_refresh_screen(uint8_t *disp)
{
  disp_s(disp);
  caca_refresh_display(dp);
}

void refresh_screen(uint8_t *disp)
{
  disp_refresh[disp_driver](disp);
}

static void ncurses_screen_end(void)
{
  curs_set(1);
  echo();
  nocbreak();
  endwin();
}

static void caca_screen_end(void)
{
  caca_free_display(dp);
  caca_free_canvas(cv);
}

void screen_end(void)
{
  disp_end[disp_driver]();
}

void keyboard_sync(void)
{
  nodelay(main_window, FALSE);
}

void keyboard_async(void)
{
  nodelay(main_window, TRUE);
}


