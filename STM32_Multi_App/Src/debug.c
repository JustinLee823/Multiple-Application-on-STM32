/*
 * debug.c
 *
 *  Created on: Sep 23, 2025
 *      Author: justinlee
 */

#include "stm32l5xx.h"


//To redirect the standard output (stdout) from functions like printf() to
//the communication channel of our choice, all we need to do is implement the __io_putchar() function
int __io_putchar(int c){
	ITM_SendChar(c);
	return c;
}
