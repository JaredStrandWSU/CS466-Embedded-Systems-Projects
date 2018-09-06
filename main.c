//      Multicolor LED tolggles between red-green. 
//      When SW1 pressed the LED toggles blue-green.

// 0.Documentation Section 
// main.c
// Runs on LM4F120 or TM4C123
// Lab1_HelloLaunchPad, Input from PF4, output to PF3,PF2,PF1 (LED)

// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

// 1. Pre-processor Directives Section
// Constant declarations to access port registers using 
// symbolic names instead of addresses
//#include "TExaS.h"
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

// 2. Declarations Section
//   Global Variables
unsigned long sw1In;		//input from PF4
unsigned long sw2In;// input from PF0
unsigned long Out; // outputs to PF3,PF2,PF1 (multicolor LED)

//   Function Prototypes
void PortF_Init(void);
void Delay(void);
//void EnableInterrupts(void);


// 3. Subroutines Section
// MAIN: Mandatory for a C Program to be executable
int main(void){    
	int i,j = 0;
  PortF_Init();        // Call initialization of port PF4 PF2    
  while(1){
		sw1In = GPIO_PORTF_DATA_R&0x10; // read PF4 in to detect if sw1 is pressed by &-ing the register
		sw2In = GPIO_PORTF_DATA_R&0x01; // read PF0 in to detect if sw2 is pressed by &-ing the register
    
		Delay(); //Delay so that you can press two buttons before triggering the single button case
		
		if(sw1In == 0x00 && sw2In == 0x00){ // If sw1 and sw2 are pressed, the &-ing is 1 to 1.
      GPIO_PORTF_DATA_R = 0x06;  // Set LED to Pink, Hex 0x06 sets the binary RGB scheme
			for (i = 0; i < 40; i++) // delay for 4 second
			{
				Delay();
				sw1In = GPIO_PORTF_DATA_R&0x10; // Reread the status of the register to get the most up to date value
				if (sw1In == 0x00 ) // if sw1 is pressed
				{
					GPIO_PORTF_DATA_R = 0x08; // set LED Green
					for (j = 0; j < 10; j++) // delay for 1 second
					{
						Delay();
					}
					i = 40; //Exit pink loop early
				}
			}
				
		}
		else if(sw1In == 0x00 && sw2In != 0x00){ // If sw1 is pushed, and sw2 is not pressed
      GPIO_PORTF_DATA_R = 0x02;  // Set LED to red
			for (i = 0; i < 10; i++) // delay for 1 second
			{
				Delay();
			}
		}
		else if(sw2In == 0x00 && sw1In != 0x00){ // If sw2 is pushed, and sw1 is not pressed
      GPIO_PORTF_DATA_R = 0x04;  // Set LED to Blue
			for (i = 0; i < 20; i++) //delay for 2 seconds
			{
				Delay();
			}
		}
		else{
			GPIO_PORTF_DATA_R = 0x00; //Set LED to Dark mode
		}
  }
}

// Subroutine to initialize port F pins for input and output
// PF4 and PF0 are input SW1 and SW2 respectively
// PF3,PF2,PF1 are outputs to the LED
// Inputs: None
// Outputs: None
// Notes: These five pins are connected to hardware on the LaunchPad
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay   
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
  //GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  //GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

// Subroutine to wait 0.1 sec
// Inputs: None
// Outputs: None
// Notes: ...
void Delay(void){unsigned long volatile time;
  time = 727240*200/999;  // 0.1sec
  while(time){
		time--;
  }
}
