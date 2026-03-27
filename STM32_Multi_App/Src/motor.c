/*
 * motor.c
 *
 *  Created on: Nov 25, 2025
 *      Author: justinlee
 */

#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "motor.h"
#include "display.h"
#include "touchpad.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "sysclk.h"

// GPIO pins
// Refer to Lab User’s Guide
static const Pin_t AI1 = {GPIOB, 10}; // Pin P??? -> Motor driver
static const Pin_t AI2 = {GPIOB, 11}; // Pin P??? -> Motor driver
static const Pin_t STBY = {GPIOE, 15}; // Pin P??? -> Motor driver
static const Pin_t EncA = {GPIOB, 0}; // Pin P??? <- Rotary encoder A
static const Pin_t EncB = {GPIOB, 1}; // Pin P??? <- Rotary encoder B

// Analog-to-digital converter for potentiometer
// Refer to Lab User’s Guide
static const ADCInput_t Pot = {ADC1, 1, {GPIOC, 0}}; // ADC1 Chan1 <- P???

// Timer channels
// Refer to Lab User’s Guide
static const TimerIO_t Motor = {TIM1, 1, {GPIOE, 9}, 1}; // Timer? Chan? -> P??? AF?

//System Clock frequency in Hz
#define SYSCLK_FREQ 48e6

// Timer period
#define PWM_PSC (100 - 1) // Pre-scaler
#define PWM_ARR (240 - 1) // Auto-reload
#define PWM_RCR ( 50 - 1) // Repetition count
#define MAX_SPEED 350.0
static enum { CW=0, CCW=1 } direction = CCW; // Clockwise or counter-clockwise
static enum { OL=0, CL=1, CLT=2 } loopMode = OL; // Open/closed loop (w/ tuning)
static float rpmScalingFactor;
static float desiredRPM = 0;
static float errorRpm;
static float error = 0;
static float errorRpmSum = 0;
static float errorSum = 0;

static float fUP = 1.0; //timer Update Frequency
static const float PPR = 748.0; //Pulses Per Revolution, each encoder is 374, *2 because we're counting pulses from two encoders

// PI controller parameters
// Note: Update defaults for Np and Ni after tuning
// Kp = Np * dp
int Np = 6;
float dp = 0.1;

// Ki = Ni * di
int Ni = 8;
float di = 0.001;

//Kp = 0.6 Ki = 0.008

uint32_t pulsesA = 0;
uint32_t pulsesB = 0;
uint32_t timerCount = 0;
uint32_t totalPulses = 0;
uint32_t prevTimerCount = 0;

// Interrupt callback functions
static void CallbackMotor(void);
static void CallbackEncA(void);
static void CallbackEncB(void);

// Change motor direction
// Refer to motor driver datasheet
void MotorDirection(int dir) {
	if (dir == CW) {
		// Clockwise
		GPIO_Output(AI1, LOW);
		GPIO_Output(AI2, HIGH);
	}
	else {
		// Counter-clockwise
		GPIO_Output(AI1, HIGH);
		GPIO_Output(AI2, LOW);
	}
}

// Initialize app
void Init_Motor (void) {
	#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
		SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));
	#endif

	//GPIO Enable
	GPIO_Enable(AI1);
	GPIO_Enable(AI2);
	GPIO_Enable(STBY);
	GPIO_Enable(EncA);
	GPIO_Enable(EncB);
	ADC_Enable(Pot); // Speed control potentiometer
	// Configure GPIO pins
	// Refer to motor controller datasheet for outputs in stop mode
	// Register callbacks for rotary encoder inputs

	//Set GPIO Mode
	GPIO_Mode(AI1, OUTPUT);
	GPIO_Mode(AI2, OUTPUT);
	GPIO_Mode(STBY, OUTPUT);
	GPIO_Mode(EncA, INPUT);
	GPIO_Mode(EncB, INPUT);

	//Set GPIO Output
	GPIO_Output(AI1, LOW);
	GPIO_Output(AI2, LOW);
	GPIO_Output(STBY, HIGH);

	//Set Motor
	TimerEnable(Motor);
	TimerPeriod(Motor, PWM_PSC, PWM_ARR, PWM_RCR);
	TimerMode(Motor, OUTCMP, PWM1);
	TimerCallback(Motor, CallbackMotor, UP);
	TimerStart(Motor, OUTCMP);
	MotorDirection(direction);

	//Set Interrupts
	GPIO_Callback(EncA, CallbackEncA, RISE);
	GPIO_Callback(EncB, CallbackEncB, RISE);

	fUP = (((SYSCLK_FREQ / PWM_PSC) / PWM_ARR) / PWM_RCR);
	rpmScalingFactor = (fUP/PPR) * 60.0; //(fUP/PPR) * 60s/1min
}

// Execute app
void Task_Motor (void) {
	float motorDrivePercentage = (float)ADC_Read(Pot) / 4096.0;
	desiredRPM = motorDrivePercentage * MAX_SPEED;
	if (timerCount-prevTimerCount > 0) {
		float measuredRPM = (float)totalPulses*rpmScalingFactor;

		if (loopMode == OL) {
			TimerOutput(Motor, (uint16_t) (motorDrivePercentage * (float)(PWM_ARR + 1)));
			// Display status for Open Loop mode
			DisplayPrint(MOTOR, 0, " OL %3d %%", (int)(motorDrivePercentage*100));
			DisplayPrint(MOTOR, 1, "%cCW %3d RPM", direction == CCW ? 'C' : ' ', (int)measuredRPM);
		}
		else {
			// Closed Loop control
			float Kp = (float)Np * dp;
			float Ki = (float)Ni * di;
			errorRpm = desiredRPM - measuredRPM;
			errorRpmSum += errorRpm;
			// Convert errorRpm to change on TIM1->CCR1
			error = errorRpm / MAX_SPEED * (float)(PWM_ARR + 1);
			errorSum = errorRpmSum / MAX_SPEED * (float)(PWM_ARR + 1);
			TimerOutput(Motor,round((Kp*error + Ki*errorSum)));
			if (loopMode == CLT) {
				// Display status for Closed Loop mode /w tuning enabled
				DisplayPrint(MOTOR, 0, "Kp:%.1f  %d RPM", Kp, (int)desiredRPM);
				DisplayPrint(MOTOR, 1, "Ki:%.3f  %d RPM", Ki, (int)measuredRPM);
			}
			else {
				// Display status for normal Closed Loop mode
				DisplayPrint(MOTOR, 0, "CL %d RPM", (int)desiredRPM);
				DisplayPrint(MOTOR, 1, "%cCW %d RPM", direction == CCW ? 'C' : ' ', (int)measuredRPM);
			}
		}
		totalPulses = 0;
		prevTimerCount = timerCount;
	}

	// Touchpad controls
	switch (TouchInput(MOTOR)) {
 	 // Motor direction: update and apply
	case 0: //Set Kp and Ki to zero
		if(loopMode == CLT){
			Np = 0;
			Ni = 0;
		}
		break;
	case 1: //CW
		direction = CW;
		MotorDirection(direction);
		break;
	case 2: //Increase Kp
		if(loopMode == CLT){
			Np++;
		}
		break;
	case 3: //Increase Ki
		if(loopMode == CLT){
			Ni++;
		}
		break;
	case 4: //CCW
		direction = CCW;
		MotorDirection(direction);
		break;
	case 5: //Decrease Kp
		if(loopMode == CLT && Np > 0.0){
			Np--;
		}
		break;
	case 6: //Decrease Ki
		if(loopMode == CLT && Ni > 0.0){
			Ni--;
		}
		break;
	case 7: //OL (open loop)
		loopMode = OL;
		break;
	case 8: //CL (Closed Loop)
		loopMode = CL;
		break;
	case 9: //CLT (Closed Loop with Tuning)
		loopMode = CLT;
		break;
 	 default: break;
 	 }
}

// Timer 1 update
void CallbackMotor (void) {
	timerCount++;
 	totalPulses = pulsesA+pulsesB;
 	pulsesA = 0;
 	pulsesB = 0;
}

// Rotary encoder A rising edge
void CallbackEncA (void) {
	pulsesA++;
}

// Rotary encoder B rising edge
void CallbackEncB (void) {
	pulsesB++;
}
