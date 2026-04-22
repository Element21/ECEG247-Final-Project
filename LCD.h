#pragma once

#include <msp430.h>

// PIN DEFINITIONS
#define RS BIT2                        // P2.2 is Register Select
#define EN BIT3                        // P2.3 is Enable

// Function Prototypes
void pulseEnable(void);
void writeNibble(char nibble);
void writeCommand(char cmd);
void writeChar(char data);
void initLCD(void);
void printString(char* text);
void printLongString(char inputString[], int delay);
