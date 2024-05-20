#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Testing
void displayFrame();

// Init
void loadSprites();

extern uint8_t V[16];
extern uint16_t in;
extern uint16_t stack[64];
extern uint8_t sp;
extern uint8_t dt;
extern uint8_t st;
extern bool fb[64][32];
extern uint16_t pc;
extern uint8_t keys[16];
extern int timer_counter;

extern uint8_t addrMem[4096];

struct pixel {
    int x;
    int y;
    bool on;
};

// Op Codes
void cls();
void ret();
void jp(uint16_t opcode);
void call(uint16_t opcode);
void se(uint16_t opcode);
void sne(uint16_t opcode);
void sexy(uint16_t opcode);
void ldx(uint16_t opcode);
void addx(uint16_t opcode);
void ldvxvy(uint16_t opcode);
void orxy(uint16_t opcode);
void andxy(uint16_t opcode);
void xorxy(uint16_t opcode);
void addxy(uint16_t opcode);
void subxy(uint16_t opcode);
void shrxy(uint16_t opcode);
void subnxy(uint16_t opcode);
void shlx(uint16_t opcode);
void snexy(uint16_t opcode);
void ldi(uint16_t opcode);
void jpv0(uint16_t opcode);
void rndx(uint16_t opcode);
void drwxy(uint16_t opcode);
void skpvx(uint16_t opcode, uint8_t keys[]);
void sknpvx(uint16_t opcode, uint8_t keys[]);
void ldvx(uint16_t opcode);
void ldvx_wait(uint16_t opcode, uint8_t keys[]);
void lddt(uint16_t opcode);
void ldst(uint16_t opcode);
void addi(uint16_t opcode);
void ldf(uint16_t opcode);
void ldb(uint16_t opcode);
void ldivx(uint16_t opcode);
void ldvx65(uint16_t opcode);