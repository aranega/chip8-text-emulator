#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#include "tc8e.h"
#include "display.h"
#include "keyboard.h"

#define OP_16(op1,op2,op3,op4) (op1<<12 | op2<<8 | op3<<4 | op4)
#define OP_12(op1,op2,op3)     (op1<<8 | op2<<4 | op3)
#define OP_8(op1,op2)          (op1<<4 | op2)
#define OP1(inst_16)           ((inst_16 & 0xF000)>>12)
#define OP2(inst_16)           ((inst_16 & 0x0F00)>>8)
#define OP3(inst_16)           ((inst_16 & 0x00F0)>>4)
#define OP4(inst_16)           (inst_16 & 0x000F)


static chip8 cpu;
static uint8_t mem[MEM_SIZE];
static uint8_t display[DISP_SIZE];

static uint8_t op1, op2, op3, op4;
static int cycle;
static unsigned int rom_size;

static int debug = (0==1);

/* All in one file, yippi! -_-' */

void init_chip8(void){
  memset(mem,0,MEM_SIZE);
  memset(display,0,DISP_SIZE);
  memset(cpu.V,0,REGS_NUM);
  memset(cpu.stack,0,STACK_SIZE);

  cpu.SP = 0xFF;
  cpu.PC = MEM_ROM_ADRESS;
  srand(time(NULL));
}

void load_rom(char *path)
{
  FILE *rom;

  if (!(rom = fopen(path,"r"))){
    fprintf(stderr,"File %s not found\n",path);
    exit(FILE_NOT_FOUND);
  }

  fread(mem+MEM_ROM_ADRESS,sizeof(uint8_t),MEM_SIZE-MEM_ROM_ADRESS,rom);
  rom_size = ftell(rom);

  fclose(rom);
}

int read_opcode(void)
{
  uint16_t inst;

  if (cpu.PC == MEM_SIZE)
    return EOF;

  if (cpu.PC == rom_size)
    return END_ROM;

  inst = (mem[cpu.PC]<<8) | mem[cpu.PC+1];
  op1 = OP1(inst);
  op2 = OP2(inst);
  op3 = OP3(inst);
  op4 = OP4(inst);
  cpu.PC += 2;

  return cpu.PC;
}

void print_mem(unsigned int n)
{
  unsigned int i, line = n;

  for (i=MEM_ROM_ADRESS;i<=rom_size+MEM_ROM_ADRESS;i+=2){
    if (line == n) printf("Adress %x:\t",i);
    if (line == 0){
      printf("\nAdress %x:\t",i);
      line=n;
    }
    printf("%x%x\t",mem[i],mem[i+1]);
    line--;
  }
}

void print_disp(unsigned int n, int pp)
{
  unsigned int i, col = n;

  for (i=0;i<DISP_SIZE;i++){
    if (col == 0){
      printf("\n");
      col=n;
    }
    if (pp)
      printf(display[i]==1?"#":" ");
    else
      printf("%x",display[i]);
    col--;
  }
  printf("\n");
}


void exec_cls()
{
  memset(display,0,DISP_SIZE);
  if (debug) printf("CLS\n");
}

void exec_ret()
{
  cpu.PC = cpu.stack[cpu.SP];
  cpu.SP--;
  if (debug) printf("RET | PC=%x, SP=%x\n",cpu.PC,cpu.SP);
}

void exec_jmp()
{
  cpu.PC = OP_12(op2,op3,op4);
  if (debug) printf("JP %x | PC=%x\n", OP_12(op2,op3,op4), cpu.PC);
}

void exec_call()
{
  cpu.SP++;
  cpu.stack[cpu.SP] = cpu.PC;
  cpu.PC = OP_12(op2,op3,op4);
  if (debug) printf("CALL %x | PC=%x, SP=%x, stack=%x\n",OP_12(op2,op3,op4),cpu.PC,cpu.SP,cpu.stack[cpu.SP]);
}

/* To Check */
void exec_se_v()
{
  if (cpu.V[op2] == OP_8(op3,op4))
    cpu.PC += 2;
  if (debug) printf("SE V%x, %x | PC=%x\n",op2,OP_8(op3,op4),cpu.PC);
}

/* To check */
void exec_sne_v()
{
  if (cpu.V[op2] != OP_8(op3,op4))
    cpu.PC += 2;
  if (debug) printf("SNE V%x, %x | PC=%x\n",op2,OP_8(op3,op4),cpu.PC);
}

/* To check */
void exec_se_v_v()
{
  if (cpu.V[op2] == cpu.V[op3])
    cpu.PC += 2;
  if (debug) printf("SE V%x, V%x | PC=%x\n",op2,op3,cpu.PC);
}

void exec_ld_v()
{
  cpu.V[op2] = OP_8(op3,op4);
  if (debug) printf("LD V%x, %x | V%x=%x\n",op2,OP_8(op3,op4),op2,cpu.V[op2]);
}

void exec_add_v()
{
  cpu.V[op2] += OP_8(op3,op4);
  if (debug) printf("ADD V%x, %x | V%x=%x\n",op2,OP_8(op3,op4),op2,cpu.V[op2]);
}

void exec_ld_v_v()
{
  cpu.V[op2] = cpu.V[op3];
  if (debug) printf("LD V%x, V%x\n",op2,op3);
}

void exec_or_v_v()
{
  cpu.V[op2] |= cpu.V[op3];
  if (debug) printf("OR V%x, V%x\n",op2,op3);
}

void exec_and_v_v()
{
  cpu.V[op2] &= cpu.V[op3];
  if (debug) printf("OR V%x, V%x\n",op2,op3);
}

void exec_xor_v_v()
{
  cpu.V[op2] ^= cpu.V[op3];
  if (debug) printf("XOR V%x, V%x\n",op2,op3);
}

/* To check */
void exec_add_v_v()
{
  if (cpu.V[op2]+cpu.V[op3] > 0xFF)
    cpu.V[0x0F] = 1;
  else
    cpu.V[0x0F] = 0;
  cpu.V[op2] += cpu.V[op3];

  if (debug) printf("ADD V%x, V%x | %x \n",op2,op3, cpu.V[op2]);
}

/* To check */
void exec_sub_v_v()
{
  if (cpu.V[op2] > cpu.V[op3])
    cpu.V[0x0F] = 1;
  else
    cpu.V[0x0F] = 0;
  cpu.V[op2] = cpu.V[op2] - cpu.V[op3];
  if (debug) printf("SUB V%x, V%x | %x \n",op2,op3, cpu.V[op2]);
}

/* To check */
void exec_shr_v_v()
{
  cpu.V[0x0F] = cpu.V[op2] & 0x01;
  cpu.V[op2] >>= 1;
  if (debug) printf("SHR V%x\n",op2);
}

/* To check */
void exec_subn_v_v()
{
  if (cpu.V[op2] < cpu.V[op3])
    cpu.V[0x0F] = 1;
  else
    cpu.V[0x0F] = 0;
  cpu.V[op2] = cpu.V[op3] - cpu.V[op2];
  if (debug) printf("SUBN V%x, V%x | %x \n",op2,op3, cpu.V[op2]);
}

/* To check */
void exec_shl_v_v()
{
  cpu.V[0x0F] = (cpu.V[op2] & 0x80)>>7;
  cpu.V[op2] <<= 1;
  if (debug) printf("SHl V%x\n",op2);
}

/* To check */
void exec_sne_v_v()
{
  if (cpu.V[op2] != cpu.V[op3])
    cpu.PC +=2;
  if (debug) printf("SNE V%x, V%x\n",op2,op3);
}

void exec_ld_i()
{
  cpu.I = OP_12(op2,op3,op4);
  if (debug) printf("LD I, %x\n", OP_12(op2,op3,op4));
}

void exec_jmp_v()
{
  cpu.PC = cpu.V[0x00] + OP_12(op2,op3,op4);
  if (debug) printf("JP V0, %x\n", OP_12(op2,op3,op4));
}

/* To check */
void exec_rnd_v()
{
  cpu.V[op2] = (rand() % 256) & OP_8(op3,op4);
  if (debug) printf("RND V%x, %x | rnd = %x\n",op2,OP_8(op3,op4),cpu.V[op2]);
}

/* To check, must be refactored */
void exec_drw_v_v()
{
  int l,c;
  uint8_t gfx,x,y;

  for (l = 0; l < op4; l++){
    gfx = mem[cpu.I+l];
    y = (cpu.V[op3] + l);

    for (c = 0; c < 8; c++){
      x = (cpu.V[op2]+c);
      if ((gfx & (0x80 >> c)) != 0){
	if (display[((y%32)*64)+(x%64)] == 1)
	  cpu.V[0x0F]=1;

	display[((y%32)*64) + (x%64)] ^= 1;
      }
    }
  }
  /* refresh_screen(display); */
  if (debug) printf("DRW V%x, V%x, %x\n",op2,op3,op4);
}

/* TO COMPLETE */
void exec_skp_v()
{
  if (keyboard.map[cpu.V[op2]])
    cpu.PC += 2;

  if (debug) printf("SKP V%x\n",op2);
}

/* TO COMPLETE */
void exec_sknp_v()
{
  if (!keyboard.map[cpu.V[op2]])
    cpu.PC += 2;
  if (debug) printf("SKNP V%x\n",op2);
}

void exec_ld_v_t()
{
  cpu.V[op2] = cpu.delay;
  if (debug) printf("LD V%x, DT\n",op2);
}


/* To be refactored */
void exec_ld_v_k()
{
  int key, value;
  keyboard_sync();
  switch(key=getch()){
  case KEY_UP: value=0; break;
  case 'd': value=1; break;
  case 'u': value=2; break;
  case KEY_DOWN: value=3; break;
  case KEY_LEFT: value=4; break;
  case 'y': value=5; break;
  case KEY_RIGHT: value=6; break;
  case 'g': value=7; break;
  case 'h': value=8; break;
  case 'j': value=9; break;
  case 'k': value=10; break;
  case 'l': value=11; break;
  case 'n': value=12; break;
  case ',': value=13; break;
  case ';': value=14; break;
  case ':': value=16; break;
  default: value = 0;
  };
  cpu.V[op2] = value;
  if (debug) printf("LD V%x, k\n",op2);
  keyboard_async();
}

void exec_ld_dt_v()
{
  cpu.delay = cpu.V[op2];
  if (debug) printf("LD DT,V%x\n",op2);
}

void exec_ld_st_v()
{
  cpu.sound = cpu.V[op2];
  if (debug) printf("LD ST,V%x\n",op2);
}

void exec_add_i_v()
{
  cpu.I += cpu.V[op2];
  if (debug) printf("ADD I, V%x\n",op2);
}

void exec_ld_f_v()
{
  cpu.I = cpu.V[op2];
  if (debug) printf("LD F,V%x\n",op2);
}

/* To check */
void exec_ld_b_v()
{
  uint8_t h,t,l;

  h = cpu.V[op2] / 100;
  t = (cpu.V[op2] / 100)%10;
  l = cpu.V[op2]%10;

  mem[cpu.I] = h;
  mem[cpu.I+1] = t;
  mem[cpu.I+2] = l;
  if (debug) printf("LD B, V%x | val = %d\n",op2,cpu.V[op2]);
}

/* To check */
void exec_ld_ai_v()
{
  memcpy(mem+cpu.I,cpu.V,(op2+1)*2);
  if (debug) printf("LD [I], V%x | %x%x\n",op2,mem[cpu.I],mem[cpu.I+1]);
}

/* To check */
void exec_ld_v_ai()
{
  memcpy(cpu.V,mem+cpu.I,(op2+1)*2);
  if (debug) printf("LD V%x,[I] | %x%x\n",op2,mem[cpu.I],mem[cpu.I+1]);
}


/* To be refactored */
void execute_opcode()
{
  /* printf("%x: ",cpu.PC-2); */
  if (OP_16(op1,op2,op3,op4) == 0x00E0)
    exec_cls();
  else if (OP_16(op1,op2,op3,op4) == 0x00EE)
    exec_ret();
  else {
    switch(op1){
    case 0x01: exec_jmp(); break;
    case 0x02: exec_call(); break;
    case 0x03: exec_se_v(); break;
    case 0x04: exec_sne_v(); break;
    case 0x05: exec_se_v_v(); break;
    case 0x06: exec_ld_v(); break;
    case 0x07: exec_add_v(); break;
    case 0x08:
      switch(op4){
      case 0x00: exec_ld_v_v(); break;
      case 0x01: exec_or_v_v(); break;
      case 0x02: exec_and_v_v(); break;
      case 0x03: exec_xor_v_v(); break;
      case 0x04: exec_add_v_v(); break;
      case 0x05: exec_sub_v_v(); break;
      case 0x06: exec_shr_v_v(); break;
      case 0x07: exec_subn_v_v(); break;
      case 0x0E: exec_shl_v_v(); break;
      default: printf("%x: Error 0x08 ?\n",cpu.PC-2);
      } break;
    case 0x09: exec_sne_v_v();  break;
    case 0x0A: exec_ld_i(); break;
    case 0x0B: exec_jmp_v(); break;
    case 0x0C: exec_rnd_v(); break;
    case 0x0D: exec_drw_v_v(); break;
    case 0x0E:
      switch(OP_8(op3,op4)){
      case 0x9E: exec_skp_v(); break;
      case 0xA1: exec_sknp_v(); break;
      default: printf("%x: Error 0x0E ?\n",cpu.PC-2);
      } break;
    case 0x0F:
      switch(OP_8(op3,op4)){
      case 0x07: exec_ld_v_t(); break;
      case 0x0A: exec_ld_v_k(); break;
      case 0x15: exec_ld_dt_v(); break;
      case 0x18: exec_ld_st_v(); break;
      case 0x1E: exec_add_i_v(); break;
      case 0x29: exec_ld_f_v(); break;
      case 0x33: exec_ld_b_v(); break;
      case 0x55: exec_ld_ai_v(); break;
      case 0x65: exec_ld_v_ai(); break;
      default: printf("%x: Error 0x0F ?\n",cpu.PC-2); break;
      } break;
      /* default: printf("%x\n",cpu.PC-2); break; */
    }
  }
}

void step_by_step()
{
  char cmd[80], p1[80], buff[80];
  unsigned int hex;

  printf("cmd> ");
  fgets(buff,80*sizeof(char),stdin);
  sscanf(buff,"%s",cmd);

  if (!strcmp(cmd,"next") || !strcmp(cmd,"n"))
    return;
  if (!strcmp(cmd,"quit") || !strcmp(cmd,"q"))
    exit(EXIT_SUCCESS);

  if (!strcmp(cmd,"help") || !strcmp(cmd,"h"))
    {
      printf("Debugger help\n");
      printf(" next/n         -> go to the next instruction\n");
      printf(" print [option] -> print stuff (print help for help ;) )\n");
      printf(" quit/q         -> quit");
    }

  if (!strcmp(cmd,"print")){
    sscanf(buff+strlen(cmd),"%s",p1);
    if (*p1 == 'V' || *p1 == 'v'){
      sscanf(p1+1,"%x",&hex);
      if (hex > 0xF) printf("Error, there is only 0xF registers\n");
      else printf("Register V%x = %x\n",hex,cpu.V[hex]);
    } else if (!strcmp(p1,"PC") || !strcmp(p1,"pc")){
      printf("PC = %x\n",cpu.PC);
    } else if (!strcmp(p1,"SP") ||!strcmp(p1,"sp") ){
      printf("SP = %x\n",cpu.SP);
    } else if (!strcmp(p1,"I") || !strcmp(p1,"i")){
      printf("I = %x\n",cpu.I);
    } else if (!strcmp(p1,"delay") || !strcmp(p1,"d")){
      printf("Delay = %x\n",cpu.delay);
    } else if (!strcmp(p1,"sound") || !strcmp(p1,"s")){
      printf("Sound = %x\n",cpu.sound);
    } else if (!strcmp(p1,"stack")){
      printf("Stack = ");
      for (hex = 0; hex <= 0x0F; hex++)
	printf("%x ",cpu.stack[hex]);
      printf("\n");
    }else if (!strcmp(p1,"regs")){
      printf("regs : ");
      for (hex = 0; hex <= 0x0F; hex++)
	printf("V%x=%x ",hex,cpu.V[hex]);
      printf("\n");
    } else if (!strcmp(p1,"freq")){
      printf("freq : %d\n",cycle);
    } else if (!strcmp(p1,"mem")){
      print_mem(8);
      putchar('\n');
    } else if (!strcmp(p1,"disp")){
      print_disp(64,0);
    } else if (!strcmp(p1,"screen")){
      print_disp(64,1);
    } else if (!strcmp(p1,"help")){
      printf("print [option]\n");
      printf("option:\n");
      printf(" PC     -> print Program Counter\n");
      printf(" Vx     -> print register x\n");
      printf(" SP     -> print Stack Pointer\n");
      printf(" I      -> print I reg\n");
      printf(" delay  -> print Delay value\n");
      printf(" sound  -> print Sound value\n");
      printf(" stack  -> print stack\n");
      printf(" regs   -> print regs\n");
      printf(" freq   -> print cycle\n");
      printf(" mem    -> print mem\n");
      printf(" disp   -> print disp mem\n");
      printf(" screen -> print screen\n");
      printf(" xxx -> print byte at xxx (hexa)\n");
      printf(" help   -> print this help\n");
     }
    else {
      sscanf(p1,"%x",&hex);
      printf("MEM @%x: ",hex);
      printf("%x%x\n",mem[hex],mem[hex+1]);
    }


  } else
    printf("Command %s unknown\n",cmd);
  step_by_step();
}

void print_help(void)
{
  printf("TC8E (Text mode Chip8 Emulator)\n");
  printf(" Usage: TC8E {option} ROM_FILE\n");
  printf("  -g \t Debug mode, in debug mode use \'help\' command to see aliases\n");
  exit(0);
}

int main(int argc, char *argv[])
{
  int debug_mode = 0;
  int looping = 1;
  int delay_emu = DELAY;
  int next = 1;

  if (argc < 2){
    fprintf(stderr,"Usage: %s ROM_FILE\n",argv[0]);
    return BAD_USAGE;
  }

  init_chip8();

  for (;next<argc;next++)
    if (!strcmp(argv[next],"-g")){
      debug_mode = 1;
      debug = (0==0);
    }
    else if (!strcmp(argv[next], "-ad"))
      change_disp_mode();
    else if (!strcmp(argv[next], "-caca"))
      change_driver();
    else if (!strcmp(argv[next],"-h")
	     || !strcmp(argv[next], "--help"))
      print_help();

  load_rom(argv[next-1]);

  if (!debug_mode)
    screen_init();
  /* Must be refactored */
  /* while (read_opcode()-0x200 <= rom_size){ */
  while (looping){
    /* printf("%x\t%x%x %x%x\n",cpu.PC-2,op1,op2,op3,op4); */
    for (cycle=0;cycle<FREQ && looping;cycle++){
      switch (looping = keyboard_manager()){
      case KEY_USPEED:
	delay_emu -= DELAY_STEP;
	break;
      case KEY_DSPEED:
	delay_emu += DELAY_STEP;
	break;
      }

      read_opcode();
      execute_opcode();

      if (!debug_mode)
	refresh_screen(display);

      if (debug_mode)
	step_by_step();
    }

    if (cpu.delay > 0)
      cpu.delay--;

    if (cpu.sound > 0)
      cpu.sound--;

    if (!debug_mode)
      switch (looping = keyboard_manager()){
      case KEY_USPEED:
	delay_emu -= DELAY_STEP;
	break;
      case KEY_DSPEED:
	delay_emu += DELAY_STEP;
	break;
      }
      usleep(delay_emu);
  }

  if (!debug_mode)
    screen_end();
  return 0;
}
