#include <msp430.h>
#include "PulseEnable.h"

void pulseEnable() {
    P2DIR |= BIT1; // Set E OUT (BIS)
    P2OUT |= BIT1; // Set E HIGH

    // 1 cycle = 1000ns (too lazy to figure out how to speedup clock)
                        // unsigned short int i = 1; // Ti compiler gets angry if I don't do this
    __delay_cycles(1);  // for (unsigned short int i = 1; i > 0; i--) {}

    // Set E LOW (BIC) ALWAYS INVERT MASK
    P2OUT &= ~BIT1;
}