// main.c for Lab3
// Runs on TM4C123
// Kinan Hernandez, Alice Lam
// September 18, 2016
// Modified


// hardware connections
// **********ST7735 TFT and SDC*******************
// ST7735
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// **********wide.hk ST7735R with ADXL345 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// SDA  – (NC) I2C data for ADXL345 accelerometer
// SCL  – (NC) I2C clock for ADXL345 accelerometer
// SDO  – (NC) I2C alternate address for ADXL345 accelerometer
// Backlight + - Light, backlight connected to +3.3 V

// **********wide.hk ST7735R with ADXL335 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// X– (NC) analog input X-axis from ADXL335 accelerometer
// Y– (NC) analog input Y-axis from ADXL335 accelerometer
// Z– (NC) analog input Z-axis from ADXL335 accelerometer
// Backlight + - Light, backlight connected to +3.3 V

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "ST7735.h"
#include "PLL.h"
#include "inc/tm4c123gh6pm.h"
#include "fixed.h"
#include <math.h>
#include "Graphics.h"
#include "Time.h"
#include "Switch.h"
#include "Speaker.h"


void DelayWait10ms(uint32_t n);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void PortF_Init(void);

#define PF1   (*((volatile uint32_t *)0x40025008))
#define PF2   (*((volatile uint32_t *)0x40025010))
#define PF3   (*((volatile uint32_t *)0x40025020))
#define PF4   (*((volatile uint32_t *)0x40025040))
	
//GLOBALS
int32_t lastsec;
int32_t lastmin;
int32_t pm = 0;
// Subroutine to wait 10 msec
// Inputs: None
// Outputs: None
// Notes: ...
void DelayWait10ms(uint32_t n){uint32_t volatile time;
  while(n){
    time = 727240*2/91;  // 10msec
    while(time){
      time--;
    }
    n--;
  }
}

void Pause(void){
  while(PF4==0x00){ 
    DelayWait10ms(10);
  }
  while(PF4==0x10){
    DelayWait10ms(10);
  }
}

int32_t getSec(int32_t t){int32_t s;
	s = t%60;
	return s;
}
int32_t getMin(int32_t t){int32_t m;
	m = floor(t%(60*60)/60);;
	return m;
}
int32_t getHr(int32_t t){int32_t h;
	// Calculate HOUR, (different for night vs. day)
	if (t<43200){							// CHECK: AM/PM, seconds in 24hrs = 86400. 86400/2 = 43200
		h = ((t*60/(60*60))/12);// calculate hours, range 0 < hr < 60. Divide by 5 to get 0 < hr < 12
		if (pm == 0){
			pm = 1;
			day();
		}
	}else{
		h = (((t-43200)*60/(60*60))/12);
		if (pm == 1){
			pm=0;
			night();
		}
	}
	return h;
}

void timeSet(int32_t hr, int32_t min,int32_t sec){
		
}
void UpdateAnalog(int32_t hr, int32_t min, int32_t sec){
	if (sec != lastsec){
				//draw second
				lastsec = sec; 
				//DrawBackground();
				DrawHour(hr);
				DrawMinute(min);
				DrawSecond(sec);
				if (min != lastmin){ 
						lastmin = min;
						//DrawBackground();
						DrawHour(hr);
						DrawMinute(min);
						DrawSecond(sec);
				}
	}
}
void UpdateDigital(int32_t hr, int32_t min, int32_t sec){
	if (sec != lastsec){
		//draw second
		lastsec = sec;
		DigitalHour(hr);
		DigitalMinute(min);
		DigitalSecond(sec);
		if (min != lastmin){ 
			lastmin = min;
			DigitalHour(hr);
			DigitalMinute(min);
			DigitalSecond(sec);
		}
	}
}
int main(void){uint32_t i;
	int32_t time = 0;
	int32_t min;
	int32_t sec;
	int32_t lasttime=0;
	int32_t hr;
	int32_t state = 1; //1=analog,2=digital 3=set-time 4=set-alarm
	int32_t rstate = 1;
	int32_t ainit =0;
	int32_t dinit = 0;
	
  PLL_Init(Bus80MHz);
  PortF_Init();
	SYSCTL_RCGCGPIO_R |= 0x20; 
  ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_WHITE);  		// set screen to black
  ST7735_SetCursor(0,0);
	GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                     				  // turn off LED
	Timer0A_Init1HzInt();	
	DrawClockInit();
	ainit = 1;
	EnableInterrupts();
	
  while(1){
		while(time == lasttime){			// wait for whattime() to return a new value 
			time = whattime();  			// whattime() returns the current time from in seconds, range 0 < time < 86400			
		}
		time = whattime();
		if(time!=lasttime){
		lasttime = time;
		sec = getSec(time);
		min = getMin(time);				// calculate minutes, range 0< min < 60
		hr = getHr(time);
		// ANALOG CLOCK MODE, CALLS UPDATE CLOCK ROUTINE
		if (state == 1) {	
		 if (ainit == 0){
					DrawClockInit();
					ainit=1;
			}
			UpdateAnalog(hr, min, sec);
		}
		// DIGITAL CLOCK MODE, CALLS UPDATE CLOCK ROUTINE
		if (state == 2) { 				// DIGITAL CLOCK MODE
			if (dinit == 0){
					DigitalInit();
					dinit=1;
			}
			UpdateDigital(hr,min,sec);
		}
		//if button press, switch clock mode: state = 2;
		
		//if button press, go into time-set menu and save last state into rstate
		if (state == 3){
			dinit=0;
			ainit=0;
			timeSet(hr,min,sec);
		}
		//if button press, go into alarm menu;
	}
  } 
} 

// PF4 is input
// Make PF2 an output, enable digital I/O, ensure alt. functions off
void PortF_Init(void){ 
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2, PF4
  GPIO_PORTF_PCTL_R &= ~0x000F0F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x14;      // 4) disable analog function on PF2, PF4
  GPIO_PORTF_PUR_R |= 0x10;         // 5) pullup for PF4
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTF_AFSEL_R &= ~0x14;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x14;         // 7) enable digital port
}
