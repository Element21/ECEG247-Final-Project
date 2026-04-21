#include <msp430.h>
#include "PrintString.h"
#include "WriteChar.h"

void printString(char* text, unsigned int delay_ms) {
    while (*text != '\0') {
        unsigned int i;
        for (i = 0; i < delay_ms; i++) {
            __delay_cycles(1000); // 1ms
        }
        writeChar(*text);
        text++;
    }
}