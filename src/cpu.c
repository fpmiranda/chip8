#include "../include/cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\include\cpu.h"

int init_cpu(cpu_t *cpu, const char rom_path[]){
    const u16 entry_point = 0x0200;
    const u8 font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0   
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1  
        0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2 
        0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,   // 4    
        0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xF0, 0x80, 0x80,   // F
    };
    
    memset(cpu, 0, sizeof(cpu_t));
    memcpy(&cpu->mem[0], font, sizeof(font));

    FILE *rom = fopen(rom_path, "rb");

    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom);
    rewind(rom);

    if(fread(&cpu->mem[entry_point], rom_size, 1, rom) != 1){
        printf("Could not read Rom file into memory.");
    }

    fclose(rom);

    cpu->stk_ptr = &cpu->Stack[0];
    cpu->PC = entry_point;
    cpu->state = RUNNING;

    return 1;
}

//Adapted from https://www.arjunnair.in/p37/
void draw_sprite(cpu_t *cpu){
    cpu->V[0xF] = 0;

    for(int i=0; i < (cpu->inst.N) ; i++){
        u8 sprite = cpu->mem[cpu->Address_I + i];
        int row = (cpu->V[cpu->inst.Y] + i) % 32;

        for(int j=0; j<=8; j++){
            int b = (sprite & 0x80) >> 7;
            int col = (cpu->V[cpu->inst.X] + j) % 64;
            int offset = row * 64 + col;

            if(b==1){
                if(cpu->display[offset] != 0x0){
                    cpu->display[offset] = 0x0;
                    cpu->V[0xF] = 0x1;
                }else cpu->display[offset] = 0x1;
            }
            sprite <<= 1;
        }
    }
}

void run_instruction(cpu_t *cpu){
    bool carry;

    cpu->inst.opcode = (cpu->mem[cpu->PC] << 8) | cpu->mem[cpu->PC+1];
    cpu->PC+=2;

    cpu->inst.NNN = cpu->inst.opcode & 0x0FFF;
    cpu->inst.NN = cpu->inst.opcode & 0x0FF;
    cpu->inst.N = cpu->inst.opcode & 0x0F;
    cpu->inst.X = (cpu->inst.opcode >> 8) & 0x0F;
    cpu->inst.Y = (cpu->inst.opcode >> 4) & 0x0F;

    switch (((cpu->inst.opcode >> 12) & 0x0F))
    {
        case 0x00:
            if(cpu->inst.NN == 0xE0) memset(&cpu->display[0], false, sizeof cpu->display);
            else if(cpu->inst.NN == 0xEE) cpu->PC = *--cpu->stk_ptr;
        break;

        case 0x01:
            cpu->PC = cpu->inst.NNN;
        break;

        case 0x02:
            *cpu->stk_ptr++ = cpu->PC;  
            cpu->PC = cpu->inst.NNN;
        break;

        case 0x03:
            if(cpu->V[cpu->inst.X] == cpu->inst.NN) cpu->PC+=2;
        break;
        
        case 0x04:
            if(cpu->V[cpu->inst.X] != cpu->inst.NN) cpu->PC+=2;
        break;
        
        case 0x05:
            if(cpu->V[cpu->inst.X] == cpu->V[cpu->inst.Y]) cpu->PC+=2;
        break;
        
        case 0x06:
            cpu->V[cpu->inst.X] = cpu->inst.NN;
        break;

        case 0x07:
            cpu->V[cpu->inst.X] += cpu->inst.NN;
        break;

        case 0x08:
            switch (cpu->inst.N){
                case 0:
                    cpu->V[cpu->inst.X] = cpu->V[cpu->inst.Y];
                break;

                case 1:
                    cpu->V[cpu->inst.X] |= cpu->V[cpu->inst.Y];
                    cpu->V[0xF] = 0;
                break;

                case 2:
                    cpu->V[cpu->inst.X] &= cpu->V[cpu->inst.Y];
                    cpu->V[0xF] = 0;
                break;

                case 3:
                    cpu->V[cpu->inst.X] ^= cpu->V[cpu->inst.Y];
                    cpu->V[0xF] = 0;
                break;

                case 4:
                    carry = (cpu->V[cpu->inst.X] + cpu->V[cpu->inst.Y]) > 256;

                    cpu->V[cpu->inst.X] += cpu->V[cpu->inst.Y];
                    cpu->V[0xF] = carry;
                break;
                
                case 5:
                    carry = cpu->V[cpu->inst.Y] <= cpu->V[cpu->inst.X];

                    cpu->V[cpu->inst.X] -= cpu->V[cpu->inst.Y];
                    cpu->V[0xF] = carry;
                break;

                case 6:
                    carry = cpu->V[cpu->inst.Y] & 1;
                    cpu->V[cpu->inst.X] = cpu->V[cpu->inst.Y] >> 1;

                    cpu->V[0xF] = carry;
                break;

                case 7:
                    carry = (cpu->V[cpu->inst.X] <= cpu->V[cpu->inst.Y]);

                    cpu->V[cpu->inst.X] = cpu->V[cpu->inst.Y] - cpu->V[cpu->inst.X];
                    cpu->V[0xF] = carry;
                break;

                case 0xE:
                    carry = (cpu->V[cpu->inst.Y] & 0x80) >> 7;
                    cpu->V[cpu->inst.X] = cpu->V[cpu->inst.Y] << 1;

                    cpu->V[0xF] = carry;
                break;

                default:
                    break;
                }
        break;

        case 0x09:
            if(cpu->V[cpu->inst.X] != cpu->V[cpu->inst.Y]) cpu->PC+=2;
        break;

        case 0x0A:
            cpu->Address_I = cpu->inst.NNN;
        break;

        case 0x0B:
            cpu->PC = cpu->inst.NNN + cpu->V[0];
        break;

        case 0x0C:
            cpu->V[cpu->inst.X] = (rand() % 256) & cpu->inst.NN;
        break;

        case 0x0D:
            draw_sprite(cpu);
        break;

        case 0x0E:
            if(cpu->inst.NN == 0x9E) {
                if(cpu->keypad[cpu->V[cpu->inst.X]]) 
                    cpu->PC += 2;
            }
            else if(cpu->inst.NN == 0xA1) {
                if(!cpu->keypad[cpu->V[cpu->inst.X]]) 
                    cpu->PC += 2;
            }
        break;

        case 0x0F:
            switch (cpu->inst.NN)
            {
                case 0x07:
                    cpu->V[cpu->inst.X] = cpu->delay_t;
                break;
                
                case 0x15:
                    cpu->delay_t = cpu->V[cpu->inst.X];
                break;

                case 0x18:
                    cpu->sound_t = cpu->V[cpu->inst.X];
                break;

                case 0x1E:
                    cpu->Address_I += cpu->V[cpu->inst.X]; 
                break;

                case 0x0A:
                    bool isPressed = false;
                    u8 key = 0xFF;

                    for(u8 i = 0; key == 0xFF && i < sizeof(cpu->keypad); i++ ){
                        if(cpu->keypad[i]){
                            key = i;
                            isPressed = true;
                            break;
                        }
                    }

                    if(!isPressed) cpu->PC -= 2;
                    else{
                        if (cpu->keypad[key]) 
                            cpu->PC -= 2;
                        else {
                            cpu->V[cpu->inst.X] = key;
                            key = 0xFF;
                            isPressed = false;
                        }
                    }

                break;

                case 0x29:
                    cpu->Address_I = cpu->V[cpu->inst.X] * 5;
                break;

                case 0x33:
                    u8 bcd = cpu->V[cpu->inst.X]; 
                    cpu->mem[cpu->Address_I+2] = bcd % 10;
                    bcd /= 10;
                    cpu->mem[cpu->Address_I+1] = bcd % 10;
                    bcd /= 10;
                    cpu->mem[cpu->Address_I] = bcd;
                break;

                case 0x55:
                    for(u8 i = 0; i <= cpu->inst.X; i++)  {
                        cpu->mem[cpu->Address_I++] = cpu->V[i];
                    }
                break;

                case 0x65:
                    for(u8 i = 0; i<=cpu->inst.X; i++) {
                        cpu->V[i] = cpu->mem[cpu->Address_I++];
                    }
                break;
                
                default:
                    break;
            }
            break;

        default:
        break;
    }


}