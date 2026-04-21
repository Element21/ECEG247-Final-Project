#include <msp430.h>
#include "WriteChar.h"
#include "PulseEnable.h"

void writeChar(char data) {
    _BIC_SR(GIE); // Disable interrupts

    // Sets the RS pin HIGH (P2.0).
    P2OUT |= BIT0;

    // Send upper nibble
    P1OUT = (P1OUT & 0x0F) | (data & 0xF0);
    pulseEnable();

    // Send lower nibble
    P1OUT = (P1OUT & 0x0F) | ((data & 0x0F) << 4);
    pulseEnable();

    // Standard character execution delay
    __delay_cycles(50); // 50us (for ~37 us standard command duration)

    _BIS_SR(GIE);                   // Activate all interrupts
}