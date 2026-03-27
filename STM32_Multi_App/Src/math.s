/*
 * math.s
 *
 *  Created on: Nov 5, 2025
 *      Author: justinlee
 */

.syntax unified
.cpu cortex-m33
.thumb

.section .text

//Given
.global Increment
.type Increment, %function
Increment:
	add		r0, r0, #1
	bx		lr

//Given
.global Decrement
.type Increment, %function
Decrement:
	sub 	r0, r0, #1
	bx 		lr

//Homemade
.global FourFunction
.type FourFunction, %function
FourFunction:
	cmp r0, #1
    beq add_case
    cmp r0, #2
    beq sub_case
    cmp r0, #3
    beq mul_case
    cmp r0, #4
    beq div_case
    bx lr

	add_case:
    add r0, r1, r2
    bx lr
	sub_case:
    sub r0, r1, r2
    bx lr
	mul_case:
    mul r0, r1, r2
    bx lr
	div_case:
    udiv r0, r1, r2
    bx lr

//Homemade with love
.global GCD
.type GCD, %function
GCD:
	//check that both inputs are not zero
	CMP r0, #0
	BEQ GCD_done
	CMP r1, #0
	BEQ GCD_done

	loop_GCD:
	CMP r0, r1
	BEQ GCD_done
	BGT Foo //wow great name
	BLT Bar //wow great name again

	Foo:
	SUB r0, r0, r1
	B loop_GCD

	Bar:
	SUB r1, r1, r0
	B loop_GCD

	GCD_done:
	bx lr

//From Tutorial 4, Slide 8
.global Factorial
.type Factorial, %function
Factorial:
	PUSH {r4, lr}
	MOV r4, r0
	CMP r4, #0
	BNE NZ
	MOVS r0, #0x01
	loop_Factorial:
	POP {r4, pc}
	NZ:
	SUBS r0, r4, #1
	BL Factorial
	MUL r0, r4, r0
	B loop_Factorial

	END:
	MOV r0, #6
	BL Factorial
	MOV r4, r0

//Homemade
.global Fibonacci
.type Fibonacci, %function
Fibonacci:
	CMP r0, #1 //check if r0 is less than 1
	BLE base_0
	CMP r0, #1 //check if r0 is equal to 1
	BEQ base_1

	MOV r3, r0 //r3 is our loop counter
	SUB r3, #2
	MOV r0, #0 //initial values for fibonacci r0 = 0
	MOV r1, #1 //r1 = 1

	loop_Fibo:
	ADD r4, r0, r1 //sum the two previous numbers r0 + r1
	MOV r0, r1
	MOV r1, r4
	SUB r3, #1 //subtract the loop counter
	CMP r3, #0
	BGT loop_Fibo
	BLE end_Fibo

	base_0:
	MOV r0, #0
	B Fibo_done

	base_1:
	MOV r0, #1
	B Fibo_done

	end_Fibo:
	MOV r0, r4
	B Fibo_done

	Fibo_done:
	bx lr

//From Group 31
.global Sort
.type Sort, %function
	Sort:
		PUSH {r4-r9, LR}
		MOV r3, r0
		MOV r2, #0
		SUB r5, r1, #1

	loop1:
		CMP r2, r5
		BGE loop_end

	MOV r4, r2
	ADD r6, r2, #1

	loop2:
		CMP r6, r1
		BGE loop1_continue

	LDR r8, [r3, r6, LSL #2]
	LDR r9, [r3, r4, LSL #2]
	CMP r8, r9
	BGE skip_min

	MOV r4, r6

	skip_min:
		ADD r6, r6, #1
		B loop2

	loop1_continue:
		CMP r4, r2
		BEQ no_swap

	LDR r8, [r3, r2, LSL #2]
	LDR r9, [r3, r4, LSL #2]
	STR r8, [r3, r4, LSL #2]
	STR r9, [r3, r2, LSL #2]

	no_swap:
		ADD r2, r2, #1
		B loop1

	loop_end:
		POP {r4-r9, LR}
		BX LR


//Homemade with no love
.global Average
.type Average, %function
Average:
	PUSH {r4-r8, lr}
	MOV r3, r0
	MOV r0, #0 // Sum
	MOV r4, #1 // loop counter


	loop_Avg:
	LDR r5, [r3, r4, LSL #2]
	ADD r0, r0, r5
	ADD r4, r4, #1
	CMP r4, r1
	BLE loop_Avg
	B Avg_done

	Avg_done:
	UDIV r0, r0, r1
	//MOV r0, r3
	POP {r4-r8,lr}
	bx lr




