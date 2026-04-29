#include "Ultrasonic.h"

// Trig and Echo
#define TRIG_PIN BIT1 // P2.1
#define ECHO_PIN BIT0 // P2.0

void initUltrasonic(void) {
    P2SEL &= ~(TRIG_PIN | ECHO_PIN);
    P2SEL2 &= ~(TRIG_PIN | ECHO_PIN);
    P2DIR |= TRIG_PIN;    // Trig is an output
    P2DIR &= ~ECHO_PIN;   // Echo is an input
    P2OUT &= ~TRIG_PIN;   // Ensure Trig starts LOW
}

float getDistance(void) {
    float distanceSum = 0.0f;
    int validCount = 0;

    int i = 0;
    for (i = 0; i < 3; i++) {
        // Disable interrupts to prevent timer overhead from skewing measurements
        unsigned short old_int = __get_SR_register() & GIE;
        __disable_interrupt();

        // Send a pulse on the Trig pin to start the measurement
        P2OUT &= ~TRIG_PIN; // Give a short LOW pulse beforehand
        __delay_cycles(2);
        P2OUT |= TRIG_PIN;
        __delay_cycles(10); // 10us delay
        P2OUT &= ~TRIG_PIN;

        // Wait for the Echo pin to go HIGH
        unsigned int waitTimeout = 50000U;
        while(((P2IN & ECHO_PIN) == 0) && (waitTimeout > 0)) {
            waitTimeout--;
        }

        if (waitTimeout == 0) {
            if (old_int) __enable_interrupt();
            continue; // Timeout: move to next measurement attempt
        }

        // Measure how long the Echo pin stays HIGH
        unsigned int pulseLength = 0;
        while(((P2IN & ECHO_PIN) > 0) && (pulseLength < 30000)) {
            __delay_cycles(10); // Measure in 10us increments
            pulseLength++;
        }

        if (old_int) __enable_interrupt();

        // Convert time to distance
        // Speed of sound is 0.0135039 in/us
        // 10us units: Distance = (pulseLength * 10 * 0.0135039) / 2    
        distanceSum += (float)((pulseLength * 10.0f * 0.0135039f) / 2.0f);
        validCount++;

        // Add a short delay between measurements to avoid ultrasonic echoes
        __delay_cycles(10000); // 10ms
    }

    if (validCount == 0) {
        return 0.0f; // Sensor didn't respond on any attempt
    }

    // Return the average of valid measurements
    return distanceSum / validCount;
}