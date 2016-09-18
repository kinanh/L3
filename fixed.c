// filename ******** fixed.c ************** 
// c file for Lab 1 
// feel free to change the specific syntax of your system
// Alice Lam / Kinan Hernandez
// 9/5/16


/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 
#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"

int32_t Ymx,Ymn;  			// Y min and Y max set during initialiazaion and plotting 
int32_t Xmx,Xmn;        // X min and X max for initialiazaion and plotting 
int32_t Yrnge;		
int32_t Xrnge;	

/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.001
 range -9.999 to +9.999
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 
#include <stdio.h>
#include <stdint.h>
#include "ST7735.h"

int32_t Ymx,Ymn;  			// Y min and Y max set during initialiazaion and plotting 
int32_t Xmx,Xmn;        // X min and X max for initialiazaion and plotting 
int32_t Yrnge;		
int32_t Xrnge;	

void ST7735_sDecOut3(int32_t n){
	//CODE
	char number[6];
	int32_t sign = 0;
	int32_t div = 1000;
	int32_t i;
	
	//check >9999 or <-9999
	if ((n>9999)||(n<-9999)){
		number[0] = ' ';                   
		number[1] = '*';
		number[2] = '.';
		number[3] = '*';
		number[4] = '*';
		number[5] = '*';
	
	}
	else{
	//*check neg/pos
	if (n<0){
		sign = -1;
		n=-n;		//change to absolute value
	}
	//check length of int
//		if ((n < 10)&&(n>-10)){ length = 1;}
//		else if ((n < 100)&&(n>-100)){ length = 2;}
//		else if ((n < 1000)&&(n>-1000)){ length = 3;}
//		else if ((n < 10000)&&(n>-10000)){ length = 4;}
	
	//output to screen w/ "."
		
		for(i = 0; i < 6; i++){
			
			if(i == 0 && sign==-1){
				number[i] = '-';
			}
			else if(i == 0 && sign==0){
				number[i] = ' ';
			}
			else if(i == 2){
				number[i] = '.';
			}
			else{
				int num = n/div;
				number[i] = num + '0';
				n = n%div;
				
				div = div / 10;
				
			}
		}
}		
	//fputc(c);? or imp printf
		ST7735_OutString((char*) number); 

}
/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     2	  "  0.01"
    64	  "  0.25"
   100	  "  0.39"
   500	  "  1.95"
   512	  "  2.00"
  5000	  " 19.53"
 30000	  "117.19"
255997	  "999.99"
256000	  "***.**"
*/
void ST7735_uBinOut8(uint32_t n){
	char number[6];
	int32_t div = 1000;
	int32_t i;
	int32_t blank;
	
	//check >256000 or <-256000
	if (n>255999){
		number[0] = '*';                   
		number[1] = '*';
		number[2] = '*';
		number[3] = '.';
		number[4] = '*';
		number[5] = '*';
	}
	else{
	//*1000 then divide by 256
	n = n*1000;
	n = n/256;
	if(n<1000){blank=2;}
	else if ((n>999)&&(n<10000)){blank=2;}
	else if ((n > 9999)&&(n<100000)){
		blank=1;
		div=10000;
	}
	else if (n > 99999){
		blank=0;
		div=100000;
	}
	
	for(i = 0; i < 6; i++){

			if (blank!=0){
			number[i]=' ';
			blank=blank-1;
			}
		
			else if(i == 3){
				number[i] = '.';
			}
			
			else{
				int num = n/div;
				number[i] = num+'0';
				n = n%div;
				div = div / 10;
			}
	}	
}	
	//fputc(c);? or imp printf
		ST7735_OutString((char*) number); 
			
}

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
void ST7735_XYplotInit(int32_t minX, int32_t maxX, int32_t minY, int32_t maxY){
  ST7735_InitR(INITR_REDTAB);
 //ST7735_OutString(title);
 //ST7735_PlotClear(minY, maxY);  // this fuction is used here to clear the screen
	Yrnge = maxY - minY;
	Xrnge = maxX - minX;
	Ymx = maxY; 
	Ymn = minY;			
	Xmx = maxX;
	Xmn = minX;
	
  
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]){	int32_t x; int32_t y;
	// Mix color for ploted points, Red 0-254 
	uint8_t r = 0;																// Red
	uint8_t g = 0;																// Green
	uint8_t b = 254;															// Blue
	uint16_t color = ST7735_Color565(r, g ,b);		// Generates 16-bit color code
	// Scale and plot points to 128 X 128 grid
	for(uint32_t j=0; j<num; j++){
		//y = 32+(127*(Ymx-bufY[j]))/Yrnge;
		y = (127*(Ymx-bufY[j]))/Yrnge;
		x = (127*(-Xmn+bufX[j]))/Xrnge;
		//y = 32+(127*(bufY[j]))/Yrnge;
		//x = (127*(bufX[j]))/Xrnge;
		//Gradient Color
			//ST7735_DrawPixel(x,y,color+31*x/127);
			//ST7735_DrawPixel(x+1,y,color+31*x/127);
			//ST7735_DrawPixel(x,y+1,color+31*x/127);
			//ST7735_DrawPixel(x+1,y+1,color+31*x/127);
		//Normal Color
			ST7735_DrawPixel(x,y,color);
			ST7735_DrawPixel(x+1,y,color);
			ST7735_DrawPixel(x,y+1,color);
			ST7735_DrawPixel(x+1,y+1,color);
	}
}



