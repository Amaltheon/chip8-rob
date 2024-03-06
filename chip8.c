#include "chip8.h"

uint8_t V[16];
uint16_t in;
uint16_t stack[64];
uint8_t sp;
uint8_t dt;
uint8_t st;
bool fb[64][32];
uint16_t pc = 512;

uint8_t addrMem[4096];

uint8_t sprites[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void loadSprites(){
    for(int i = 0; i<80; i++) {
        addrMem[i] = sprites[i];
    }
}



// 00E0 Clears Display, doubles as init for screen
void cls()
{
    for(int i=0; i<64; i++) {
        for(int j=0; j<32; j++) {
            fb[i][j] = false;
        }
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t kk = opcode & 0x00FF;
    if(V[x] == kk) {
        pc += 2;
    }
}

// 4xkk Skip next instruction if Vx != kk
void sne(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t kk = opcode & 0x00FF;
    if(V[x] != kk) {
        pc += 2;
    }
}

// 5xy0 Skip next instruction if Vx = Vy
void sexy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    if (V[x] == V[y]){
        pc += 2;
    }
}

// 6xkk Set Vx = kk
void ldx(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t kk = opcode & 0x00FF;
    printf("V%i", x);
    printf("=%i\n", kk);
    V[x] = kk;
}

// 7xkk Adds the value kk to the value of register Vx, then stores the result in Vx
void addx(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint8_t kk = opcode & 0x00FF;
    V[x] += kk;
    printf("V%i = ", x);
    printf("%i\n", V[x]);
}

// 8xy0 Stores the value of the register Vy in Vx
void ldvxvy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    uint8_t vy = V[y];
    V[x] = vy;
}

// 8xy1 Bitwise OR on Vx and Vy. Stores result in Vx
void orxy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    V[x] = V[x] | V[y];
}

// 8xy2 Bitwise AND on Vx and Vy. Stores result in Vx
void andxy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    V[x] = V[x] & V[y];
}

// 8xy3 XOR on Vx and Vy then store in Vx
void xorxy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    V[x] = V[x] ^ V[y];
}

// 8xy4 Set Vx = Vx + Vy, set VF = carry
void addxy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
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
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t kk = opcode & 0x00FF;
    srand(time(NULL));
    uint16_t rand_num = rand() % 255;
    V[x] = rand_num & kk;
}

// Dxyn  Display n-byte sprite starting at memory location in and (Vx, Vy), set VF = collision
void drwxy(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint16_t y = (opcode & 0x00F0) >> 4;
    uint16_t n = opcode & 0x000F;
    uint16_t xCoord = V[x] + 6;
    uint16_t yCoord = V[y];
    printf("X = %i\n", xCoord);
    printf("Y = %i\n", yCoord);
    printf("n = %i\n", n);

    V[0xF] = 0;
    
    for(int i = in; i<(in+n); i++){
        uint8_t sprite = addrMem[i];
        for(int j=0; j<8; j++) {
            if(sprite & 1) {
                if(fb[xCoord - j][yCoord] == true) {
                    V[0xF] = 1;
                    fb[xCoord - j][yCoord] = false;
                    continue;
                }
                fb[xCoord - j][yCoord] = true;
            }
            else {
                if(fb[xCoord - j][yCoord] == true) {
                    fb[xCoord - j][yCoord] = true;
                    continue;
                }
                fb[xCoord - j][yCoord] = false;
            }
            sprite >>= 1;
        }
        yCoord++;
    }
}

// Ex9E - SKP Vx Skip next instruction if key with the value of Vx is pressed.
void skpvx(uint16_t opcode, uint16_t key)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    if(x == key){
        pc+=2;
    }
}

// ExA1 - SKNP Vx Skip next instruction if key with the value of Vx is not pressed.
void sknpvx(uint16_t opcode, uint16_t key)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    if(x != key){
        pc+=2;
    }
}

// Fx07 - LD Vx, DT Set Vx = delay timer value.
void ldvx(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    V[x] = dt;
}

// Fx0A - LD Vx, K Wait for a key press, store the value of the key in Vx.
void ldvx_wait(uint16_t opcode, uint8_t key)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    V[x] = key;
}

// Fx15 - LD DT, Vx Set delay timer = Vx.
void lddt(uint16_t opcode) 
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    dt = V[x];
}

// Fx18 - LD ST, Vx Set sound timer = Vx.
void ldst(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    st = V[x];
}

// Fx1E - ADD I, Vx Set I = I + Vx
void addi(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    in = (in + V[x]);
}

// Fx29 - LD F, Vx Set I = location of sprite for digit Vx.
void ldf(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint8_t digit = V[x];

    in = digit * 5;
}

// Fx33 - LD B, Vx Store BCD representation of Vx in memory locations I, I+1, and I+2.
void ldb(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    uint8_t num = V[x];
    for(int i=0; i<3; i++){
        uint8_t digit = num % 10;
        addrMem[in + (2 - i)] = digit;
        num = num / 10;
    } 
}

// Fx55 - LD [I], Vx Store registers V0 through Vx in memory starting at location I.
void ldivx(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    for(int i = 0; i<=x; i++) {
        addrMem[in + i] = V[i];
    }
}

// Fx65 - LD Vx, [I] Read registers V0 through Vx from memory starting at location I
void ldvx65(uint16_t opcode)
{
    uint16_t x = (opcode & 0x0F00) >> 8;
    for(int i = 0; i<=x ; i++) {
        V[i] = addrMem[in + i];
    }
}