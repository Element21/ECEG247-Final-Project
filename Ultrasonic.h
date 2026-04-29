#pragma once

#include <msp430.h>

// Initializes the pins for the ultrasonic sensor in UART mode
void initUltrasonic(void);

// Triggers the sensor and measures the echo pulse
// Returns the distance in centimeters
float getDistance(void);