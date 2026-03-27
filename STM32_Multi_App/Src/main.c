// System headers
#include <stdio.h>
#include "systick.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"
#include "display.h"
#include "touchpad.h"
#include "motor.h"
#include "enviro.h"
// App headers
#include "alarm.h"
#include "game.h"
#include "calc.h"
int main (void){
	// Initialize apps
	Init_Alarm();
	Init_Game();
	Init_Calc();
	Init_Enviro();
	Init_Motor();

	// Enable system services
	StartSysTick();
	while (1) {
		// Run apps
		Task_Alarm();
		Task_Game();
		Task_Calc();
		Task_Enviro();
		Task_Motor();

		// Housekeeping
		UpdateIOExpanders();
		UpdateDisplay();
		ScanTouchpad();
		ServiceI2CRequests();
		ServiceSPIRequests();
		WaitForSysTick();
	}
}
