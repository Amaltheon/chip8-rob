#include <stdio.h>
#include "chip8.h"
#include <SDL2/SDL.h>
#include "constants.h"
#include <time.h>

int running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time = 0;

struct ball {
    float x;
    float x_vel;
    float y;
    float y_vel;
    float width;
    float height;
};

int initialise_window(void) {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr, "Error Initialising SDL.\n");
        return FALSE;
    };
    window = SDL_CreateWindow(
        "Robs Chip8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_BORDERLESS);
    if(!window) {
        fprintf(stderr, "Error creating SDL window.\n");
        return FALSE;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return FALSE;
    }

    return TRUE;
}

void render(struct pixel pixels[64][32]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0 , 255);
    SDL_RenderClear(renderer);

    for(int i=0; i<64; i++) {
        for(int j=0; j<32; j++) {
            if(fb[i][j]) {
                SDL_Rect pixel_rect = {
                    (int)i * PIXEL_SIZE,
                    (int)j * PIXEL_SIZE,
                    PIXEL_SIZE, //width
                    PIXEL_SIZE //height
                };
                SDL_SetRenderDrawColor(renderer, 255,255,255,255);
                SDL_RenderFillRect(renderer, &pixel_rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void setup() {
    loadSprites();
    // init stack pointer
    sp = 0;
    dt = 0;
    st = 0;

    in = 0;

    timer_counter = 0;

    // Setup keyboard
    for(int i=0; i<16; i++) {
        keys[i] = 255;
    }
    // init stack memory
    for(int i=0; i<64; i++) {
        stack[i] = 0;
    }
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void operate(uint16_t opcode, struct pixel pixels[64][32], uint8_t keys[]) {
    //printf("pc = %i\n", pc);
    //printf("opcode: %x \n", opcode);
    //printf("index I = %i \n", in);
    timer_counter++;
    if (timer_counter == 9) {
        if(st > 0) {
            st--;
        }
        if(dt > 0) {
            dt--;
        }
        timer_counter = 0;
        render(pixels);
    }
    uint16_t expression = opcode & 0xF000;
    uint16_t flag = (opcode & 0x000F);
    uint16_t double_flag = (opcode & 0x00FF);
    switch (expression)
    {
    case 0:
        if(opcode == 0x00E0) {
            cls();
            pc+=2;
            //printf("cls\n");
            break;
        }
        else if(opcode == 0x00EE) {
            ret();
            pc+=2;
            //printf("ret\n");
            break;
        }
        else {
            //printf("undefined 0 opcode: %x \n", opcode);
            pc+=2;
            break;
        }
    case 0x1000:
        jp(opcode);
        //printf("jp\n");
        break;
    case 0x2000:
        call(opcode);
        //printf("call\n");
        break;
    case 0x3000:
        se(opcode);
        //printf("se\n");
        pc+=2;
        break;
    case 0x4000:
        sne(opcode);
        //printf("sne\n");
        pc+=2;
        break;
    case 0x5000:
        sexy(opcode);
        //printf("sexy\n");
        pc+=2;
        break;
    case 0x6000:
        ldx(opcode);
        //printf("ldx\n");
        pc+=2;
        break;
    case 0x7000:
        addx(opcode);
        //printf("addx\n");
        pc+=2;
        break;
    case 0x8000:
        if(flag == 0x0000) {
            ldvxvy(opcode);
            //printf("ldvxvy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0001) {
            orxy(opcode);
            //printf("orxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0002) {
            andxy(opcode);
            //printf("andxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0003) {
            xorxy(opcode);
            //printf("xorxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0004) {
            addxy(opcode);
            //printf("addxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0005) {
            subxy(opcode);
            //printf("subxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0006) {
            shrxy(opcode);
            //printf("shrxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0007) {
            subnxy(opcode);
            //printf("subnxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x000E) {
            shlx(opcode);
            //printf("shlx\n");
            pc+=2;
            break;
        }
    case 0x9000:
        snexy(opcode);
        //printf("snexy\n");
        pc+=2;
        break;
    case 0xA000:
        ldi(opcode);
        //printf("ldi\n");
        pc+=2;
        break;
    case 0xB000:
        jpv0(opcode);
        //printf("jpv0\n");
        break;
    case 0xC000:
        rndx(opcode);
        //printf("rndx\n");
        pc+=2;
        break;
    case 0xD000:
        drwxy(opcode);
        //printf("drwxy\n");
        pc+=2;
        break;
    case 0xE000:
        if (double_flag == 0x9E) {
            skpvx(opcode, keys);
            pc+=2;
            break;
        }
        if (double_flag == 0xA1) {
            sknpvx(opcode, keys);
            pc+=2;
            break;
        }
    case 0xF000:
        if (double_flag == 0x07) {
            ldvx(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x0A) {
            ldvx_wait(opcode, keys);
            pc+=2;
            break;
        }
        if (double_flag == 0x15) {
            lddt(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x18) {
            ldst(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x1E) {
            addi(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x29) {
            ldf(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x33) {
            ldb(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x55) {
            ldivx(opcode);
            pc+=2;
            break;
        }
        if (double_flag == 0x65) {
            ldvx65(opcode);
            pc+=2;
            break;
        }

    
    default:
        printf("undefined opcode: %x \n", opcode);
        pc+=2;
        break;
    }
}

int main() {
    cls();
    running = initialise_window();

    uint8_t data[4096];
    FILE *file;

    file = fopen("./roms/space_invaders.ch8", "rb");
    if(file == NULL) {
        printf("Error opening file!");
        return 1;
    }

    fread(data, sizeof(data), 1, file);

    fclose(file);
    for(int i = 0; i < 4096; i++) {
        addrMem[512 + i] = data[i];
    }

    setup();
    struct pixel pixels[64][32];
    while (running) {
        SDL_Delay(1);
        SDL_Event e;
        while( SDL_PollEvent( &e) ) {
            switch( e.type ){
                case SDL_QUIT:
                    running = FALSE;
                    break;
                case SDL_KEYDOWN:
                    printf("Key down, repeat = %i\n", e.key.repeat);
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_ESCAPE:
                        running = FALSE;
                        break;
                    case SDLK_1:
                        keys[1] = 0x1;
                        break;
                    case SDLK_2:
                        keys[2] = 0x2;
                        break;
                    case SDLK_3:
                        keys[3] = 0x3;
                        break;
                    case SDLK_4:
                        keys[0xC] = 0xC;
                        break;
                    case SDLK_q:
                        keys[4] = 0x4;
                        break;
                    case SDLK_w:
                        keys[5] = 0x5;
                        break;
                    case SDLK_e:
                        keys[6] = 0x6;
                        break;
                    case SDLK_r:
                        keys[0xD] = 0xD;
                        break;
                    case SDLK_a:
                        keys[7] = 0x7;
                        break;
                    case SDLK_s:
                        keys[8] = 0x8;
                        break;
                    case SDLK_d:
                        keys[9] = 0x9;
                        break;
                    case SDLK_f:
                        keys[0xE] = 0xE;
                        break;
                    case SDLK_z:
                        keys[0xA] = 0xA;
                        break;
                    case SDLK_x:
                        keys[0] = 0x0;
                        break;
                    case SDLK_c:
                        keys[0xB] = 0xB;
                        break;
                    case SDLK_v:
                        keys[0xF] = 0xF;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_KEYUP:
                    printf("Key up, repeat = %i\n", e.key.repeat);
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_1:
                        keys[1] = 255;
                        break;
                    case SDLK_2:
                        keys[2] = 255;
                        break;
                    case SDLK_3:
                        keys[3] = 255;
                        break;
                    case SDLK_4:
                        keys[0xC] = 255;
                        break;
                    case SDLK_q:
                        keys[4] = 255;
                        break;
                    case SDLK_w:
                        keys[5] = 255;
                        break;
                    case SDLK_e:
                        keys[6] = 255;
                        break;
                    case SDLK_r:
                        keys[0xD] = 255;
                        break;
                    case SDLK_a:
                        keys[7] = 255;
                        break;
                    case SDLK_s:
                        keys[8] = 255;
                        break;
                    case SDLK_d:
                        keys[9] = 255;
                        break;
                    case SDLK_f:
                        keys[0xE] = 255;
                        break;
                    case SDLK_z:
                        keys[0xA] = 255;
                        break;
                    case SDLK_x:
                        keys[0] = 255;
                        break;
                    case SDLK_c:
                        keys[0xB] = 255;
                        break;
                    case SDLK_v:
                        keys[0xF] = 255;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
            }
        }
        uint16_t opcode = (addrMem[pc] << 8 ) + addrMem[pc+1];
        operate(opcode, pixels, keys);
        //update(pixels);
    }


    destroy_window();
    return 0;
}