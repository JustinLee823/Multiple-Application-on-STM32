/*
 * systick.h
 *
 *  Created on: Sep 23, 2025
 *      Author: justinlee
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "stm32l5xx.h"

typedef unsigned int Time_t;
#define TIME_MAX (Time_t)(-1);

void StartSysTick(); //Initializes the system timer and enables the interrupt
void WaitForSysTick(); //Puts the CPU to sleep and waits for the next interrupt
void msDelay(int t); //Uses the SysTick to accurately measure delay in milliseconds. This is a replacement
		//for the counting loop delay() function used in Lab 0. The function blocks until the time is elapsed

Time_t TimeNow(); //Returns the current system time
Time_t TimePassed(); //Calculates the elapsed time since a previous system time.
				//Can be used for nonblocking days, measuring time between events, and many more things


#endif /* SYSTICK_H_ */
