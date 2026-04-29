#include "LCD.h"
#include "Ultrasonic.h"
#include "Utils.h"
#include <math.h>
#include <msp430.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Global vars (state machine)
volatile unsigned long timeCount = 0; // Count in 0.1ms (100us) increments
bool running = false;
const signed int SCROLL_DELAY_MS = 250;
const signed int MIN_DELAY_MS = 1000; // ms
const signed int MAX_DELAY_MS = 5000; // ms
const signed int NUM_TRIALS = 10;
volatile unsigned int trialIdx = 0;
unsigned long totalTimeCount = 0; // Accumulate time
unsigned int correctCount = 0;
unsigned int incorrectCount = 0;
signed int gameSelect = -1; // 0 = Go/no-go; 1 = Distance
signed int lastSwitchState = -1;
signed int currentSwitchState; // Set later

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
    
    // 1MHz clock
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;

    // Setup timer A0: SMCLK (1MHz), Up mode
    TA0CCR0 = 100;                // 100us interval (1MHz clock)
    TA0CCTL0 = CCIE;                  // Enable Timer A0 CCR0 interrupt
    TA0CTL = TASSEL_2 | MC_1 | TACLR; // SMCLK, Up mode, clear TA0R

    // Setup timer A1: SMCLK (1MHz), Continuous mode for RNG seeding
    TA1CTL = TASSEL_2 | MC_2 | TACLR; // SMCLK, Continuous mode, clear TA1R

    __enable_interrupt();             // Enable global interrupts

    // Setting up port directions
    // P1.0 correct led, P1.1 incorrect led, P1.2 Buzzer
    P1DIR |= (BIT0 | BIT1 | BIT2);

    P1DIR &= ~(BIT3 | BIT4); // P1.3 button input, P1.4 game select switch input
    P1REN |= BIT3;           // Enable internal resistor for P1.3
    
    // Initialize outputs to LOW for safety
    P1OUT &= ~(BIT0 | BIT1 | BIT2);
    P1OUT |= BIT3;           // Select pull-up for P1.3

    // Initialize the LCD in 4-bit mode
    initI2C();
    initLCD();

    // Initialize the Ultrasonic sensor in UART mode
    initUltrasonic();

    while (true) {
        // Move to col 1, row 2
        writeCommand(0xC0);
        printString("      V1.0      ");

        // Home cursor
        writeCommand(0x02);
        printLongString("MSP430 Human Factors Platform", SCROLL_DELAY_MS - 80);

        __delay_cycles(500000); // 500 ms
        
        // // Game Selection
        while (true) {
            currentSwitchState = (P1IN & BIT4) ? 1 : 0; // 1 if HIGH, 0 if LOW
            
            // Re-draw menu only if switch changes
            if (currentSwitchState != lastSwitchState) {
                lastSwitchState = currentSwitchState;
                
                writeCommand(0x01); // Clear display
                writeCommand(0x02); // Home cursor
                
                if (currentSwitchState) {
                    printString(" Game: Go/No-Go ");
                    gameSelect = 0;
                } else {
                    // Fits in 16 chars so it doesn't block the loop by scrolling
                    printString("Game: Dist Match"); 
                    gameSelect = 1;
                }
                
                writeCommand(0xC0); // Col 1, row 2
                printString("Press to begin! ");
            }
            
            // Check button to start
            if (!(BIT3 & P1IN)) {
                debouceDelay();
                if (!(BIT3 & P1IN)) {
                    break; // button pressed
                }
            }
            
            // Re-seed based on human button press time
            srand(TA1R);
        }
        
        // // Game Code
        // Reset scores for new round
        totalTimeCount = 0;
        correctCount = 0;
        incorrectCount = 0;
        float totalAccuracy = 0.0f;

        for (trialIdx = 0; trialIdx < NUM_TRIALS; trialIdx++) {
            // Clear display, home
            writeCommand(0x01);
            writeCommand(0x02);
            printString("   Get Ready!   ");
            writeCommand(0xC0); // Next line
            
            char trialStr[17];
            formatCount(trialStr, "  Trial: 00/00  ", trialIdx + 1, 9, NUM_TRIALS, 12);
            printString(trialStr);

            // Give user a moment to see the trial text
            __delay_cycles(2000000); // 2 second delay

            if (gameSelect == 0) {
                // Generate random time to wait before lighting LED
                
                // Delay for the random amount of time
                signed int delayTime = (signed int)randomNumBetween((float)MIN_DELAY_MS, (float)MAX_DELAY_MS);

                // Wait for delayTime
                int d;
                for (d = 0; d < delayTime; d++) {
                    __delay_cycles(1000); // 1ms delay per iteration
                }

                // Choose green or yellow
                bool isGreen = (randomNumBetween(0.0f, 1.0f)) < 0.5f;
                
                // Light LED to signal the user
                if (isGreen) {
                    P1OUT |= BIT0; // Green LED
                } else {
                    P1OUT |= BIT1; // Yellow LED
                }

                // Start counter to measure reaction time
                timeCount = 0;
                running = true;

                bool buttonPressed = false;
                while(timeCount < 30000) { // 3 seconds timeout
                    if (!(BIT3 & P1IN)) {
                        debouceDelay();
                        if (!(BIT3 & P1IN)) {
                            buttonPressed = true;
                            break;
                        }
                    }
                }

                // Stop counter 
                running = false;

                writeCommand(0x01); // clear
                writeCommand(0x02); // Home

                if (isGreen) {
                    if (buttonPressed) {
                        correctCount++;
                        totalTimeCount += timeCount;
                        printString(" Reaction Time: ");
                        writeCommand(0xC0); // Second line
                        char timeStr[17];
                        formatTimeString(timeStr, "0.0000s         ", timeCount, 0);
                        printString(timeStr);
                    } else {
                        incorrectCount++;
                        printString("   Too slow!    ");
                    }
                } else {
                    if (buttonPressed) {
                        incorrectCount++;
                        printString("     Wrong!     ");
                        // Play incorrect noise
                        int b;
                        for (b = 0; b < 200; b++) {
                            P1OUT ^= BIT2;
                            __delay_cycles(1000); // Tone
                        }
                        P1OUT &= ~BIT2; // Ensure buzzer off
                    } else {
                        correctCount++;
                        printString("  Good Reject!  ");
                    }
                }
            } else if (gameSelect == 1) {
                // Distance Match Game

                // Generate target between 0.8 and 10 inches
                float targetInches = randomNumBetween(0.8f, 10.0f);

                writeCommand(0x01); // clear
                writeCommand(0x02); // Home
                
                char distStr[17];
                formatDistance(distStr, "Target: 00.0 in ", targetInches, 8);
                printString(distStr);

                writeCommand(0xC0); // Next line
                printString("Press when ready");

                // Wait for button press
                while(true) { 
                    if (!(BIT3 & P1IN)) {
                        debouceDelay();
                        if (!(BIT3 & P1IN)) {
                            break;
                        }
                    }
                }

                // Get actual distance
                float actualInches = getDistance();

                writeCommand(0x01); // clear
                writeCommand(0x02); // Home
                
                char actualStr[17];
                formatDistance(actualStr, "Actual: 00.0 in ", actualInches, 8);
                printString(actualStr);

                writeCommand(0xC0);
                
                // Calculate accuracy %
                float error = fabsf(targetInches - actualInches);
                float accuracy = 100.0f * (1.0f - (error / targetInches));
                if (accuracy < 0) accuracy = 0;
                
                totalAccuracy += accuracy;

                char accStr[17];
                formatAccuracy(accStr, " Acc: 000.0%    ", accuracy, 6);
                printString(accStr);
            }

            // Turn off LED and prepare for next round
            P1OUT &= ~(BIT0 | BIT1);

            // Give the user 2 seconds to read their reaction time
            __delay_cycles(2000000); 

            // Clear display, home
            writeCommand(0x01);
            writeCommand(0x02);
        }

        // Display the calculated average reaction time
        writeCommand(0x01); // Clear
        writeCommand(0x02); // Home
        
        if (gameSelect == 0) {
            // Show correct / incorrect
            char scoreStr[26];
            formatCount(scoreStr, "Correct: 00 Incorrect: 00", correctCount, 9, incorrectCount, 23);

            writeCommand(0xC0); // Second line
            
            char avgStr[17];
            unsigned long avgCount = 0;
            if (correctCount > 0) avgCount = totalTimeCount / correctCount;
            
            formatTimeString(avgStr, "Avg: 0.0000s    ", avgCount, 5);
            
            printString(avgStr);

            writeCommand(0x02); // Home for scrolling text
            printLongString(scoreStr, SCROLL_DELAY_MS);
        } else if (gameSelect == 1) {
            float avgAcc = totalAccuracy / NUM_TRIALS;
            char finalAccStr[17];
            formatAccuracy(finalAccStr, " Avg Acc: 000.0%", avgAcc, 10);
            
            printString("Distance Match  ");
            writeCommand(0xC0);
            printString(finalAccStr);
        }

        // Wait for button press to restart the whole sequence
        while (true) {
            if (!(BIT3 & P1IN)) {
                debouceDelay();
                if (!(BIT3 & P1IN)) {
                    break;
                }
            }
        }
        writeCommand(0x01); // Clear display before restarting
    }
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    if (running) {
        timeCount++;
    }
}