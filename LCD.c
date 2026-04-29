#include "LCD.h"

// I2C Backpack Pin Mappings
#define LCD_BACKLIGHT 0x08 // P3 - Backlight Enable
#define EN 0x04            // P2 - Enable Pin
#define RW 0x02            // P1 - Read/Write Pin
#define RS_MODE 0x01       // P0 - Register Select Pin

void initI2C(void) {
    P1SEL |= BIT6 + BIT7;                 // Assign I2C pins (P1.6 SCL, P1.7 SDA)
    P1SEL2 |= BIT6 + BIT7;
    UCB0CTL1 |= UCSWRST;                  // Enable SW reset to configure I2C
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC; // Master mode, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;        // Use SMCLK, SW reset
    UCB0BR0 = 10;                         // frequency select = SMCLK/10 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = 0x27;                     // I2C LCD backpack address
    UCB0CTL1 &= ~UCSWRST;                 // Clear SW reset, resume operation
}

void writeI2C(char data) {
    while (UCB0CTL1 & UCTXSTP);           // Ensure stop condition got sent (idle)
    UCB0CTL1 |= UCTR + UCTXSTT;           // I2C TX, start condition

    while((IFG2 & UCB0TXIFG) == 0);       // Wait for TX buffer to be ready
    UCB0TXBUF = data;                     // Load data into TX buffer
    
    while((IFG2 & UCB0TXIFG) == 0);       // Wait for data to finish transmitting
    UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
    IFG2 &= ~UCB0TXIFG;                   // Clear TX interrupt flag
}

// Send 4 bits (a nibble) along with the RS mode and backpack backlight state
void writeNibble(char nibble, char mode) {
    // Combine upper 4 bits of data, RS mode, and backlight state
    char data = (char)((nibble & 0xF0) | mode | LCD_BACKLIGHT);
    
    writeI2C(data);           // Send data with Enable LOW
    
    writeI2C((char)(data | EN));      // Set Enable HIGH
    __delay_cycles(10);       // Short delay for Enable pulse

    writeI2C((char)(data & ~EN));     // Set Enable LOW (Clocking-in data)
    __delay_cycles(50);       // Wait for LCD controller to process
}

void writeCommand(char cmd){
    // In 4-bit mode, we send the High Nibble, then the Low Nibble with RS=0 (cmd mode)
    writeNibble((char)(cmd & 0xF0), 0);           // Send upper 4 bits
    writeNibble((char)((cmd << 4) & 0xF0), 0);    // Shift lower 4 bits up, then send

    if (cmd == 0x01 || cmd == 0x02) {
        __delay_cycles(2000); // 2ms delay for clear/home commands
    } else {
        __delay_cycles(60);   // 60us delay for normal commands
    }
}

void writeChar(char data) {
    __disable_interrupt();    // Prevent interrupts from disrupting I2C timing
    
    // send High Nibble, then Low Nibble with RS=1 (Data/Text mode)
    writeNibble((char)(data & 0xF0), RS_MODE); // Send upper 4 bits
    writeNibble((char)((data << 4) & 0xF0), RS_MODE); // Shift lower 4 bits up, then send

    __delay_cycles(50);
    __enable_interrupt();
}

void initLCD(void) {
    __delay_cycles(100000); // Wait for power to stabilize

    // Special Wakeup Sequence for 4-bit mode
    
    writeNibble(0x30, 0);      // Wake up 1
    __delay_cycles(5000);
    writeNibble(0x30, 0);      // Wake up 2
    __delay_cycles(150);
    writeNibble(0x30, 0);      // Wake up 3
    
    writeNibble(0x20, 0);      // Switch to 4-bit mode
    __delay_cycles(150);

    writeCommand(0x28);     // Function set: 4-bit mode, 2 lines, 5x7 font
    writeCommand(0x0C);     // Display Control: Display ON, Cursor OFF
    writeCommand(0x01);     // Clear Display
    writeCommand(0x06);     // Entry Mode: Increment cursor
}

void printString(char* text) {
    unsigned int i = 0;
    while (text[i] != '\0') {
        writeChar(text[i]);
        i++;
    }
}

void printLongString(char inputString[], int delay) {
    /* Print 16 chars at a time, shifting left by one character each iteration */
    unsigned int len = 0;
    while (inputString[len] != '\0') {
        len++;
    }
    
    int numShifts;
    if (len > 16) {
        numShifts = len - 15;
    } else {
        numShifts = 1;
    }
    char chunk[17]; // 16 chars + end char

    unsigned int i = 0;
    for (i = 0; i < numShifts; i++) {
        unsigned int j;
        for (j = 0; j < 16; j++) {
            // Check if current char is in inputString
            if ((i + j) < len) { // i = shift offset; j = char index in chunk
                chunk[j] = inputString[i + j];
            } else {
                chunk[j] = ' '; // fill with whitespace if not
            }
        }
        chunk[16] = '\0';

        // Print 16 char chunk
        printString(chunk);
        
        // Move cursor back 16 spaces to the beginning of the current line
        unsigned int k;
        for (k = 0; k < 16; k++) {
            writeCommand(0x10); // Shift cursor left
        }

        for (j = 0; j < delay; j++) {
            __delay_cycles(1000); // 1ms
        }
    }
}