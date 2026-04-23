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
    if (!(BIT3 & P1IN)) {
        debouceDelay();
        // Button pressed, continue execution
        break;
    }
  }
}

// Global vars (state machine)
volatile unsigned long time_count = 0; // Count in 0.1ms (100us) increments
volatile bool running = false;
volatile unsigned int random_seed = 0; // On start button pushed, grab timer value to seed RNG
const signed int SCROLL_DELAY_MS = 500;
const signed int MIN_DELAY_MS = 102; // ms
const signed int MAX_DELAY_MS = 500; // ms
const signed int NUM_TRIALS = 10;

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

    while (true) {
        // Move to col 1, row 2
        writeCommand(0xC0);
        printString("      V1.0      ");

        // Home cursor
        writeCommand(0x02);
        printLongString("MSP430 Human Factors Platform", 500);

        __delay_cycles(500000); // 500 ms

        // Clear display
        writeCommand(0x01);

        while (BIT3 & P1IN) {
            // home cursor, clear
            writeCommand(0x01);
            writeCommand(0x02);

            printLongString("When the LED lights press button!", 500);

            // Col 1, row 2
            writeCommand(0xC0);
            printLongString("Press button to begin!", 500);
        }
        
        unsigned int trialIdx = 0;
        unsigned long total_time_count = 0; // Accumulate time
        for (trialIdx = 0; trialIdx < NUM_TRIALS; trialIdx++) {
            // Clear display, home
            writeCommand(0x01);
            writeCommand(0x02);
            printString("   Get Ready!   ");
            writeCommand(0xC0); // Next line
            char trialStr[17];
            snprintf(trialStr, 17, "Trial: %d/%d", trialIdx + 1, NUM_TRIALS);
            printString(trialStr);

            // Generate random time to wait before lighting LED
            random_seed = TA0R;
            
            // Delay for the random amount of time
            // https://www.tutorialspoint.com/cprogramming/c_random_number_generation.htm - example #3
            srand(random_seed);
            signed int delay_time = (rand() % (MAX_DELAY_MS - MIN_DELAY_MS + 1)) + MIN_DELAY_MS;

            // Wait for delay_time
            int d;
            for (d = 0; d < delay_time; d++) {
                __delay_cycles(1000); // 1ms delay per iteration
            }

            // Light LED to signal the user
            P2OUT ^= BIT4;

            // Start counter to measure reaction time
            time_count = 0;
            running = true;

            // Wait for the user to press the button as quickly as possible
            waitForButton();

            // Stop counter and add to total 
            running = false;
            total_time_count += time_count;

            // Calculate reaction time (reaction_time = time_count * 0.1ms per interrupt)
            // 1 sec = 10,000 counts
            writeCommand(0x02); // Home
            printString(" Reaction Time: ");
            writeCommand(0xC0); // Second line
            char timeStr[17];
            snprintf(timeStr, 17, "%.4fs", time_count / 10000.0);
            printString(timeStr);

            // Turn off LED and prepare for next round
            P2OUT ^= BIT4;

            // Give the user 2 seconds to read their reaction time
            __delay_cycles(2000000); 

            // Clear display, home
            writeCommand(0x01);
            writeCommand(0x02);
        }

        // Display the calculated average reaction time
        writeCommand(0x01); // Clear
        writeCommand(0x02); // Home
        printString("  Avg Reaction  ");
        writeCommand(0xC0); // Second line
        char avgStr[17];
        snprintf(avgStr, 17, "%.4fs", (total_time_count / (float)NUM_TRIALS) / 10000.0); // 10000 ticks per sec
        printString(avgStr);

        // Wait 5 seconds to restart the whole sequence
        __delay_cycles(5000000);
        writeCommand(0x01); // Clear display before restarting
    }
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (running) {
        time_count++;
    }
}
