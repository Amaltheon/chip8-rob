#include "chip8.h"

uint8_t V[15];
uint16_t in;
uint8_t stack[64];
uint8_t sp;
uint8_t dt;
uint8_t st;
uint64_t fb[32];
uint16_t pc;

uint8_t addrMem[4096];

void loadSprites(){
    in = 0;
    addrMem[0x000] = 0xF0;
    addrMem[0x001] = 0x90;
    addrMem[0x002] = 0x90;
    addrMem[0x003] = 0x90;
    addrMem[0x004] = 0xF0; 
}



// 00E0 Clears Display, doubles as init for screen
void cls()
{
    for(int i=0; i<32; i++) {
        fb[i] = 0x0000000000000000;
    }
}

// 00EE Return from a subroutine
void ret()
{
    pc = stack[sp];
    sp--;
}

// 1nnn Jump to location nnn (opcode = 1nnn)
void jp(uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0FFF;
    pc = nnn;
}

// 2nnn Call subroutine at nnn
void call(uint16_t opcode)
{
    sp++;
    stack[sp] = pc;
    uint16_t nnn = opcode & 0x0FFF;
    pc = nnn;
}

// 3xkk Skip next instruction if Vx = kk
void se(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t kk = opcode & 0x00FF;
    if(V[x] == kk) {
        pc += 2;
    }
}

// 4xkk Skip next instruction if Vx != kk
void sne(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t kk = opcode & 0x00FF;
    if(V[x] != kk) {
        pc += 2;
    }
}

// 5xy0 Skip next instruction if Vx = Vy
void sexy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    if (V[x] == V[y]){
        pc += 2;
    }
}

// 6xkk Set Vx = kk
void ldx(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t kk = opcode & 0x00FF;
    V[x] = kk;
}

// 7xkk Adds the value kk to the value of register Vx, then stores the result in Vx
void addx(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint8_t kk = opcode & 0x00FF;
    V[x] += kk;
}

// 8xy0 Stores the value of the register Vy in Vx
void ldvxvy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    V[x] = V[y];
}

// 8xy1 Bitwise OR on Vx and Vy. Stores result in Vx
void orxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    V[x] = V[x] | V[y];
}

// 8xy2 Bitwise AND on Vx and Vy. Stores result in Vx
void andxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    V[x] = V[x] & V[y];
}

// 8xy3 XOR on Vx and Vy then store in Vx
void xorxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    V[x] = V[x] ^ V[y];
}

// 8xy4 Set Vx = Vx + Vy, set VF = carry
void addxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    uint16_t sum = V[x] + V[y];
    if(sum > 255){
        V[0xF] = 1;
    }
    else{
        V[0xF] = 0;
    }
    V[x] = sum & 0xFF;
}

// 8xy5 Set Vx = Vx - Vy, Set VF = NOT borrow
void subxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    if(V[x] > V[y]){
        V[0xF] = 1;
    } 
    else{
        V[0xF] = 0;
    }
    V[x] = V[x] - V[y];
}

// 8xy6 Set Vx = Vx SHR 1
void shrxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    uint8_t sig = V[x] & 0b00000001;
    if(sig == 1){
        V[0xF] = 1;
    }
    else{
        V[0xF] = 0;
    }
    V[x] = V[x] / 2;
}

// 8xy7 Set Vx = Vy - Vx, set VF = NOT borrow
void subnxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    if(V[y] > V[x]){
        V[0xF] = 1;
    }
    else{
        V[0xF] = 0;
    }
    V[x] = V[y] - V[x];
}

// 8xyE Set Vx = Vx SHL 1
void shlx(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint8_t sig = V[x] & 0b00000001;
    if(sig == 1){
        V[0xF] = 1;
    }
    else{
        V[0xF] = 0;
    }
    V[x] = V[x] * 2;
}

// 9xy0 Skip next instruction if Vx != Vy
void snexy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    if(V[x] != V[y]){
        pc += 2;
    }
}

// Annn Set I = nnn
void ldi(uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0FFF;
    in = nnn;
}

// Bnnn Jump to location nnn + V0
void jpv0(uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0FFF;
    pc = nnn + V[0];
}

// Cxkk Set Vx = random byte AND kk
void rndx(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t kk = opcode & 0x00FF;
    srand(time(NULL));
    uint16_t rand_num = rand() % 255;
    V[x] = rand_num & kk;
}

// Dxyn  Display n-byte sprite starting at memory location in and (Vx, Vy), set VF = collision
void drwxy(uint16_t opcode)
{
    uint16_t x = opcode & 0x0F00;
    uint16_t y = opcode & 0x00F0;
    uint16_t n = opcode & 0x000F;
    uint16_t xCoord = V[x];
    uint16_t yCoord = V[y];
    
    for(int i = in; i<in+n; i++){
        uint8_t sprite = addrMem[i];
        uint64_t line = fb[yCoord];
    }
}
