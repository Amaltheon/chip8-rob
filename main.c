#include <stdio.h>
#include "chip8.h"

int main() {
    cls();
    jp(0x1AF0);
    return 0;
}