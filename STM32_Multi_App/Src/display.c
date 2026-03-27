/*
 * display.c
 *
 *  Created on: Oct 22, 2025
 *      Author: justinlee
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "display.h"
#include "i2c.h"
#include "systick.h"
#include "touchpad.h"

bool enabled = false;
Page_t openPage = 0;

static const Pin_t TouchEn = {GPIOB, 5};
Time_t pressTime;
#define DEBOUNCE_TIME 50
static void CallbackTouchEnPress(void);
static void CallbackTouchEnRelease(void);

//Display controller
#define ROWS 2 //Number of rows
#define COLS 16 //Number of columns

uint8_t dispText[PAGES][ROWS][COLS+1];

typedef struct{
	uint8_t ctrl; //Control byte
	uint8_t data; //Data byte
} DispCmd_t;

typedef struct{
	DispCmd_t cnd; //Select display line
	uint8_t ctrl; //Last control word, data bytes to follow
	uint8_t text[COLS]; //ASCII text to write to display
	//uint8_t term; //NUL Terminator
} DispLine_t;

//initialization command sequence
static const DispCmd_t txInit[] = {
		{0x80,0x28}, //Function set 2-line, display off
		{0x80,0x0C}, //Display control: display ON, cursor OFF, blink OFF
		{0x80,0x01}, //Display clear
		{0x00,0x06}}; //Entry mode set: increment, no shift

//Select display line and print text
static DispLine_t txLine[ROWS] = {
		{{0x80, 0x80}, 0x40, {0}},
		{{0x80, 0xC0}, 0x40, {0}}};
static bool updateLine[2] = {false,false};

//I2C transfer
static I2C_Xfer_t DispInit = {&LeafyI2C, 0x7C, (uint8_t *)&txInit, 8, 1};
static I2C_Xfer_t DispLine[ROWS] =  {
		 {&LeafyI2C, 0x7C, (uint8_t *)&txLine[0], 19, 1},
		 {&LeafyI2C, 0x7C, (uint8_t *)&txLine[1], 19, 1} };

//enable LCD display
void DisplayEnable(void){

	if(!enabled){
		enabled = true;
		I2C_Enable(LeafyI2C);
		I2C_Request(&DispInit);

		//Blank out display
		for(int i = 0; i < PAGES; i++){
			for(int j = 0; j < ROWS; j++){
				for(int k = 0; k < COLS; k++){
					dispText[i][j][k] = ' ';
				}
			}
		}

		GPIO_Enable(TouchEn);
		GPIO_Mode(TouchEn, INPUT);
		GPIO_Callback(TouchEn, CallbackTouchEnPress, RISE);
		GPIO_Callback(TouchEn, CallbackTouchEnRelease, FALL);
	}


}

void DisplayPrint(Page_t page, const int line, const char *msg, ...){
	va_list args;
	va_start(args, msg);

	//Full buffer with formatted text and space pad the remainder
	int chars = vsnprintf((char *)dispText[page][line], COLS+1, msg, args);
	for(int i = chars; i < COLS; i++){
		dispText[page][line][i] = ' ';
	}

	if(page == openPage){
		updateLine[line] = true;
	}

}

Colour_t dispColour[PAGES] = {OFF, CYAN, MAGENTA, ORANGE, RED};

//Back light controller
typedef struct{
	uint8_t addr; //address byte
	uint8_t data; //data byte
}BltCmd_t;

//Transmit data buffers to set brightness of each LED, all off by default
static BltCmd_t txRed = {0x01, 0x00};
static BltCmd_t txGreen = {0x02, 0x00};
static BltCmd_t txBlue = {0x03, 0x00};

static bool updateBlt = true;

//I2C transfers
static I2C_Xfer_t BltRed = {&LeafyI2C, 0x5A, (void *)&txRed, 2, 1};
static I2C_Xfer_t BltGreen = {&LeafyI2C, 0x5A, (void *)&txGreen, 2, 1};
static I2C_Xfer_t BltBlue = {&LeafyI2C, 0x5A, (void *)&txBlue, 2, 1};

// Set new backlight colour
void DisplayColour(const Page_t page, Colour_t colour) {
	dispColour[page] = colour;
	if(page == openPage){
		updateBlt = true;
	}
}

//Auto background updates
//called from main
void UpdateDisplay(void){
	for(int i = 0; i < ROWS; i++){
		if(!DispLine[i].busy && updateLine[i]){
			updateLine[i] = false;

			for(int j = 0; j < COLS; j++){
				txLine[i].text[j] = dispText[openPage][i][j];
			}
			I2C_Request(&DispLine[i]);
		}
	}

	if(!BltBlue.busy && updateBlt){
		updateBlt = false;

		Colour_t colour = dispColour[openPage];
		txRed.data = (colour >> 16) & 0xFF;
		txGreen.data = (colour >> 8) & 0xFF;
		txBlue.data = (colour >> 0) & 0xFF;

		I2C_Request(&BltRed);
		I2C_Request(&BltGreen);
		I2C_Request(&BltBlue);
	}
}

Page_t GetPage(void){
	return openPage;
}

static void CallbackTouchEnPress(void){
	pressTime = TimeNow();
}

static void CallbackTouchEnRelease(void){
	Time_t heldTime = TimePassed(pressTime);

	if(heldTime > DEBOUNCE_TIME){
		openPage++;
		openPage %= PAGES;

		for(int i = 0; i<ROWS; i++){
			updateLine[i] = true;
		}
		updateBlt = true;
		ClearTouchpad();
	}
}
