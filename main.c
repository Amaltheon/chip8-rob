#include <stdio.h>
#include "chip8.h"
#include <SDL2/SDL.h>
#include "constants.h"

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

uint8_t process_input() {
    SDL_Event event;
    SDL_PollEvent(&event);

    uint8_t current_key;

    switch (event.type)
    {
    case SDL_QUIT:
        running = FALSE;
        break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            running = FALSE;
        case SDLK_0:
            current_key = 0x0;
        case SDLK_1:
            current_key = 0x1;
        case SDLK_2:
            current_key = 0x2;
        case SDLK_3:
            current_key = 0x3;
        case SDLK_4:
            current_key = 0x4;
        case SDLK_5:
            current_key = 0x5;
        case SDLK_6:
            current_key = 0x6;
        case SDLK_7:
            current_key = 0x7;
        case SDLK_8:
            current_key = 0x8;
        case SDLK_9:
            current_key = 0x9;
        case SDLK_a:
            current_key = 0xA;
        case SDLK_b:
            current_key = 0xB;
        case SDLK_c:
            current_key = 0xC;
        case SDLK_d:
            current_key = 0xD;
        case SDLK_e:
            current_key = 0xE;
        case SDLK_f:
            current_key = 0xF;
        default:
            break;
        }
    }

    return current_key;
}

void update(struct pixel pixels[64][32]) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    
    last_frame_time = SDL_GetTicks();
    for(int i=0; i<64; i++) {
        for(int j=0; j<32; j++) {
            if(fb[i][j]) {
                pixels[i][j].x = i;
                pixels[i][j].y = j;
                pixels[i][j].on = true;
            }
            else {
                pixels[i][j].x = i;
                pixels[i][j].y = j;
                pixels[i][j].on = false;
            }
        }
    }
}

void render(struct pixel pixels[64][32]) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0 , 255);
    SDL_RenderClear(renderer);

    for(int i=0; i<64; i++) {
        for(int j=0; j<32; j++) {
            if(pixels[i][j].on) {
                SDL_Rect pixel_rect = {
                    (int)pixels[i][j].x * PIXEL_SIZE,
                    (int)pixels[i][j].y * PIXEL_SIZE,
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
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void operate(uint16_t opcode, struct pixel pixels[64][32], uint8_t current_key) {
    printf("pc = %i\n", pc);
    printf("opcode: %x \n", opcode);
    printf("index I = %i \n", in);
    uint16_t expression = opcode & 0xF000;
    uint16_t flag = (opcode & 0x000F);
    uint16_t double_flag = (opcode & 0x00FF);
    switch (expression)
    {
    case 0:
        if(opcode == 0x00E0) {
            cls();
            pc+=2;
            printf("cls\n");
            break;
        }
        else if(opcode == 0x00EE) {
            ret();
            pc+=2;
            printf("ret\n");
            break;
        }
        else {
            printf("undefined 0 opcode: %x \n", opcode);
            pc+=2;
            break;
        }
    case 0x1000:
        jp(opcode);
        printf("jp\n");
        break;
    case 0x2000:
        call(opcode);
        printf("call\n");
        break;
    case 0x3000:
        se(opcode);
        printf("se\n");
        pc+=2;
        break;
    case 0x4000:
        sne(opcode);
        printf("sne\n");
        pc+=2;
        break;
    case 0x5000:
        sexy(opcode);
        printf("sexy\n");
        pc+=2;
        break;
    case 0x6000:
        ldx(opcode);
        printf("ldx\n");
        pc+=2;
        break;
    case 0x7000:
        addx(opcode);
        printf("addx\n");
        pc+=2;
        break;
    case 0x8000:
        if(flag == 0x0000) {
            ldvxvy(opcode);
            printf("ldvxvy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0001) {
            orxy(opcode);
            printf("orxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0002) {
            andxy(opcode);
            printf("andxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0003) {
            xorxy(opcode);
            printf("xorxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0004) {
            addxy(opcode);
            printf("addxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0005) {
            subxy(opcode);
            printf("subxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0006) {
            shrxy(opcode);
            printf("shrxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x0007) {
            subnxy(opcode);
            printf("subnxy\n");
            pc+=2;
            break;
        }
        else if(flag == 0x000E) {
            shlx(opcode);
            printf("shlx\n");
            pc+=2;
            break;
        }
    case 0x9000:
        snexy(opcode);
        printf("snexy\n");
        pc+=2;
        break;
    case 0xA000:
        ldi(opcode);
        printf("ldi\n");
        pc+=2;
        break;
    case 0xB000:
        jpv0(opcode);
        printf("jpv0\n");
        break;
    case 0xC000:
        rndx(opcode);
        printf("rndx\n");
        pc+=2;
        break;
    case 0xD000:
        drwxy(opcode);
        printf("drwxy\n");
        pc+=2;
        break;
    case 0xE000:
        if (double_flag == 0x9E) {
            skpvx(opcode, current_key);
            pc+=2;
            break;
        }
        if (double_flag == 0xA1) {
            sknpvx(opcode, current_key);
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
            ldvx_wait(opcode, current_key);
            if (current_key) {
                pc+=2;
            }
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

    file = fopen("./roms/3-corax+.ch8", "rb");
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
        uint8_t current_key = process_input();
        uint16_t opcode = (addrMem[pc] << 8 ) + addrMem[pc+1];
        operate(opcode, pixels, current_key);
        update(pixels);
        render(pixels);
    }

    destroy_window();
    return 0;
}