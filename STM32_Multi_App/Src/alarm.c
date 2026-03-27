/*
 * alarm.c
 *
 *  Created on: Sep 30, 2025
 *      Author: justinlee
 */


// Alarm system app
#include <stdio.h>
#include "display.h"
#include "alarm.h"
#include "systick.h"
#include "gpio.h"
// GPIO pins
static const Pin_t RedLED = {GPIOA, 9}; // This is the Red LED which the brightness is measured in Lumens
static const Pin_t BlueLED = {GPIOB, 7};// This is the Blue LED which the brightness is measured in Lumens
static const Pin_t GreenLED = {GPIOC, 7};// This is the Green LED which the brightness is measured in Lumens

static const Pin_t Button = {GPIOC, 13};// This is a button which can control the state of the alarm
static const Pin_t Sensor = {GPIOB, 9}; // This is the motion sensor which measures in motion units
static const Pin_t Buzzer = {GPIOA, 0};


static enum {DISARMED, ARMED, TRIGGERED} state;
static enum {LONG, SHORT, NONE} pressLength;
// Constants


// Variables
static Time_t whenPressed; //used to record the time when the button is pressed
static Time_t LEDSwitchDelay; //used to delay the alternating blue and green LED
static Time_t buzzerDelay; //used to delay the buzzer sound on an interval
static int isTriggered;
static int blueLEDActive; //flag used to see if the blue LED is on or off
static int buzzerActive; //flag used to see if the buzzer is on or off
#define LONG_PRESS 3000
#define DEBOUNCE 50
#define ALTERNATE_LED 1000 //every one second, blue and green LEDs alternate
#define BUZZER_INTERVAL 500 // the buzzer will make a sound


// Declare interrupt callback functions
static void CallbackMotionDetect();
static void CallbackButtonPress();
static void CallbackButtonRelease();

// Initialization code
void Init_Alarm (void) {
	state = DISARMED;

	//initializing flags
	isTriggered = 0;
	pressLength = NONE;
	blueLEDActive = 0;
	buzzerActive = 0;

	LEDSwitchDelay = 0;
	buzzerDelay = 0;

	//Initializing Blue LED
	GPIO_Enable(BlueLED);
	GPIO_Mode(BlueLED, OUTPUT);
	GPIO_Output(BlueLED, LOW);

	//Initializing Red LED
	GPIO_Enable(RedLED);
	GPIO_Mode(RedLED, OUTPUT);
	GPIO_Output(RedLED, LOW);

	//Initializing Green LED
	GPIO_Enable(GreenLED);
	GPIO_Mode(GreenLED, OUTPUT);
	GPIO_Output(GreenLED, LOW);

	//Initializing Button
	GPIO_Enable(Button);
	GPIO_Mode(Button, INPUT);

	GPIO_Enable(Sensor);
	GPIO_Mode(Sensor, INPUT);

	GPIO_Enable(Buzzer);
	GPIO_Mode(Buzzer, OUTPUT);

	DisplayEnable();
	DisplayColour(ALARM, WHITE);
	DisplayPrint(ALARM, 0, "DISARMED");

	GPIO_Callback(Sensor, CallbackMotionDetect, RISE);
	GPIO_Callback(Button, CallbackButtonPress, RISE); // Button Down
	GPIO_Callback(Button, CallbackButtonRelease, FALL); // Button Up
}

// Task code (state machine)
void Task_Alarm (void) {
	switch (state) {
		case DISARMED:
			GPIO_Output(RedLED, LOW);
			GPIO_Output(BlueLED, LOW);
			GPIO_Output(GreenLED, LOW);
			GPIO_Output(Buzzer, LOW);
			isTriggered = 0;

			DisplayColour(ALARM, WHITE);
			DisplayPrint(ALARM, 0, "DISARMED");

			if (pressLength == SHORT) {
				state = ARMED;
				LEDSwitchDelay = TimeNow();
				pressLength = NONE;
				GPIO_Output(BlueLED, HIGH);
				printf("... at time %u", TimeNow());
			}

			break;
		case ARMED:
			GPIO_Output(RedLED, LOW);
			DisplayColour(ALARM, YELLOW);
			DisplayPrint(ALARM, 0, "ARMED");

			if(blueLEDActive & (TimePassed(LEDSwitchDelay) >= ALTERNATE_LED)){
				GPIO_Output(GreenLED, HIGH);
				GPIO_Output(BlueLED, LOW);
				blueLEDActive = 0;
				LEDSwitchDelay = 0;
				LEDSwitchDelay = TimeNow();
			}
			else if(!blueLEDActive & (TimePassed(LEDSwitchDelay) >= ALTERNATE_LED)){
				GPIO_Output(GreenLED, LOW);
				GPIO_Output(BlueLED, HIGH);
				blueLEDActive = 1;
				LEDSwitchDelay = 0;
				LEDSwitchDelay = TimeNow();
			}


			if(isTriggered){
				state = TRIGGERED;
				pressLength = NONE;
				buzzerDelay = TimeNow();
			}

			if(pressLength == LONG){
				state = DISARMED;
				pressLength = NONE;
			}
			break;
		case TRIGGERED:
			GPIO_Output(RedLED, HIGH);
			GPIO_Output(BlueLED, LOW);
			GPIO_Output(GreenLED, LOW);
			DisplayColour(ALARM, RED);
			DisplayPrint(ALARM, 0, "TRIGGERED");


			if(!buzzerActive & (TimePassed(buzzerDelay) >= BUZZER_INTERVAL)){
				//GPIO_Output(Buzzer, HIGH);
				buzzerActive = 1;
				buzzerDelay = 0;
				buzzerDelay = TimeNow();
			}
			else if(buzzerActive & (TimePassed(buzzerDelay) >= BUZZER_INTERVAL)){
				GPIO_Output(Buzzer, LOW);
				buzzerActive = 0;
				buzzerDelay = 0;
				buzzerDelay = TimeNow();
			}

			if(pressLength == LONG){
				state = DISARMED;
				pressLength = NONE;
				GPIO_Output(Buzzer, LOW);
			}

			if(pressLength == SHORT){
				state = ARMED;
				pressLength = NONE;
				isTriggered = 0;
				GPIO_Output(Buzzer, LOW);
			}
			break;
	}
}

void CallbackMotionDetect (void) {
	isTriggered = 1;
}

void CallbackButtonPress (void) {
	whenPressed = TimeNow();
}

void CallbackButtonRelease (void) {
	if((TimePassed(whenPressed) >= LONG_PRESS)){
		pressLength = LONG;
	}
	else{
		pressLength = SHORT;
	}
	whenPressed = 0;
}
