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
    if (BIT2 & P2IN) {
        debouceDelay();
        // Button pressed, continue execution
        break;
    }
  }
}

// Global vars (state machine)
volatile unsigned long time_count = 0; // Count in 0.1ms (100us) increments
volatile bool running = false;
int random_seed = 0; // On start button pushed, grab timer value to seed RNG

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
    
    // 1MHz clock
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // Setting up port directions
    // We only need P1.4, P1.5, P1.6, P1.7 as outputs for data
    P1DIR |= (BIT4 | BIT5 | BIT6 | BIT7);
    P2DIR |= (RS | EN); // Set P2.2 and P2.3 as outputs

    // Initialize outputs to LOW for safety
    P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
    P2OUT &= ~(RS | EN);

    // Initialize the LCD in 4-bit mode
    initLCD();

    // Move to col 1, row 2
    writeCommand(0xC0);
    printString("      V1.0      ");

    // Home cursor
    writeCommand(0x02);
    printLongString("MSP430 Human Factors Platform", 500);

    __delay_cycles(50000);

    // Clear display
    writeCommand(0x01);

    printLongString("As soon as an led lights press the button!", 500);

    // Col 1, row 2
    writeCommand(0xC0);
    printLongString("Press button to begin!", 500);

    waitForButton();

    // Clear display
    writeCommand(0x01);

    // Generate random time to wait before lighting LED
    
    // Delay for the random amount of time

    // Light LED to signal the user
    
    // Start counter to measure reaction time
    time_count = 0;
    running = true;

    // Wait for the user to press the button as quickly as possible
    waitForButton();

    // Stop counter
    running = false;

    // Calculate reaction time (reaction_time = time_count * 0.1ms per interrupt)
    
    // Display reaction time on LCD
    writeCommand(0x01); // Clear
    printString("Reaction Time:");
    writeCommand(0xC0); // Second line
    // printString(reaction_time);

    // Turn off LED and prepare for next round
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (running) {
        time_count++;
    }
}
