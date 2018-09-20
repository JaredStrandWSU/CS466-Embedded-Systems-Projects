//      Stoplight simulator with pedestrian and oncoming
//			vehicle indicator buttons. 

// 0.Documentation Section 
// main.c
// Runs on LM4F120 or TM4C123
// Lab2_HelloLaunchPad, Input from PE4, PE0, output to PE3,PE2,PE1 (LED)

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

//and this part is for initializing Port E pins for inputs and outputs

void PortE_Init(void) {
    volatile unsigned long delay;
 
    SYSCTL_RCGC2_R    |= 0x00000010;     // 1) E clock
    delay              = SYSCTL_RCGC2_R; // delay
    GPIO_PORTE_LOCK_R  = 0x4C4F434B;     // 2) unlock PortE
    GPIO_PORTE_CR_R   |= 0x1F;           // allow changes to PE0- PE4
    GPIO_PORTE_AMSEL_R = 0x00;           // 3) disable analog function
    GPIO_PORTE_PCTL_R  = 0x00;           // 4) GPIO clear bit PCTL
    GPIO_PORTE_DIR_R  |= 0x0E;           // 5) PE0,PE4 inputs, PE1,PE2,PE3 outputs
    GPIO_PORTE_AFSEL_R = 0x00;           // 6) no alternate function
    GPIO_PORTE_DEN_R  |= 0x1F;           // 7) enable digital pins PE0-PE4
}

void Delay(void){unsigned long volatile time;
  time = 727240*200/999;  // 0.1sec
  while(time){
		time--;
  }
}
// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
//#include "TExaS.h"

// 2. Declarations Section
//   Global Variables
unsigned long PEDswIn;		//input from PE4, TOP SWITCH, PEDESTRIAN SIGNAL
unsigned long CARswIn;// input from PE0, BOTTOM SWITCH, CAR COMING
unsigned long greenOut; // outputs to PE3,PE2,PE1 (Green, Yellow, Red LEDs respectively)
unsigned long yellowOut; // outputs to PE3,PE2,PE1 (Green, Yellow, Red LEDs respectively)
unsigned long redOut; // outputs to PE3,PE2,PE1 (Green, Yellow, Red LEDs respectively)

//   Function Prototypes
void PortE_Init(void);
void Delay(void);


// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){    
	int i = 0;
  PortE_Init();        // Call initialization of port PE4 PE0
	
  while(1){
		PEDswIn = GPIO_PORTE_DATA_R&0x10; // read PE4 in to detect if sw1 is pressed by &-ing the register, if 0 not pressed else pressed

		Delay(); //Delay so that you can press two buttons before triggering the single button case
		//set value of data to 0
		//GPIO_PORTE_DATA_R = 0x08; //turn on green led only
		//GPIO_PORTE_DATA_R = 0x04; //turn on yellow led only
		//GPIO_PORTE_DATA_R = 0x02; //turn on red led only
		
		
		//set green to on
		GPIO_PORTE_DATA_R = 0x08; //turn on green led only

		//check if pedestrian button is pressed
		if (PEDswIn != 0) //if ped button is pressed
		{//start transition to red
				//delay for 0.2 seconds
				Delay();
				Delay();
				//turn off green
				//turn on yellow
				GPIO_PORTE_DATA_R = 0x04; //turn on yellow led only
				//delay for 2 seconds
				for (i = 0; i < 20; i++)
				{
					Delay();
				}
				//turn off yellow
				//turn on red
				GPIO_PORTE_DATA_R = 0x02; //turn on red led only
				
				//delay for 5 seconds
				for (i = 0; i < 50; i++)
				{
					Delay();
				}
				while(1)
				{	//while car incoming button not pressed, loop
					CARswIn = GPIO_PORTE_DATA_R&0x01; // read PE0 in to detect if sw2 is pressed by &-ing the register, if 0 not pressed else pressed
					//if car button is pressed
					if(CARswIn != 0)
					{
						//Turn off red & turn on green
						GPIO_PORTE_DATA_R = 0x08; //turn on green led only
						for (i = 0; i < 20; i++)
						{
							Delay();
						}
						break;
						//break from loop
					}
				//else loop
				}
		//else loop
			}
		}
}