// Speaker.h
// Sept. 14, 2016

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Boolean.h"

// Turns speaker on and off
// Inputs: enable signal: 1 = on, 0 = off
// Timer0A Interrupt needs to be initialized.

void SpeakerEnable(bool enable) {
	if(enable)
		PWM0_ENABLE_R |= 0x01;
	else
		PWM0_ENABLE_R &= ~0x01;
}
