#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Switch.h"
#include "Time.h"
#define DELAY10MS 160000
bool debounce[4];

void Buttons_Init(void) {
	SYSCTL_RCGCGPIO_R |= 0x10;        // 1) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10)==0); // allow time for clock to start
                                    // 2) no need to unlock PE0-3
  GPIO_PORTE_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTE_AMSEL_R &= ~0x0F;      // 4) disable analog function on PE0-3
                                    // 5) no pullup for external switches
  GPIO_PORTE_DIR_R &= ~0x0F;        // 5) set direction to output
  GPIO_PORTE_AFSEL_R &= ~0x0F;      // 6) regular port function
  GPIO_PORTE_DEN_R |= 0x0F;         // 7) enable digital port
	
	Timer1_Init();
}

void Buttons_Input(void) {
	int32_t data = GPIO_PORTE_DATA_R;
	
	if((data & 0x01) != 0)
		debounce[0] = (data & 0x01)*DELAY10MS;
	if((data & 0x02) != 0)
		debounce[1] = ((data & 0x2) >> 1)*DELAY10MS;
	if((data & 0x04) != 0)
		debounce[2] = ((data & 0x4) >> 2)*DELAY10MS;
	if((data & 0x08) != 0)
		debounce[3] = ((data & 0x8) >> 3)*DELAY10MS;
}

int Buttons_Pressed(uint32_t button) {
	if(button == 0){ return 0;}
	if(button == 1){ return 1;}
	if(button == 2){ return 2;}
	if(button == 3){ return 3;}
	else return -1;
}

void Buttons_Handler(void) {
	Buttons_Input();
	for(uint32_t i = 0; i < 4; i += 1) {
		if(debounce[i] > 0) {
			debounce[i] -= 1;
			if(debounce[i] == 0) {
				Buttons_Pressed(i);
			}
		}
	}
}