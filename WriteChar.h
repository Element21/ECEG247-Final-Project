#pragma once // Header guard

// Sets the RS pin HIGH.
// Places the bits corresponding to the character onto the DB pins.
// Calls your pulseEnable() function.
// The character table of the datasheet shows the encoding. Look at ASCII encoded data to see how characters are encoded as hex/binary values.
void writeChar(char data);