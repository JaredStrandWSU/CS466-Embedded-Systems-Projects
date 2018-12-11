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
#include "tm4c123gh6pm.h"
#include "PLL.h"

// The digital number PhotoCell_ADCvalue is a representation of the voltage on PE4 
// voltage  PhotoCell_ADCvalue	Temp (C)
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
volatile unsigned long PhotoCell_ADCvalue;
volatile unsigned long Temperature_ADCvalue;
float Celcius;
float Farenheit;
unsigned long CelciusFront;
unsigned long CelciusBack;
unsigned long FarenheitFront;
unsigned long FarenheitBack;
unsigned long screen;



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
	//Read analog value from temperture sensor
	Temperature_ADCvalue = ADC0_InSeq3(); //Reads value from ADC0, PE1
	//Read analog value from photosensor
	PhotoCell_ADCvalue = ADC1_InSeq3();  //Reads value from ADC1
	
	//UartOutString("Temperature_ADCvalue: ");
	//UartOutNum(Temperature_ADCvalue);
	//UartOutString(",");
	
	//Celcius = PhotoCell_ADCvalue / 24.8242424242;
	//may need to move decimal place 2 to left before using below
	Celcius = (Temperature_ADCvalue*(3.3/4096)-0.5)/0.0001;
	Celcius = Celcius / 100;
	
	//decimal part of celsius = -- truncate off floating point section
	CelciusFront = (unsigned long)Celcius;
	//Floating point part = subtract off front part and then 
	//move decimal right 2 places, may need to store the result in float then cast
	CelciusBack = (unsigned long)((Celcius - CelciusFront) * 100);
	//get the front decimal values, print out
	//UartOutString("TEMP in C: ");
	//UartOutNum(CelciusFront);
	//UartOutString(".");
	//UartOutNum(CelciusBack);
	//UartOutString(",");
	

	Farenheit = ((Celcius * 9) / 5 ) + 32;
	
	
	/*
		IMPLEMENT MODEL GENERATED FROM WEKA
		VARS:
		Farenheit : Temperature sensor value as a flaoting point : Temp in model
		PhotoCell_ADCvalue : value read from photo sensor at different scenarios : LightValue in model
	*/
	/*
		This model calculates from dereived light sensor values only, temperature is irrlevant for
		this model. Future plans to add sensativity to predict inside, outside, etc. The model is
		implemented with a Decision tree.
	*/
	
	if( PhotoCell_ADCvalue > 1010 ) //RIGHT TREE
	{
		if( PhotoCell_ADCvalue > 2207 )
		{
			//Label : on ( 22007.0 )
			screen = 1; //screen on
		}
		else if( PhotoCell_ADCvalue <= 2207 )
		{
			if( PhotoCell_ADCvalue > 2082 )
			{
				if( PhotoCell_ADCvalue > 2147 )
				{
					if( PhotoCell_ADCvalue > 2187 )
					{
						//Label : on ( 426.0/164.0 )
						screen = 1; //screen on
					}
					else if( PhotoCell_ADCvalue <= 2187 )
					{
						//Label : off ( 2554.0/1129.0 )
						screen = 0; //screen off
					}
					else
					{
						//Catch all : MODEL ERROR
						screen = 3; //error code
					}
				}
				else if( PhotoCell_ADCvalue <= 2147 )
				{
					//Label : off ( 5190.0/920.0 )
						screen = 0; //screen off
				}
				else
				{
					//Catch all : MODEL ERROR
					screen = 3; //error code
				}
			}
			else if( PhotoCell_ADCvalue <= 2082 )
			{
				if( PhotoCell_ADCvalue > 2002 )
				{
					if( PhotoCell_ADCvalue > 2055 )
					{
						//Label : on ( 5558.0/939.0 )
						screen = 1; //screen on
					}
					else if( PhotoCell_ADCvalue <= 2055 )
					{
						if( PhotoCell_ADCvalue > 2024 )
						{
							if( PhotoCell_ADCvalue > 2051 )
							{
								if( PhotoCell_ADCvalue > 2052 )
								{
									//Label : on ( 185.0/75.0 )
									screen = 1; //screen on
								}
								else if( PhotoCell_ADCvalue <= 2052 )
								{
									//Label : off ( 43.0/18.0 )
									screen = 0; //screen off
								}
								else
								{
									//Catch all : MODEL ERROR
									screen = 3; //error code
								}
							}
							else if( PhotoCell_ADCvalue <= 2051 )
							{
								//Label : off ( 704.0/173.0 )
								screen = 0; //screen off
							}
							else
							{
								//Catch all : MODEL ERROR
								screen = 3; //error code
							}
						}
						else if( PhotoCell_ADCvalue <= 2024 )
						{
							//Label : on ( 568.0/110.0 )
							screen = 1; //screen on
						}
						else
						{
							//Catch all : MODEL ERROR
							screen = 3; //error code
						}
					}
					else
					{
						//Catch all : MODEL ERROR
						screen = 3; //error code
					}
				}
				else if( PhotoCell_ADCvalue <= 2002 )
				{
					//Label : on ( 11514.0/92.0 )
				screen = 1; //screen on
				}
				else
				{
					//Catch all : MODEL ERROR
					screen = 3; //error code
				}
			}
			else
			{
				//Catch all : MODEL ERROR
				screen = 3; //error code
			}
		}
		else
		{
			//Catch all : MODEL ERROR
			screen = 3; //error code
		}
	}
	else if( PhotoCell_ADCvalue <= 1010 ) //LEFT TREE
	{
		if( PhotoCell_ADCvalue > 766 )
		{
			//Label : off ( 17321.0/44.0 )
			screen = 0; //screen off
		}
		else if( PhotoCell_ADCvalue <= 766 )
		{
			if( PhotoCell_ADCvalue > 152 )
			{
				//Label : on ( 31.0 )
				screen = 1; //screen on
			}
			else if( PhotoCell_ADCvalue <= 152 )
			{
				//Label : off ( 16974.0 )
				screen = 0; //screen off
			}
		}
		else
		{
			//Catch all : MODEL ERROR
			screen = 3; //error code
		}
	}
	else
	{
		//Catch all : MODEL ERROR
		screen = 3; //error code
	}
	
	
	
	
	//decimal part of celsius = -- truncate off floating point section
	FarenheitFront = (unsigned long)Farenheit;
	//Floating point part = subtract off front part and then 
	//move decimal right 2 places, may need to store the result in float then cast
	FarenheitBack = (unsigned long)((Farenheit - FarenheitFront) * 100);
	//get the front decimal values, print out
	
	//UartOutString("PhotoCell_ADCvalue: ");
	//UartOutString("Light_Value: ");
	UartOutNum(PhotoCell_ADCvalue);
	UartOutString(",");
	
	//UartOutString("  TEMP in F: ");
	UartOutNum(FarenheitFront);
	UartOutString(".");
	UartOutNum(FarenheitBack);
	
	if( screen == 1 )
	{
		//screen on
		UartOutString(" : SCREEN ON!");
	}
	else if ( screen == 0 )
	{
		//screen off
		UartOutString(" : SCREEN OFF!");
	}
	else if ( screen == 3 )
	{
		UartOutString(" : SCREEN VALUE 3 : MODEL VALUE ERROR!");
	}
	else
	{
		//error
		UartOutString(" : SCREEN VALUE ERROR!");
	}

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
  ADC0_InitSWTriggerSeq3_Ch1();         // ADC initialization PE2/AIN1, initializes both ADC's
	
	
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
