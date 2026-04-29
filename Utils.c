#include "Utils.h"
#include <msp430.h>
#include <stdlib.h>
#include <string.h>

void debouceDelay() {
    __delay_cycles(50000); // 50ms debounce
}

// Format place values for a time string
void formatTimeString(char* outStr, const char* templateStr, unsigned long tCount, unsigned int startPos) {
    strcpy(outStr, templateStr);
    unsigned int whole = tCount / 10000;
    unsigned int frac = tCount % 10000;
    outStr[startPos] = '0' + whole; // Assuming < 10s
    outStr[startPos+2] = '0' + (frac / 1000);
    outStr[startPos+3] = '0' + ((frac / 100) % 10);
    outStr[startPos+4] = '0' + ((frac / 10) % 10);
    outStr[startPos+5] = '0' + (frac % 10);
}

// Format place values for two, two digit counting string "00/00"
void formatCount(char* outStr, const char* templateStr, unsigned int v1, unsigned int p1, unsigned int v2, unsigned int p2) {
    strcpy(outStr, templateStr);
    outStr[p1] = '0' + (v1 / 10);
    outStr[p1+1] = '0' + (v1 % 10);
    outStr[p2] = '0' + (v2 / 10);
    outStr[p2+1] = '0' + (v2 % 10);
}

// Format 3 digit distance string "00.0 in"
void formatDistance(char* outStr, const char* templateStr, float value, unsigned int startPos) {
    strcpy(outStr, templateStr);
    unsigned int whole = (unsigned int)value;
    unsigned int frac = (unsigned int)(value * 10) % 10;
    
    // Set the tens digit, or a space if the value is less than 10
    if (whole >= 10) {
        outStr[startPos] = '0' + (whole / 10);
    } else {
        outStr[startPos] = ' ';
    }
    
    // Set the ones digit
    outStr[startPos+1] = '0' + (whole % 10);
    
    // Set the tenths digit
    outStr[startPos+3] = '0' + frac;
}

// Format 4 digit output accuracy string "Acc: 000.0%"
void formatAccuracy(char* outStr, const char* templateStr, float accuracy, unsigned int startPos) {
    strcpy(outStr, templateStr);
    unsigned int w = (unsigned int)accuracy;
    unsigned int f = (unsigned int)(accuracy * 10) % 10;
    
    // Set the hundreds digit, or a space if less than 100
    if (w >= 100) {
        outStr[startPos] = '1';
    } else {
        outStr[startPos] = ' ';
    }
    
    // Set the tens digit, or a space if less than 10
    if (w >= 10) {
        outStr[startPos+1] = '0' + ((w / 10) % 10);
    } else {
        outStr[startPos+1] = ' ';
    }
    
    // Set the ones digit
    outStr[startPos+2] = '0' + (w % 10);
    
    // Set the tenths digit
    outStr[startPos+4] = '0' + f;
}

float randomNumBetween(float min, float max) {
    float r = (float)rand() / (float)RAND_MAX;
    return min + r * (max - min);
}