#pragma once // Header guard

// Clears the RS pin to LOW.
// Places the bits of the command onto the DB pins.
// Calls your pulseEnable() function to clock in the data
// For 4-bit mode you will need to clock in each nibble of the command.
void writeCommand(unsigned char command);
