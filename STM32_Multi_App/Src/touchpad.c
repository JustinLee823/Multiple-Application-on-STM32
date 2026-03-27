/*
 * touchpad.c
 *
 *  Created on: Nov 5, 2025
 *      Author: justinlee
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "touchpad.h"
#include "systick.h"
#include "i2c.h"
#include "display.h"
#include "gpio.h"

static bool enabled = false;

// I2C write transfer to initialize Touchpad sensor
// Electrode Configuration Register (ECR), all 12 electrodes enabled
static uint8_t txInit[2] = {0x5E, 0x0C};// Register Adddres, Write Data
static I2C_Xfer_t PadInit = {&LeafyI2C, 0xB4, txInit, 2, 1, 0, NULL};

// I2C combined write-read transfer to read Touchpad sensor
// Touch Status Registers (lower and upper)
static uint8_t txRdAddr[1] = {0x00};// Register Address (lower)
static uint8_t rxRdData[2];// Read Data (2 bytes)
static I2C_Xfer_t PadRdAddr = {&LeafyI2C, 0xB4, txRdAddr, 1, 0, 0, NULL};
static I2C_Xfer_t PadRdData = {&LeafyI2C, 0xB5, rxRdData, 2, 1, 0, NULL};

void TouchEnable(void){
	if(!enabled){
		enabled = true;

		I2C_Enable(LeafyI2C);
		I2C_Request(&PadInit);

		//Request first read
		I2C_Request(&PadRdAddr);
		I2C_Request(&PadRdData);
	}
}

static uint16_t touchData;
static bool touchCapture = false;
static Press_t lastPress;

Press_t TouchInput(Page_t page){
	if(page != GetPage()){
		return NONE; // Hide input for inactive pages
	}

	Press_t pad = lastPress;
	lastPress = NONE;
	return pad;
}

bool TouchEntry(Page_t page, Entry_t *num){
	if(page != GetPage()){
		return false;
	}
	bool done = false;

	if(lastPress >= N0 && lastPress <= N9){
		*num = *num * 10 + lastPress;
	}
	else if(lastPress == SHIFT){
		*num /= 10;
	}
	else if(lastPress == NEXT){
		done = true;
	}

	lastPress = NONE;
	return done;
}

void ScanTouchpad(void){
	if(!PadRdData.busy){
		touchData = rxRdData[0] | rxRdData[1] << 8;

		if(!touchCapture && touchData != 0x0000){
			for(Press_t n = MIN; n <= MAX; n++){
				if(touchData == 1 << n){
					touchCapture = true;
					lastPress = n;
					break;
				}
			}
		}
		else if(touchData == 0x0000){
			touchCapture = false;
		}

		I2C_Request(&PadRdAddr);
		I2C_Request(&PadRdData);
	}
}

void ClearTouchpad(void){
	lastPress = NONE;
}
