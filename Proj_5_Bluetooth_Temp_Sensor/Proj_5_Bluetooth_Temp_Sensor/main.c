// ADCTestMain.c
// Runs on LM4F120/TM4C123
// This program periodically samples ADC channel 1 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.
// Daniel Valvano
// October 20, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


// input signal connected to PE2/AIN1

#include "ADCSWTrigger.h"
#include "PeriodicSysTickInts.h"
#include "uart.h"
#include "../tm4c123gh6pm.h"
#include "PLL.h"

// The digital number ADCvalue is a representation of the voltage on PE4 
// voltage  ADCvalue	Temp (C)
// 0.00V     0				 -40	
// 0.75V    1024			 25
// 1.50V    2048			 50
// 2.25V    3072			 75
// 2.70V    4095		   125
//-40°C = TA = +125°C -- 165 values range
//10mV per 1 Celcius

#define MAX 100

//GLOBALS
volatile unsigned long Counts = 0;
volatile unsigned long ADCvalue;
float Celcius;
float Farenheit;
unsigned long CelciusFront;
unsigned long CelciusBack;
unsigned long FarenheitFront;
unsigned long FarenheitBack;



//FUNCTIONS//
// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 62.5ns (assuming 16 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(unsigned long period){
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
                              // enable SysTick with core clock and interrupts
  NVIC_ST_CTRL_R = 0x07;
  EnableInterrupts();
}
// Interrupt service routine
// Executed every 62.5ns*(period)
void SysTick_Handler(void){
	ADCvalue = ADC0_InSeq3();
	UartOutString("ADCval: ");
	UartOutNum(ADCvalue);
	UartOutString(",");
	
	//Celcius = ADCvalue / 24.8242424242;
	//may need to move decimal place 2 to left before using below
	Celcius = (ADCvalue*(3.3/4096)-0.5)/0.0001;
	Celcius = Celcius / 100;
	
	//decimal part of celsius = -- truncate off floating point section
	CelciusFront = (unsigned long)Celcius;
	//Floating point part = subtract off front part and then 
	//move decimal right 2 places, may need to store the result in float then cast
	CelciusBack = (unsigned long)((Celcius - CelciusFront) * 100);
	//get the front decimal values, print out
	UartOutString("TEMP in C: ");
	UartOutNum(CelciusFront);
	UartOutString(".");
	UartOutNum(CelciusBack);
	UartOutString(",");
	

	Farenheit = ((Celcius * 9) / 5 ) + 32;
	//decimal part of celsius = -- truncate off floating point section
	FarenheitFront = (unsigned long)Farenheit;
	//Floating point part = subtract off front part and then 
	//move decimal right 2 places, may need to store the result in float then cast
	FarenheitBack = (unsigned long)((Farenheit - FarenheitFront) * 100);
	//get the front decimal values, print out
	UartOutString("  TEMP in F: ");
	UartOutNum(FarenheitFront);
	UartOutString(".");
	UartOutNum(FarenheitBack);

	UartOutString("\n");
  Counts = Counts + 1;
}

//
int main(void){unsigned long volatile delay;
	//Initialize Bluetooth
	UartInit();        // initialize UART
  UartOutString("Bluetooth Transmitter ON");
	
	//Temperature sensor initialization
  PLL_Init();                           // 80 MHz
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1
	
	// Init SysTick
  SysTick_Init(16000);        // initialize SysTick timer
	Counts = 0;
  EnableInterrupts();
	
  while(1){
		WaitForInterrupt();									// interrupts every 1ms, 500 Hz flash
		
    //pass ADC valur into our celcius converter and send through bluetooth to the term.
			//get the value of celcius to return to bluetooth
			//send the value to be printed from BT module with followup string
			
    for(delay=0; delay<100000; delay++){};
  }
}
