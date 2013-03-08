#ifndef __CHIP8_H__
#define __CHIP8_H__

#define DELAY                   50
#define DELAY_STEP              500000

#define BAD_USAGE              (1)
#define FILE_NOT_FOUND         (2)

#define END_ROM                (-2)
#define END_FILE               (-1)

#define MEM_SIZE               4096
#define REGS_NUM               16
#define STACK_SIZE             16
#define DISP_SIZE              64*32

#define MEM_ROM_ADRESS         0x200

#define FREQ                   50

typedef struct{
  uint8_t V[REGS_NUM];
  uint16_t I;
  uint8_t delay,sound;
  uint16_t PC;
  uint8_t SP;
  uint16_t stack[STACK_SIZE];
} chip8;


#endif
