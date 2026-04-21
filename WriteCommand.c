#include <msp430.h>
#include "WriteCommand.h"
#include "PulseEnable.h"

void writeCommand(unsigned char command) {
    // Clears the RS pin to LOW (P2.0).
    P2OUT &= ~BIT0;

    // Send upper nibble
    P1OUT = (P1OUT & 0x0F) | (command & 0xF0);
    pulseEnable();

    // Send lower nibble
    P1OUT = (P1OUT & 0x0F) | ((command << 4) & 0xF0);
    pulseEnable();

    // Standard command execution delay
    if (command == 0x01 || command == 0x02) {
        __delay_cycles(2000); // 2000us (2ms) for Clear Display and Return Home
    } else {
        __delay_cycles(50);   // 50us for other commands
    }
}