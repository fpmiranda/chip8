#ifndef CPU_H
#define CPU_H

#include "raylib.h"

typedef unsigned char u8;
typedef unsigned short u16;

typedef enum {
    RUNNING,
    PAUSED,
    QUIT,
} state_t;

typedef struct
{
    u16 opcode;
    u16 NNN;
    u8 NN;
    u8 N;
    u8 X;
    u8 Y;    
} inst_t;

typedef struct
{
    inst_t inst;
    state_t state;
    u8 V[16];
    u16 Address_I;
    u16 PC;
    u16 Stack[12];
    u16 *stk_ptr;
    u8 prev_display[64*32];
    u8 display[64*32];
    u8 mem[4096];
    bool keypad[16];
    u8 delay_t;
    u8 sound_t;
} cpu_t;

int init_cpu(cpu_t *cpu, const char rom_path[]);

void run_instruction(cpu_t *cpu);

#endif