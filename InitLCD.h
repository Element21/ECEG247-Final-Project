#pragma once // header guard

// Sets the appropriate MSP430 pins you are using to be outputs.
// Sends the required initialization sequence (found in the datasheet) to wake up the LCD.
// The specific initial procedure is different depending on whether you are using 4-bit mode or 8-bit mode.
void initLCD(void);