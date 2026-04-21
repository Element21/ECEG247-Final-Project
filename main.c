#include <msp430.h>
#include "InitLCD.h"

void waitForButton() {
  while (true) {
    if (BIT2 & P2IN) {
      // Button pressed, continue execution
      break
    }
  }
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

    P1DIR |= 0x01;                            // Set P1.0 to output direction

    initLCD();
}
