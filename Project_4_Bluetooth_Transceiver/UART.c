#include "tm4c123gh6pm.h"
#include "uart.h"
#include "string.h"
#define SYSCTL_RCC2_USERCC2     0x80000000  // Use RCC2
#define SYSCTL_RCC2_DIV400      0x40000000  // Divide PLL as 400 MHz vs. 200
                                            // MHz
#define SYSCTL_RCC2_SYSDIV2_M   0x1F800000  // System Clock Divisor 2
#define SYSCTL_RCC2_SYSDIV2LSB  0x00400000  // Additional LSB for SYSDIV2
#define SYSCTL_RCC2_PWRDN2      0x00002000  // Power-Down PLL 2
#define SYSCTL_RCC2_BYPASS2     0x00000800  // PLL Bypass 2
#define SYSCTL_RCC2_OSCSRC2_M   0x00000070  // Oscillator Source 2
#define SYSCTL_RCC2_OSCSRC2_MO  0x00000000  // MOSC
/**
 * Initializes the UART for 115,200 baud rates.
 * 8-bit word length, no parity bits, one stop bit, FIFOs enabled.
 *
 * Assumes that the clock is 80 MHz
 */
void UartInit(void) {
    volatile unsigned long delay;
	


    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
    SYSCTL_RCGC2_R |= 0x03;               // activate port A and Port B
    delay = SYSCTL_RCGC2_R;               // allow time for clock to start
    UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
    // IBRD = int(80,000,000 / (16 * 9600)) = int(520.833333):
    UART1_IBRD_R = 520;
    // FBRD = round(0.83333 * 64) = 53:
    UART1_FBRD_R = 53;
    // 8 bit word length (no parity bits, one stop bit, FIFOs)
    UART1_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
    UART1_CTL_R |= UART_CTL_UARTEN; // enable UART
    GPIO_PORTB_AFSEL_R |= 0x03;     // enable alt funct on PB1, PB0
    GPIO_PORTB_DEN_R |= 0x03;       // enable digital I/O on PB1, PB0
    // configure PB1, PB0 as UART1
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) + 0x00000011;
    // disable analog functionality on PA1, PA0:
    GPIO_PORTB_AMSEL_R &= ~0x03;
	
}

/**
 * Waits for new serial port input.
 *
 * @return    ASCII code for key typed
 */
unsigned char UartInChar(void) {
    // wait until the Receiver FIFO Empty Flag is empty:
    while ((UART1_FR_R & UART_FR_RXFE) != 0) {
        ;
    }
    // then read from UART1 and mask with 8 bits:
    return (unsigned char)(UART1_DR_R & 0xFF);
}

/**
 * Gets oldest serial port input and returns immediately if there is no data.
 *
 * @return    ASCII code for key typed
 *       n    0 if there is no data
 */
unsigned char UartInCharNonBlock(void) {
    // if the Receiver FIFO Empty Flag is full:
    if ((UART1_FR_R & UART_FR_RXFE) == 0) {
        // get the data immediately and then exit:
        return (unsigned char)(UART1_DR_R & 0xFF);
    }
    // if the Receiver FIFO Empty Flag is empty:
    else {
        return 0; // get nothing and then exit
    }
}

/**
 * Outputs 8-bit data to serial port.
 *
 * @param  data  an 8-bit ASCII character to be transferred
 */
void UartOutChar(unsigned char *data) {
    // if the Transmitter FIFO Full Flag is not busy:
    while ((UART1_FR_R & UART_FR_TXFF) != 0) {
        ;
    }
    UART1_DR_R = data; // write the data
}

/**
 * Accepts ASCII input in unsigned decimal format
 * and converts to a 32-bit unsigned number
 * valid range is 0 to 4,294,967,295 (2^32-1).
 *
 * @return    32-bit unsigned number
 *
 * @notes     if you enter a number above 4294967295,
 *            it will return an incorrect value.
 *            Backspace will remove last digit typed
 */
unsigned long UartInNum(void) {
    unsigned long number = 0, length = 0;
    char character;

    character = UartInChar(); // get input from the UART

    while (character != CR) { // accept until <enter> is typed
        // The next line checks that the input is a digit, 0-9.
        // If the character is not 0-9, it is ignored and not echoed
        if ((character >= '0') && (character <= '9')) {
            // overflows if above 4,294,967,295
            number = 10 * number + (character - '0');
            length++;
            UartOutChar(character);
        }
        // If the input is a backspace, then the return number is
        // changed and a backspace is outputted to the screen
        else if ((character == BS) && length) {
            number /= 10;
            length--;
            UartOutChar(character);
        }
        character = UartInChar();
    }
    return number;
}

/**
 * Outputs out_str (NULL termination).
 *
 * @param  buffer[]  pointer to a NULL-terminated string to be transferred
 */
void UART_OutString(unsigned char buffer[]) {
    unsigned long i = 0;

    // go from the beginning to the ending of the character string:
    while (buffer[i] != '\0') {
        UartOutChar(buffer[i]); // write every single character
        i++;                    // move to next character
    }
}

unsigned char out_str[15];
unsigned short out_str_len;

/**
 * Puts number into global variable out_str[15]
 *
 * @param  n  32-bit unsigned number to be put in
 */
void PutIntoString(unsigned long n) {
    unsigned short i = 0;
    unsigned char reverse_str[15];
    short j = 0;

    // check the special case n = 0:
    if (!n) {
        out_str_len = 1;
        out_str[0] = 0x30;
        return;
    }

    // for other cases:
    while (n) {
        // store the number from right to left into reverse_str:
        reverse_str[j] = n % 10 + 0x30;
        n /= 10;     // cut off the most right digit
        j++;         // prepare for next digit
    }
    out_str_len = j; // set out_str_len

    // move back to out_str in right order:
    for (j = out_str_len - 1; j >= 0; j--) {
        out_str[i] = reverse_str[j];
        i++;
    }
}

/**
 * Converts a 32-bit unsigned number in unsigned decimal format.
 *
 * @param  n  32-bit unsigned number to be transferred
 *
 * @notes     Fixed format 4 digits, one space after, null termination
 *
 * @examples
 *   4 to "   4 "
 *   31 to "  31 "
 *   102 to " 102 "
 *   2210 to "2210 "
 *   10000 to "**** "  any value larger than 9999 converted to "**** "
 */
void UartNumToString(unsigned long n) {
	memset(out_str, sizeof(out_str), 0);
	      
	
    PutIntoString(n); // put n into out_str

    if (n <= 9) {     // in case n has only 1 digit:
        out_str[3] = out_str[0];
        out_str[0] = 0x20;
        out_str[1] = 0x20;
        out_str[2] = 0x20;
        out_str[4] = 0x20;
				out_str[5] = '\0';
				out_str[6] = '\0';
				out_str[7] = '\0';
    } else if ((n >= 10) && (n <= 99)) { // in case n has 2 digits:
        out_str[3] = out_str[1];
        out_str[2] = out_str[0];
        out_str[0] = 0x20;
        out_str[1] = 0x20;
        out_str[4] = 0x20;
				out_str[5] = '\0';
				out_str[6] = '\0';
				out_str[7] = '\0';
    } else if ((n >= 100) && (n <= 999)) { // in case n has 3 digits:
        out_str[3] = out_str[2];
        out_str[2] = out_str[1];
        out_str[1] = out_str[0];
        out_str[0] = 0x20;
        out_str[4] = 0x20;
				out_str[5] = '\0';
				out_str[6] = '\0';
				out_str[7] = '\0';
    } else if ((n >= 1000) && (n <= 9999)) { // in case n has 4 digits:
        out_str[4] = 0x20;
    } else {                                 // overflowing case:
        out_str[0] = '*';
        out_str[1] = '*';
        out_str[2] = '*';
        out_str[3] = '*';
        out_str[4] = 0x20;
        out_str[5] = '\0';
    }
}

/**
 * Outputs a 32-bit unsigned number in unsigned decimal format.
 *
 * @param  n  32-bit unsigned number to be transferred
 *
 * @notes     Fixed format 4 digits, one space after, null termination
 */
void UartOutNum(unsigned long n) {
    UartNumToString(n);     // convert to decimal
    UartOutString(out_str); // output
}

/**
 * Converts a 32-bit distance into an ASCII string.
 *
 * @param  n  32-bit unsigned number to be converted (resolution 0.001cm)
 *
 * @notes     Fixed format
 *            1 digit, point, 3 digits, space, units, null termination
 *
 * @examples
 *   4 to "0.004 cm"
 *   31 to "0.031 cm"
 *   102 to "0.102 cm"
 *   2210 to "2.210 cm"
 *   10000 to "*.*** cm"  any value larger than 9999 converted to "*.*** cm"
 */
void UartDistanceToString(unsigned long n) {
		memset(out_str, sizeof(out_str), 0);
    PutIntoString(n); // put n into out_str

    if (n <= 9) {     // in case n has only 1 digit:
        out_str[4] = out_str[0];
        out_str[0] = 0x30;
        out_str[1] = '.';
        out_str[2] = 0x30;
        out_str[3] = 0x30;
        out_str[5] = 0x20;
        out_str[6] = 'c';
        out_str[7] = 'm';
				out_str[8] = '\0';
				out_str[9] = '\0';
    } else if ((n >= 10) && (n <= 99)) { // in case n has 2 digits:
        out_str[4] = out_str[1];
        out_str[3] = out_str[0];
        out_str[0] = 0x30;
        out_str[1] = '.';
        out_str[2] = 0x30;
        out_str[5] = 0x20;
        out_str[6] = 'c';
        out_str[7] = 'm';
				out_str[8] = '\0';
				out_str[9] = '\0';
			
    } else if ((n >= 100) && (n <= 999)) { // in case n has 3 digits:
        out_str[4] = out_str[2];
        out_str[3] = out_str[1];
        out_str[2] = out_str[0];
        out_str[0] = 0x30;
        out_str[1] = '.';
        out_str[5] = 0x20;
        out_str[6] = 'c';
        out_str[7] = 'm';
				out_str[8] = '\0';
				out_str[9] = '\0';
    } else if ((n >= 1000) && (n <= 9999)) { // in case n has 4 digits:
        out_str[4] = out_str[3];
        out_str[3] = out_str[2];
        out_str[2] = out_str[1];
        out_str[1] = '.';
        out_str[5] = 0x20;
        out_str[6] = 'c';
        out_str[7] = 'm';
				out_str[8] = '\0';
				out_str[9] = '\0';
    } else { // overflowing case:
        out_str[0] = '*';
        out_str[1] = '.';
        out_str[2] = '*';
        out_str[3] = '*';
        out_str[4] = '*';
        out_str[5] = 0x20;
        out_str[6] = 'c';
        out_str[7] = 'm';
				out_str[8] = '\0';
				out_str[9] = '\0';
    }
}

/**
 * Outputs a 32-bit number in unsigned decimal fixed-point format.
 *
 * @param  n  32-bit unsigned number to be transferred (resolution 0.001cm)
 *
 * @notes     Fixed format
 *            1 digit, point, 3 digits, space, units, null termination
 */
void UartOutDistance(unsigned long n) {
    UartDistanceToString(n); // convert to distance
    UartOutString(out_str);  // output
}

//--------------------------------------------------------------------------------------------------------------------------

//JaredCODE
// UART.c
// Runs on LM3S811, LM3S1968, LM3S8962, LM4F120, TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// September 11, 2013
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Program 4.12, Section 4.9.4, Figures 4.26 and 4.40

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

// U1Rx (VCP receive) connected to PA0
// U1Tx (VCP transmit) connected to PA1

//Using Uart 1 because we are Port B, otherwise switch to port A and use different TX/RX ports

#include "tm4c123gh6pm.h"
#include "uart.h"
#include "string.h"
#define SYSCTL_RCC2_USERCC2     0x80000000  // Use RCC2
#define SYSCTL_RCC2_DIV400      0x40000000  // Divide PLL as 400 MHz vs. 200
                                            // MHz
#define SYSCTL_RCC2_SYSDIV2_M   0x1F800000  // System Clock Divisor 2
#define SYSCTL_RCC2_SYSDIV2LSB  0x00400000  // Additional LSB for SYSDIV2
#define SYSCTL_RCC2_PWRDN2      0x00002000  // Power-Down PLL 2
#define SYSCTL_RCC2_BYPASS2     0x00000800  // PLL Bypass 2
#define SYSCTL_RCC2_OSCSRC2_M   0x00000070  // Oscillator Source 2
#define SYSCTL_RCC2_OSCSRC2_MO  0x00000000  // MOSC
/**
 * Initializes the UART for 115,200 baud rates.
 * 8-bit word length, no parity bits, one stop bit, FIFOs enabled.
 *
 * Assumes that the clock is 80 MHz
 */
void UART_Init(void) {
    volatile unsigned long delay;
 
    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
    SYSCTL_RCGC2_R |= 0x03;               // activate port A and Port B
    delay = SYSCTL_RCGC2_R;               // allow time for clock to start
    UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
    // IBRD = int(80,000,000 / (16 * 9600)) = int(520.833333):
    UART1_IBRD_R = 520;
    // FBRD = round(0.83333 * 64) = 53:
    UART1_FBRD_R = 53;
    // 8 bit word length (no parity bits, one stop bit, FIFOs)
    UART1_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
    UART1_CTL_R |= UART_CTL_UARTEN; // enable UART
    GPIO_PORTB_AFSEL_R |= 0x03;     // enable alt funct on PB1, PB0
    GPIO_PORTB_DEN_R |= 0x03;       // enable digital I/O on PB1, PB0
    // configure PB1, PB0 as UART1
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) + 0x00000011;
    // disable analog functionality on PB1, PB0:
    GPIO_PORTB_AMSEL_R &= ~0x03;
 
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
unsigned char UART_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0);
  return((unsigned char)(UART1_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(unsigned char data){
  while((UART1_FR_R&UART_FR_TXFF) != 0);
  UART1_DR_R = data;
}

//Part B---

//------------UART_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
unsigned long UART_InUDec(void){
unsigned long number=0, length=0;
char character;
  character = UART_InChar();
  while(character != CR){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(unsigned long n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

//Part A---

//------------UART_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is inputted.
// If a backspace is inputted, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART_InString(char *bufPt, unsigned short max) {
int length=0;
char character;
  character = UART_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        UART_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  *bufPt = 0;
}

//------------UART_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}


//------------------------------------------------------------------------


//---------------------UART_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 32-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 8 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFFFFFF
// If you enter a number above FFFFFFFF, it will return an incorrect value
// Backspace will remove last digit typed
unsigned long UART_InUHex(void){
unsigned long number=0, digit, length=0;
char character;
  character = UART_InChar();
  while(character != CR){
    digit = 0x10; // assume badx
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit <= 0xF){
      number = number*0x10+digit;
      length++;
      UART_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS) && length){
      number /= 0x10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

//--------------------------UART_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void UART_OutUHex(unsigned long number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    UART_OutUHex(number/0x10);
    UART_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      UART_OutChar(number+'0');
     }
    else{
      UART_OutChar((number-0x0A)+'A');
    }
  }
}


//Part C---

////--------------------------UART_OutDist----------------------------

void UART_OutDist(unsigned long number){
// This function converts a number into a distance and prints it out.
	int decimal = 0;
	int floatingPoint = 0;
	
  if(number > 9999)
	{
		//print "****"
	}
	else if (number == 0)
	{
		//print "   0 "
	}
	else
	{
		//calculate number x (0.001)
		//decimal = floor(float(number) * 0.001);
		//print whole decimal part of num
		//UART_OutUDec()
		//print '.'
		//print floating point part of num
		//UART_OutUDec()
		//print ' '
		//print 'c' 'm'
	}
	
	if(number >= 0x10){
    UART_OutUHex(number/0x10);
    UART_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      UART_OutChar(number+'0');
     }
    else{
      UART_OutChar((number-0x0A)+'A');
    }
  }
}
