/**
 * @file     main.c
 * @author   Phi Luu
 * @date     April 07, 2016
 *
 * @brief    UTAustinX: UT.6.03x Embedded Systems - Shape the World
 *           Lab 11: UART - Serial Interface
 *
 * @section  DESCRIPTION
 *
 * Converts numbers into ASCII strings and display on the serial monitor.
 */

#include "uart.h" // include all UART related functions


//#include "TExaS.h"

// Function prototypes
void EnableInterrupts(void);

int main(void) {
    // Setup
    unsigned long input_num;
		PLL_Init();
    UartInit();        // initialize UART
  //  UartOutString("Running Project 4 by Mahdi Pedram");

    // Loop
    while (1) {
        UartOutString("\n\rInput:");
        input_num = UartInNum();   // read n
        UartOutString(" UART_OutUDec = ");
        UartOutNum(input_num);     // write n in number format
        UartOutString(",  UART_OutDistance ~ ");
        UartOutDistance(input_num); // write n in distance format
    }
}
