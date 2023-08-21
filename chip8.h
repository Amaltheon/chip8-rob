#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Testing
void displayFrame();

// Init
void loadSprites();

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