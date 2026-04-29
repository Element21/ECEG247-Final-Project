#pragma once

#include <msp430.h>

void initI2C(void);
void writeI2C(char data);
void writeNibble(char nibble, char mode);
void writeCommand(char cmd);
void writeChar(char data);
void initLCD(void);
void printString(char* text);
void printLongString(char inputString[], int delay);
