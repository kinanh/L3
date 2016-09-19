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


void ST7735_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	// Draws one line on the ST7735 color LCD
	//  Inputs: (x1,y1) is the start point, (x2,y2) is the end point
	    
	for(int16_t i = x1; i < x2; i += 1) {
		ST7735_DrawPixel(i, y1, ST7735_BLUE);
	}
	
	for(int16_t j = y1; j < y2; j += 1) {
		ST7735_DrawPixel(x1, j, ST7735_BLUE);
	}
}

int main(void){uint32_t i;
	int32_t time;
	int32_t min=0;
	int32_t sec;
	int32_t ls=0;
	int32_t hr=0;
	int32_t lm=0;
	int32_t tmp;
	int32_t pm = 0;
	int32_t analog = 1;
	int32_t ainit =0;
	
  PLL_Init(Bus80MHz);
  PortF_Init();
	SYSCTL_RCGCGPIO_R |= 0x20; 
  ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(ST7735_WHITE);  // set screen to black
  ST7735_SetCursor(0,0);
	GPIO_PORTF_DIR_R |= 0x06;             // make PF2, PF1 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x06;          // disable alt funct on PF2, PF1
  GPIO_PORTF_DEN_R |= 0x06;             // enable digital I/O on PF2, PF1
                                        // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;               // disable analog functionality on PF
  PF2 = 0;                      // turn off LED
	Timer0A_Init1HzInt();	
	DrawClockInit();
	ainit = 1;
	EnableInterrupts();
	
  while(1){
		while(sec==ls){
			time = whattime();  
			sec = time%60;
		}
    		time = whattime();  //test comment
		sec = time%60;
		min = floor(time%(60*60)/60);
		if (time<43200){
			hr = ((time*60/(60*60))/12);
			if (pm == 0){
				pm = 1;
				day();
			}
		}else{
			hr = (((time-43200)*60/(60*60))/12);
			if (pm == 1){
				pm=0;
				night();
			}
		}
		if (analog == 1) {		//ANALOG CLOCK MODE
			if (ainit == 0){
					ST7735_FillScreen(ST7735_WHITE);
					DrawClockInit();
					ainit=1;
			}
			if (sec != ls){
			//draw second
					ls = sec;
					//DrawBackground();
					DrawHour(hr);
					DrawMinute(min);
					DrawSecond(sec);
				if (min != lm){ 
						lm = min;
						//DrawBackground();
						DrawHour(hr);
						DrawMinute(min);
						DrawSecond(sec);
				}
			}
		}
		if (analog == 0) { 				// DIGITAL CLOCK MODE
			if (ainit == 1){
					ST7735_FillScreen(ST7735_WHITE);
					DigitalInit();
					ainit=0;
			}
			if (sec != ls){
			//draw second
					ls = sec;
					DigitalHour(hr);
					DigitalMinute(min);
					DigitalSecond(sec);
				if (min != lm){ 
						lm = min;
						DigitalHour(hr);
						DigitalMinute(min);
						DigitalSecond(sec);
				}
			}
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


int main41(void){ 
  PortF_Init();
  while(1){
    DelayWait10ms(1);
    PF2 ^= 0x04;
  }
}
int main51(void){ 
  PortF_Init();
  while(1){
    DelayWait10ms(100);
    PF2 ^= 0x04;
  }
}
//********************************* ST7735TestMain.c Test Cases ***************************
//-----------------------------------------------------------------------------------------

int main0(void){
  PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
  Output_Init();
  printf("hello world");
  while(1){
  }
}

int main1(void){uint32_t j; // main 1
  PLL_Init(Bus80MHz);                  // set system clock to 80 MHz
  ST7735_InitR(INITR_REDTAB);
  ST7735_OutString("Graphics test\n");
  ST7735_OutString("cubic function\n");
  ST7735_PlotClear(-2000,2095);  // range from 0 to 4095
  for(j=0;j<128;j++){
    ST7735_PlotPoints(j*j/2+900-(j*j/256)*j,32*j); // cubic,linear
    ST7735_PlotNext();
  }   // called 128 times
  while(1){
  }
}

void BookExamples(void){ // examples from the book
  int8_t cc = 0x56; // (‘V’)
  int32_t xx = 100;
  int16_t yy = -100;
  float zz = 3.14159265;
  printf("Hello world\n");      //Hello world
  printf("cc = %c %d\n",cc,cc);  //cc = V 86
  printf("cc = %#x\n",cc);      //cc = 0x56
  printf("xx = %c %d\n",xx,xx);  //xx = d 100
  printf("xx = %#x\n",xx);      //xx = 0x64
  printf("yy = %d\n",yy);        //yy = -100
  printf("%#x   \n",yy);        //yy = 0xffffff9c
  printf("%e \n",zz);            //zz = 3.14159e+00
  printf("%E \n",zz);            //zz = 3.14159E+00
  printf("%f     \n",zz);        //zz = 3.14159
  printf("%g     \n",zz);        //zz = 3.14159 (shorter of two, either f or e)
  printf("%3.2f     \n",zz);    //zz =  3.14
}
#define PF2   (*((volatile uint32_t *)0x40025010))

// Make PF2 an output, enable digital I/O, ensure alt. functions off
void SSR_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;        // 1) activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){}; // allow time for clock to start
                                    // 2) no need to unlock PF2
  GPIO_PORTF_PCTL_R &= ~0x00000F00; // 3) regular GPIO
  GPIO_PORTF_AMSEL_R &= ~0x04;      // 4) disable analog function on PF2
  GPIO_PORTF_DIR_R |= 0x04;         // 5) set direction to output
  GPIO_PORTF_AFSEL_R &= ~0x04;      // 6) regular port function
  GPIO_PORTF_DEN_R |= 0x04;         // 7) enable digital port
}


void Delay1ms(uint32_t n);
int main4(void){
  SSR_Init();
  while(1){
    Delay1ms(10);
    PF2 ^= 0x04;
  }
}
int main5(void){
  SSR_Init();
  while(1){
    DelayWait10ms(1000);
    PF2 ^= 0x04;
  }
}
int main6(void){ int32_t i,n; // main 6
  Output_Init();              // initialize output device
  Output_Color(ST7735_YELLOW);
  BookExamples();
  n = 0;
  while(1){
    printf("\ni=");
    for(i=0; i<1; i++){
      printf("%d ",i+n);
    }

    n = n+10000000; // notice what happens when this goes above 2,147,483,647
  }
}

