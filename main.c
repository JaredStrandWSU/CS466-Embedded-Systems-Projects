//      Project 3.
//			4 way intersection

// 0.Documentation Section 
// main.c
// Runs on LM4F120 or TM4C123

// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
//#include "TExaS.h"

//MACRO
#define CHECK_BIT(n,k) (n & ( 1 << k )) >> k

#define GPIO_PORTD_DATA_R       (*((volatile unsigned long *)0x400073FC))
#define GPIO_PORTD_DIR_R        (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_PUR_R        (*((volatile unsigned long *)0x40007510))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_LOCK_R       (*((volatile unsigned long *)0x40007520))
#define GPIO_PORTD_CR_R         (*((volatile unsigned long *)0x40007524))
#define GPIO_PORTD_AMSEL_R      (*((volatile unsigned long *)0x40007528))
#define GPIO_PORTD_PCTL_R       (*((volatile unsigned long *)0x4000752C))

//This part is for accessing Port E registers:
#define GPIO_PORTE_DATA_R     (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R      (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R    (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R      (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R      (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_LOCK_R     (*((volatile unsigned long *)0x40024520))
#define GPIO_PORTE_CR_R       (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_AMSEL_R    (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R     (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R        (*((volatile unsigned long *)0x400FE108))

//This part is for accessing Port F registers:
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
	
//This section is dedicated to initializing Port D pins for inputs switches
void PortD_Init(void) {									//Input Switches
    volatile unsigned long delay;
 
    SYSCTL_RCGC2_R    |= 0x00000008;     // 1) D clock
    delay              = SYSCTL_RCGC2_R; // delay
    GPIO_PORTD_LOCK_R  = 0x4C4F434B;     // 2) unlock PortD
    GPIO_PORTD_CR_R   |= 0x0F;           // allow changes to PD0- PD3
    GPIO_PORTD_AMSEL_R = 0x00;           // 3) disable analog function
    GPIO_PORTD_PCTL_R  = 0x00;           // 4) GPIO clear bit PCTL
    GPIO_PORTD_DIR_R  |= 0x00;           // 5) PD0-4 dedicated to input
    GPIO_PORTD_AFSEL_R = 0x00;           // 6) no alternate function
	  //GPIO_PORTF_PUR_R = 0x0F;          	 // 8) enable pullup resistors on PD0-PD3, we want pulldown resistors
    GPIO_PORTD_DEN_R  |= 0x0F;           // 7) enable digital pins PD0-PD3
}

// This part is for initializing Port E pins for output street light LEDs
void PortE_Init(void) {									 //Output LEDs
    volatile unsigned long delay;
 
    SYSCTL_RCGC2_R    |= 0x00000010;     // 1) E clock
    delay              = SYSCTL_RCGC2_R; // delay
    GPIO_PORTE_LOCK_R  = 0x4C4F434B;     // 2) unlock PortE
    GPIO_PORTE_CR_R   |= 0x3F;           // 3) allow changes to PE0 - PE5 0x3F = 0000...0011 1111
    GPIO_PORTE_AMSEL_R = 0x00;           // 4) disable analog function
    GPIO_PORTE_PCTL_R  = 0x00;           // 5) GPIO clear bit PCTL
		GPIO_PORTE_DIR_R  |= 0x3F;           // 6) PE0,PE1,PE2,PE3,PE4,PE5 outputs : 0x3F = 0000...0011 1111
    GPIO_PORTE_AFSEL_R = 0x00;           // 7) no alternate function
    GPIO_PORTE_DEN_R  |= 0x3F;           // 8) enable digital pins PE0-PE5
}

// This part is for initializing Port F pins for output pedestrian LEDs
void PortF_Init(void){ 										//Onboard Multi-colored LED
	volatile unsigned long delay;
  
	SYSCTL_RCGC2_R |= 0x00000020;     			// 1) F clock
	delay     				= SYSCTL_RCGC2_R;			// delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   			// 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           			// 3) allow changes to PF1 - PF4
  //GPIO_PORTF_AMSEL_R = 0x00;        		// 4) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   			// 5) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          			// 6) PF0-PF4 as outputs 
  //GPIO_PORTF_AFSEL_R = 0x00;        		// 7) no alternate function
  //GPIO_PORTF_PUR_R = 0x00;          			// 8) enable pullup resistors on PF0-PF2
  GPIO_PORTF_DEN_R = 0x1F;          			// 9) enable digital pins PF0-PF2 
}

void Delay(void){
	unsigned long volatile time;
  
	time = 727240*200/999;  // 0.1sec
  while(time){
		time--;
  }
}

// 2. Declarations Section
//   Global Variables

unsigned long MAIN_STREET_PEDESTRIAN_SENSOR;		//INPUT : PD0
unsigned long SPRING_STREET_PEDESTRIAN_SENSOR;	//INPUT : PD1

unsigned long MAIN_STREET_VEHICLE_SENSOR;				//INPUT : PD2
unsigned long SPRING_STREET_VEHICLE_SENSOR;			//INPUT : PD3

unsigned long MAIN_STREET_LIGHT_RED;			//OUTPUT : PE0
unsigned long MAIN_STREET_LIGHT_YELLOW;		//OUTPUT : PE1
unsigned long MAIN_STREET_LIGHT_GREEN;		//OUTPUT : PE2

unsigned long SPRING_STREET_LIGHT_RED;		//OUTPUT : PE3
unsigned long SPRING_STREET_LIGHT_YELLOW;	//OUTPUT : PE4
unsigned long SPRING_STREET_LIGHT_GREEN;	//OUTPUT : PE5

unsigned long PEDESTRIAN_SIGNAL_LIGHT;		//OUTPUT : PF1, PF2, PF3

//E-port - Light Signals
//	SPRING \	 MAIN
// 1  1  1    1  1  1
// p5 p4 p3   p2 p1 p0
// G	Y  R    G  Y  R

//GPIO
//Main Red,		Spring Red = 001001 = 0x09
//Main Yellow,Spring Red = 001010 = 0x0A
//Main Green, Spring Red = 001100 = 0x0C

//Spring Red,		Main Red = 001001 = 0x09
//Spring Yellow,Main Red = 010001 = 0x11
//Spring Green, Main Red = 100001 = 0x21

//F-port - Ped RGB LED PF3 green Led for walk, PF1 red LED for dont walk
// GPIO_PORTF_DATA_R = 0x08; //Ped Led is Green
// GPIO_PORTF_DATA_R = 0x02; //Ped Led is Red
// GPIO_PORTF_DATA_R = 0x00; //Ped Led is Off

//D-Port - Pedestrian buttons and vehicle signals
//if GPIO_PORTD_DATA_R = 0001 = 0x01 : then PD0 is pressed -> MAIN_STREET_PEDESTRIAN_SENSOR
//if GPIO_PORTD_DATA_R = 0010 = 0x02 : then PD1 is pressed -> SPRING_STREET_PEDESTRIAN_SENSOR
//if GPIO_PORTD_DATA_R = 0100 = 0x04 : then PD2 is pressed -> MAIN_STREET_VEHICLE_SENSOR
//if GPIO_PORTD_DATA_R = 1000 = 0x08 : then PD3 is pressed -> SPRING_STREET_VEHICLE_SENSOR


//   Function Prototypes
void PortD_Init(void);
void PortE_Init(void);
void PortF_Init(void);
void Delay(void);

// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){    
	int i = 0;
	PortD_Init();
  PortE_Init(); // Call initialization of port PE4 PE0
	PortF_Init(); // Call initialization of ports : PF0 - PF3
	GPIO_PORTF_DATA_R = 0x02; //Ped Led is Red
	GPIO_PORTE_DATA_R = 0x0C; //Green Main, Red Spring
	GPIO_PORTD_DATA_R = 0x00; //Clear Sensors
	
	while(1){
	
		//Read buttons
	
		MAIN_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x01;
	
		SPRING_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x02;
	
		MAIN_STREET_VEHICLE_SENSOR = GPIO_PORTD_DATA_R&0x04;

		SPRING_STREET_VEHICLE_SENSOR = GPIO_PORTD_DATA_R&0x08;
	
		//Read Lights Red Lights
		
		if (GPIO_PORTE_DATA_R << 31)
		{
			//MAIN STREET LIGHT IS RED
			MAIN_STREET_LIGHT_RED = 0x01;
		}
		
		SPRING_STREET_LIGHT_RED = GPIO_PORTE_DATA_R >> 3;
		if(SPRING_STREET_LIGHT_RED << 31)
		{
			//SPRING STREET LIGHT IS RED
			SPRING_STREET_LIGHT_RED = 0x01;
		}
		
		Delay();
		
			//if main street is selected
		if ( MAIN_STREET_VEHICLE_SENSOR != 0x00 && SPRING_STREET_LIGHT_RED == 0x01) //button pressed and spring street is Red : pE3 is 1
			{
				//set green for 5 seconds then
				//Main Green, Spring Red = 001100 = 0x0C - portE
				GPIO_PORTE_DATA_R = 0x0C;
				for(i = 0; i < 20; i++)
				{
					Delay();	//5 second delay : sub in 2
				}
				
				//Begin checking if any button is pressed or 30 seconds has passed
				for (i = 0; i < 25; i++) //sub in 25, revert back to 250
				{
					Delay(); //Delay
					SPRING_STREET_VEHICLE_SENSOR = GPIO_PORTD_DATA_R&0x08; //Read spring street sensor
					MAIN_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x04;		//Read Main ped sensor
					SPRING_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x02;	//Read Spring ped sensor
					
					//check if another car is coming on spring or ped wants to cross
					if( SPRING_STREET_VEHICLE_SENSOR || MAIN_STREET_PEDESTRIAN_SENSOR || SPRING_STREET_PEDESTRIAN_SENSOR )
					{
						break;
					}
				}
				
				//begin transition back to red
				//set yellow for 2 seconds
				GPIO_PORTE_DATA_R = 0x0A; //Main yellow, Spring Red
				for (i = 0; i < 20; i++)
				{
					Delay();
				}
				
				if(SPRING_STREET_VEHICLE_SENSOR > 1 )
				{
					//set to red and spring to green
					GPIO_PORTE_DATA_R = 0x21; //Main Red, Spring Green
				}
				else
				{
					//Set both to red from timeout
					GPIO_PORTE_DATA_R = 0x09; //Main Red, Spring red
				}
				
				MAIN_STREET_VEHICLE_SENSOR = 0x00;
			}
			
			else if( SPRING_STREET_VEHICLE_SENSOR != 0x00 && MAIN_STREET_LIGHT_RED == 0x01 ) //button is pressed and Main is Red : pE0 is 1
			{
				//set green for 5 seconds then
				//Spring Green, Main Red = 100001 = 0x21 - PortE
				GPIO_PORTE_DATA_R = 0x21;
				for(i = 0; i < 20; i++)
				{
					Delay();	//5 second delay Sub in 2
				}
				
				//Begin checking if any button is pressed or 30 seconds has passed
				for (i = 0; i < 25; i++) //Sub back to 250
				{
					Delay(); //Delay
					MAIN_STREET_VEHICLE_SENSOR = GPIO_PORTD_DATA_R&0x01; 			//Read main street sensor
					MAIN_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x04;		//Read Main ped sensor
					SPRING_STREET_PEDESTRIAN_SENSOR = GPIO_PORTD_DATA_R&0x02;	//Read Spring ped sensor
					
					//check if another car is coming on spring or ped wants to cross
					if( MAIN_STREET_VEHICLE_SENSOR || MAIN_STREET_PEDESTRIAN_SENSOR || SPRING_STREET_PEDESTRIAN_SENSOR )
					{
						break;
					}
				}
				
				//begin transition back to red
				//set yellow for 2 seconds
				GPIO_PORTE_DATA_R = 0x11; //Spring yellow, Main Red = 010001 = 0x11
				for (i = 0; i < 20; i++)
				{
					Delay();
				}
				
				if(MAIN_STREET_VEHICLE_SENSOR > 1 )
				{
					//set to red and main to green
					GPIO_PORTE_DATA_R = 0x0C; //Main Green, Spring Red = 001100 = 0x0C
				}
				else
				{
					//Set both to red from timeout
					GPIO_PORTE_DATA_R = 0x09; //Main Red, Spring red
				}
				
				SPRING_STREET_VEHICLE_SENSOR = 0x00;
			}
			
			if( MAIN_STREET_PEDESTRIAN_SENSOR != 0x00 && GPIO_PORTE_DATA_R == 0x09) //both lights red port E = 0x09 both red
			{
				//allow person to cross
				//turn crossing LED to green for 5 seconds
				//OnBoard LED Green = 0x08 - PortF
				GPIO_PORTF_DATA_R = 0x08;
				for(i = 0; i < 50; i++)
				{
					Delay();	//5 second delay
				}
				
				//Blink Crossing LED for 3 Seconds
				
				for(i = 0; i < 30; i++)
				{
					//set dark
					GPIO_PORTF_DATA_R = 0x00; //Ped Led is Off
					
					Delay();	//.1 second delay
					
					//set red
					GPIO_PORTF_DATA_R = 0x02; //Ped Led is Red
					
					Delay();
				}
				
				MAIN_STREET_PEDESTRIAN_SENSOR = 0x00;
			}
			
			if( SPRING_STREET_PEDESTRIAN_SENSOR != 0x00 && GPIO_PORTE_DATA_R == 0x09)//both lights red
			{
				//allow person to cross
				//turn crossing LED to green for 5 seconds
				//OnBoard LED Green = 0x08 - PortF
				GPIO_PORTF_DATA_R = 0x08;
				for(i = 0; i < 50; i++)
				{
					Delay();	//5 second delay
				}
				
				//Blink Crossing LED for 3 Seconds
				
				for(i = 0; i < 30; i++)
				{
					//set dark
					GPIO_PORTF_DATA_R = 0x00; //Ped Led is Off
					
					Delay();	//.1 second delay
					
					//set red
					GPIO_PORTF_DATA_R = 0x02; //Ped Led is Red
					
					Delay();
				}
				
				SPRING_STREET_PEDESTRIAN_SENSOR = 0x0;
			}
		}
}
