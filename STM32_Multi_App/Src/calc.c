/*
 * calc.c
 *
 *  Created on: Nov 5, 2025
 *      Author: justinlee
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "calc.h"
#include "display.h"
#include "touchpad.h"


#define NMAX 10

static Press_t operation;
static Press_t returnMenu;
static Entry_t operand[NMAX];
static bool done;
static int count;
static int scroll;
static int array_scroll;
static int myArray[NMAX - 1];
static Entry_t result;

static enum {MENU, PROMPT, ENTRY, RUN, SHOW, WAIT} state;

uint32_t Increment(uint32_t n);
uint32_t Decrement(uint32_t n);
uint32_t FourFunction(int op, uint32_t n1, uint32_t n2);
uint32_t GCD(uint32_t n1, uint32_t n2);
uint32_t Factorial(uint32_t n);
uint32_t Fibonacci(uint32_t n);
void Sort(uint32_t* n, uint32_t count);
uint32_t Average(Entry_t *n, int count);


void Init_Calc(void){
	DisplayEnable();
	TouchEnable();
}

void Task_Calc(void){
	switch(state){

	case MENU:
		//Prepare for a new operation
		operation = NONE;
		for (int i = 0; i < NMAX; i++){
			operand[i] = 0;
		}
		count = 0;
		scroll = 0;
		array_scroll = 1;
		result = 0;

		//Display Menu
		DisplayPrint(CALC, 0, "Calculator app");
		DisplayPrint(CALC, 1, "1:INC 2:DEC");
		state = PROMPT;
		break;

	case PROMPT:
		if(operation ==  NONE){
			operation = TouchInput(CALC);
		}

		switch((int)operation){
		case 1: //Increment
		case 2: //Decrement
			if(count == 1){
				state = RUN;
			}
			else{
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;
		case 3: //4 Function Arith
			if(count <= 0){
				DisplayPrint(CALC, 0, "Select Operation");

				state = ENTRY;
				break;
			}
			else if(count == 3){
				state = RUN;
				break;
			}
			else{
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
				break;
			}
		case 4://GCD Function
			if(count < 2){
				DisplayPrint(CALC, 0, "Enter a number");
				state = ENTRY;
				break;
			}
			else{
				state = RUN;
				break;
			}
		case 5:
		case 6: //Fibonacci
			if(count == 1){
				state = RUN;
			}
			else{
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;
		case 7:
		case 8://Average function
			if(count == 0){
				DisplayPrint(CALC, 0, "How many items?");
				state = ENTRY;
			}
			else if(count == operand[0] + 1){
				state = RUN;
			}
			else{ //operand[0] is the number of items
				DisplayPrint(CALC, 0, "Enter a number:");
				state = ENTRY;
			}
			break;
		case 11:
			if(scroll == 0){
				DisplayPrint(CALC, 1, "3:FourFunc 4:GCD");
				scroll = 1;
			}
			else if(scroll == 1){
				DisplayPrint(CALC, 1, "5:That->! 6:Fib");
				scroll = 2;
			}
			else if(scroll == 2){
				DisplayPrint(CALC, 1, "7:Sort 8:Average");
				scroll = 3;
			}
			else{
				DisplayPrint(CALC, 1, "1:INC 2:DEC");
				scroll = 0;
			}
			operation = NONE;
			break;

		default:
			break;
		}
		break;

	case ENTRY:
		done = TouchEntry(CALC, &operand[count]);
		if(operation == 3){
			DisplayPrint(CALC, 1, "1:Add,2:Sub,3:Mult,4:Div");
		}
		else{
			DisplayPrint(CALC, 1, "%u", operand[count]);
		}

		if(done){
			count++;
			state = PROMPT;
		}
		break;
	case RUN:
		DisplayPrint(CALC, 0, "Calculating...");
		state = SHOW;

		switch((int)operation){
		case 1: result = Increment(operand[0]); break;
		case 2: result = Decrement(operand[0]); break;
		case 3: result = FourFunction(operand[0], operand[1], operand[2]); break;
		case 4: result = GCD(operand[0], operand[1]); break;
		case 5: result = Factorial(operand[0]); break;
		case 6: result = Fibonacci(operand[0]); break;
		case 7: Sort(&operand[1], operand[0] - 1);
			result = operand[1]; break;
		case 8: result = Average(operand, operand[0] - 1); break;
		default: break;
		}

		break;

	case SHOW:

		if(operation != 7){
			DisplayPrint(CALC, 0 , "Result:");
			DisplayPrint(CALC, 1, "%u", result);
		}
		else{
			DisplayPrint(CALC, 0, "Result:");
			for(int i = 1; i <= operand[0]; i++){
				myArray[i - 1] = operand[i];
			}
		}

		state = WAIT;
		break;

	case WAIT:
		if (operation == 7){
			returnMenu = TouchInput(CALC);
			if(returnMenu == 11){
				DisplayPrint(CALC, 1, "%u", operand[array_scroll]);

				if(array_scroll < operand[0]){
					array_scroll++;
				}
				else{
					array_scroll = 1;
				}
			}
			if(returnMenu == 10){
				state = MENU;
			}
		}
		else{
			if(TouchInput(CALC) == 11){
				state = MENU;
			}
		}
		break;
	}
}
