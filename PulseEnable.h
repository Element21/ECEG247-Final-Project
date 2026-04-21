#pragma once

// Prime the Clock: Set the E pin HIGH.
// Wait: You must leave the E pin HIGH for a short duration. (Look up the minimum "E Pulse Width" in the datasheet). If your MSP430 is using a 1 MHz clock, how many instructions (or cycles) does E need to remain high?
// E Pulse Width = 450 ns. 1 cycle = 1000ns (too lazy to figure out how to speedup clock)
// Clock it in: Pull the E pin LOW. The exact moment the E pin transitions from High to Low (the falling edge) is when the LCD "clocks in" and saves the parallel bits.
void pulseEnable();