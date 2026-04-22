#include "LCD.h"

void pulseEnable(void) {
    P2OUT |= EN;            // Set Enable HIGH
    __delay_cycles(10);     // Delay 
    P2OUT &= ~EN;           // Set Enable LOW (Clocking-in)
    __delay_cycles(10);
}

// New Function: Sends just 4 bits (a nibble) to P1.4-P1.7
void writeNibble(char nibble) {
    // Clear the upper 4 bits of P1OUT without affecting the lower 4 bits
    P1OUT &= 0x0F; 
    // Put the nibble on the upper 4 bits of P1OUT
    P1OUT |= (nibble & 0xF0); 
    
    pulseEnable();
}

void writeCommand(char cmd){
    P2OUT &= ~RS;           // RS LOW for command mode
    
    // In 4-bit mode, we send the High Nibble, then the Low Nibble
    writeNibble(cmd);       // Send upper 4 bits
    writeNibble(cmd << 4);  // Shift lower 4 bits up, then send

    if (cmd == 0x01 || cmd == 0x02) {
        __delay_cycles(2000); // 2ms delay for clear/home commands
    } else {
        __delay_cycles(50);   // 50us delay for normal commands
    }
}

void writeChar(char data) {
    P2OUT |= RS;            // RS HIGH for Data/Text Mode
    
    // Send High Nibble, then Low Nibble
    writeNibble(data);      
    writeNibble(data << 4); 

    __delay_cycles(50);
}

void initLCD(void) {
    __delay_cycles(100000); // Wait for power to stabilize

    // Special Wakeup Sequence for 4-bit mode
    P2OUT &= ~RS;           // Command mode
    
    writeNibble(0x30);      // Wake up 1
    __delay_cycles(5000);
    writeNibble(0x30);      // Wake up 2
    __delay_cycles(150);
    writeNibble(0x30);      // Wake up 3
    
    writeNibble(0x20);      // **Switch to 4-bit mode**
    __delay_cycles(150);

    writeCommand(0x28);     // Function set: 4-bit mode, 2 lines, 5x7 font
    writeCommand(0x0C);     // Display Control: Display ON, Cursor OFF
    writeCommand(0x01);     // Clear Display
    writeCommand(0x06);     // Entry Mode: Increment cursor
}

void printString(char* text) {
    int i = 0;
    while (text[i] != '\0') {
        writeChar(text[i]);
        i++;
    }
}

void printLongString(char inputString[], int delay) {
    /* Split inputString into chunks of length 16 and write them to the lcd with a specified delay in ms */
    int len = 0;
    while (inputString[len] != '\0') {
        len++;
    }
    
    int num_outputs = (len + 15) / 16;
    char chunk[17]; // 16 chars + end char

    unsigned int i = 0;
    for (i = 0; i < num_outputs; i++) {
        unsigned int j;
        for (j = 0; j < 16; j++) {
            // Check if current char is in inputString
            if ((i * 16 + j) < len) { // i = chunk #; j = offset
                chunk[j] = inputString[i * 16 + j];
            } else {
                chunk[j] = ' '; // fill with whitespace if not
            }
        }
        chunk[16] = '\0';

        // Print 16 char chunk
        printString(chunk);
        
        for (j = 0; j < delay; j++) {
            __delay_cycles(1000); // 1ms
        }
    }
}