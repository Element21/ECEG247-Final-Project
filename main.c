#include <msp430.h>
#include <stdbool.h>
#include <stdlib.h>
#include "InitLCD.h"
#include "PrintString.h"

void scrollString(char[] inputString) {

}

void showSplashScreen() {
    initLCD();

    printString("Reaction Time Test");
}

void debouceDelay() {
    __delay_cycles(50000); // 50ms debounce
}

void waitForButton() {
  while (true) {
    if (BIT2 & P2IN) {
        debouceDelay();
        // Button pressed, continue execution
        break
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

    P1DIR |= 0x01;                            // Set P1.0 to output direction
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (running) {
        time_count++;
    }