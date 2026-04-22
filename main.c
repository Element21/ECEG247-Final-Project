#include "LCD.h"
#include <math.h>
#include <msp430.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void debouceDelay() {
    __delay_cycles(50000); // 50ms debounce
}

void waitForButton() {
  while (true) {
    if (BIT3 & P1IN) {
        debouceDelay();
        // Button pressed, continue execution
        break;
    }
  }
}

// Global vars (state machine)
volatile unsigned long time_count = 0; // Count in 0.1ms (100us) increments
volatile bool running = false;
signed int random_seed = 0; // On start button pushed, grab timer value to seed RNG
const signed int MIN_DELAY = 102; // ms
const signed int MAX_DELAY = 1000; // ms

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
    
    // 1MHz clock
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // Setting up port directions
    // We only need P1.4, P1.5, P1.6, P1.7 as outputs for data
    P1DIR |= (BIT4 | BIT5 | BIT6 | BIT7);
    P2DIR |= (RS | EN | BIT4); // Set P2.2, P2.3, P2.4 as outputs

    P1DIR &= ~BIT3; // P1.3 input
    
    // Initialize outputs to LOW for safety
    P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
    P2OUT &= ~(RS | EN | BIT4);

    // Initialize the LCD in 4-bit mode
    initLCD();

    // Move to col 1, row 2
    writeCommand(0xC0);
    printString("      V1.0      ");

    // Home cursor
    writeCommand(0x02);
    printLongString("MSP430 Human Factors Platform", 1000);

    __delay_cycles(500000); // 500 ms

    // Clear display
    writeCommand(0x01);

    while (!(BIT2 & P2IN)) {
        // home cursor, clear
        writeCommand(0x01);
        writeCommand(0x02);

        printLongString("As soon as an led lights press the button!", 1000);
        
        // Col 1, row 2
        writeCommand(0xC0);
        printLongString("Press button to begin!", 1000);
    }
    
    // Clear display, home
    writeCommand(0x01);
    writeCommand(0x02);
    printString("   Get Ready!   ");
    
    // Generate random time to wait before lighting LED
    random_seed = TA0R;
    srand(random_seed);
    
    // Delay for the random amount of time
    // https://www.tutorialspoint.com/cprogramming/c_random_number_generation.htm - example #3
    signed int delay_time = (rand() % (MAX_DELAY - MIN_DELAY + 1)) + MIN_DELAY;

    // Light LED to signal the user
    P2OUT ^= BIT4;

    // Start counter to measure reaction time
    time_count = 0;
    running = true;

    // Wait for the user to press the button as quickly as possible
    waitForButton();

    // Stop counter
    running = false;

    // Calculate reaction time (reaction_time = time_count * 0.1ms per interrupt)
    writeCommand(0x01); // Clear
    printString("Reaction Time: ");
    writeCommand(0xC0); // Second line
    printString(snprintf("%f", time_count * 0.1));

    // Turn off LED and prepare for next round
    P2OUT ^= BIT4;
    // Clear display, home
    writeCommand(0x01);
    writeCommand(0x02);
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (running) {
        time_count++;
    }
}
