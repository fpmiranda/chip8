#include <stdio.h>
#include "raylib.h"
#include <string.h>

#include "../include/cpu.h"

void input_handler(cpu_t *cpu){
    //Read key down
    if(IsKeyDown(KEY_KP_1)) cpu->keypad[0x1] = true;
    if(IsKeyDown(KEY_KP_2)) cpu->keypad[0x2] = true;
    if(IsKeyDown(KEY_KP_3)) cpu->keypad[0x3] = true;
    if(IsKeyDown(KEY_KP_4)) cpu->keypad[0xC] = true;

    if(IsKeyDown(KEY_Q)) cpu->keypad[0x4] = true;
    if(IsKeyDown(KEY_W)) cpu->keypad[0x5] = true;
    if(IsKeyDown(KEY_E)) cpu->keypad[0x6] = true;
    if(IsKeyDown(KEY_R)) cpu->keypad[0xD] = true;

    if(IsKeyDown(KEY_A)) cpu->keypad[0x7] = true;
    if(IsKeyDown(KEY_S)) cpu->keypad[0x8] = true;
    if(IsKeyDown(KEY_D)) cpu->keypad[0x9] = true;
    if(IsKeyDown(KEY_F)) cpu->keypad[0xE] = true;

    if(IsKeyDown(KEY_Z)) cpu->keypad[0xA] = true;
    if(IsKeyDown(KEY_X)) cpu->keypad[0x0] = true;
    if(IsKeyDown(KEY_C)) cpu->keypad[0xB] = true;
    if(IsKeyDown(KEY_V)) cpu->keypad[0xF] = true;
    
    //Read key up
    if(IsKeyUp(KEY_KP_1)) cpu->keypad[0x1] = false;
    if(IsKeyUp(KEY_KP_2)) cpu->keypad[0x2] = false;
    if(IsKeyUp(KEY_KP_3)) cpu->keypad[0x3] = false;
    if(IsKeyUp(KEY_KP_4)) cpu->keypad[0xC] = false;

    if(IsKeyUp(KEY_Q)) cpu->keypad[0x4] = false;
    if(IsKeyUp(KEY_W)) cpu->keypad[0x5] = false;
    if(IsKeyUp(KEY_E)) cpu->keypad[0x6] = false;
    if(IsKeyUp(KEY_R)) cpu->keypad[0xD] = false;

    if(IsKeyUp(KEY_A)) cpu->keypad[0x7] = false;
    if(IsKeyUp(KEY_S)) cpu->keypad[0x8] = false;
    if(IsKeyUp(KEY_D)) cpu->keypad[0x9] = false;
    if(IsKeyUp(KEY_F)) cpu->keypad[0xE] = false;

    if(IsKeyUp(KEY_Z)) cpu->keypad[0xA] = false;
    if(IsKeyUp(KEY_X)) cpu->keypad[0x0] = false;
    if(IsKeyUp(KEY_C)) cpu->keypad[0xB] = false;
    if(IsKeyUp(KEY_V)) cpu->keypad[0xF] = false;
    
}

void renderScreen(cpu_t *cpu, int height, int width){
    int b = -1; //defines the line i'm currently on. used as y scale
    int Xm = -1; //Scale to know where my X is currently on

    if(b == 32) b = -1; //Resets y scale if i'm already outside the defined height

    for (int a = 0; a < 2048; a++){

        if((a%64) == 0) b++; //increase b (y scale)
        if((Xm*10) == (width-10)) Xm = 0; //verify if i'm 10px before the end of the screen. if true, reset Xm
        else Xm++;

        int x = width - ((-10*Xm) + width); //find x position based on scale and width
        int y = height - ((-10*b) + height); //find y position based on scale and height

        if(cpu->display[a] == 0x1) DrawRectangle(x, y, 10, 10, WHITE);
        if(cpu->display[a] == 0x0) DrawRectangle(x, y, 10, 10, BLACK);
    }

}

void update_timers(cpu_t *cpu) {
    if (cpu->delay_t > 0) 
        cpu->delay_t--;

    if (cpu->sound_t > 0) {
        cpu->sound_t--;
        //SDL_PauseAudioDevice(sdl.dev, 0); // Play sound
    } else {
        //SDL_PauseAudioDevice(sdl.dev, 1); // Pause sound
    }
}

int main(int argc, char const *argv[])
{
    int i;
    static const int screenW = 640;
    static const int screenH = 320;
    const char *rom_path = argv[1];
    cpu_t cpu = {0};

    InitWindow(screenW, screenH, "Chip8 Emulator");
    SetTargetFPS(60);

    if(!init_cpu(&cpu, rom_path)) printf("Error starting the emulator.\n");

    while(cpu.state != QUIT && !WindowShouldClose()){
        
        input_handler(&cpu);

        for(i=0; i < 1000/60 ;i++) run_instruction(&cpu);

        BeginDrawing();

        renderScreen(&cpu, screenH, screenW);

        ClearBackground(BLACK);

        EndDrawing();

        update_timers(&cpu);
    }
    
    CloseWindow();

    return 0;
}
