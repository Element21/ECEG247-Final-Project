#include <msp430.h>
#include "InitLCD.h"
#include "WriteCommand.h"
#include "PulseEnable.h"

void initLCD(void) {
    // Set Data pins (P1.4 - P1.7) to output
    P1DIR |= 0xF0;
    P1OUT &= ~0xF0;
    
    // Set RS (P2.0) and E (P2.1) to output
    P2DIR |= BIT0 | BIT1;
    P2OUT &= ~(BIT0 | BIT1);
    
    // Wait for LCD power up (>40ms) Default DCO ~1MHz -> 1 cycle = ~1us
    __delay_cycles(50000);
    
    // 4 bit init sequence
    P2OUT &= ~BIT0; // RS = 0
    
    P1OUT = (P1OUT & 0x0F) | 0x30;
    pulseEnable();
    __delay_cycles(5000);   // Wait >4.1ms
    
    P1OUT = (P1OUT & 0x0F) | 0x30;
    pulseEnable();
    __delay_cycles(150);    // Wait >100us
    
    P1OUT = (P1OUT & 0x0F) | 0x30;
    pulseEnable();
    __delay_cycles(50);     // Wait >37us
    
    P1OUT = (P1OUT & 0x0F) | 0x20; // 4-bit mode
    pulseEnable();
    __delay_cycles(50);     // Wait >37us
    
    // Function set: 4-bit mode, 2 lines, 5x8 font
    writeCommand(0x28);
    
    // Display off
    writeCommand(0x08);
    
    // Display clear
    writeCommand(0x01);
    
    // Entry mode set: Increment, no shift
    writeCommand(0x06);
    
    // Display on, Cursor blinking 
    writeCommand(0x0F);
}