/*
 * gpio.c
 *
 *  Created on: Sep 23, 2025
 *      Author: justinlee
 */

// General-purpose input/output driver
#include <stddef.h>
#include <stdbool.h>
#include "gpio.h"
#include "i2c.h"
// --------------------------------------------------------
// Initialization
// --------------------------------------------------------
// Enable the GPIO port peripheral clock for the specified pin
void GPIO_Enable (Pin_t pin) {
	GPIO_PortEnable(pin.port);
}

void GPIO_PortEnable(GPIO_TypeDef *port){
	if(port == GPIOX){
		//I2C I/O Expander
		I2C_Enable(LeafyI2C);
	}
	else{
		//Regular GPIO Port
		RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN << GPIO_PORT_NUM(port);
	}
}

// Set the operating mode of a GPIO pin:
// Input (IN), Output (OUT), Alternate Function (AF), or Analog (ANA)
void GPIO_Mode (Pin_t pin, PinMode_t mode) {
	uint32_t pos = pin.bit*2; // Multiply by two because modes are two bit wide (at least I think that's the reason)
	pin.port -> MODER &= ~(0b11 << pos); // Clear the current mode
	pin.port -> MODER |= (uint32_t)mode << pos; // Set the new mode
}

void GPIO_Config(Pin_t pin, PinType_t ot, PinSpeed_t osp, PinPUPD_t pupd){
	uint32_t pos = pin.bit*2;

	pin.port -> OTYPER &= ~(0b1 << pin.bit); //I don't know about this
	pin.port -> OTYPER |= ot << pin.bit; //I don't know, hopefully it works

	pin.port -> OSPEEDR &= ~(0b11 << pos); //Clear
	pin.port -> OSPEEDR |= osp << pos; //Set

	pin.port -> PUPDR &= ~(0b11 << pos); //Clear
	pin.port -> PUPDR |= pupd << pos; //Set
}

//The AFR is a 32 bit wide register with pins 0-7 for Alternate function low register (AFRL)
//and pins 8-15 for Alternate function high register (AFRH)
//Each pin is 4 bits wide!!!!!!
void GPIO_AltFunc(Pin_t pin, int af){
	pin.port -> AFR[pin.bit/8] = (pin.port -> AFR[pin.bit/8] & ~(0xF << 4*(pin.bit%8))) | af << 4*(pin.bit%8);
}

// --------------------------------------------------------
// Pin observation and control
// --------------------------------------------------------
// Observe the value of an input pin
PinState_t GPIO_Input (const Pin_t pin) {
	/* If I wanted to see what pin 7 is, I make a mask (1 << pin.bit) and compare it to the respective bit which would be 7.
	 * pin.bit in this case would be bit 7*/
	if (pin.port->IDR & (1 << pin.bit)){
		return HIGH;
	}
	else{
		return LOW;
	}
}

uint16_t GPIO_PortInput(GPIO_TypeDef *port){
	return port -> IDR;
}

void GPIO_Output (Pin_t pin, const PinState_t state) {
	if(pin.port == GPIOX){
		if(state == HIGH){
			pin.port -> ODR |= 1 << pin.bit;
		}
		else{
			pin.port -> ODR &= ~(1 << pin.bit);
		}
	}
	else{
		if(state == HIGH){
			pin.port -> BSRR = 1 << pin.bit;
		}
		else{
			pin.port -> BSRR = 1 << (pin.bit + 16);
		}
	}
}

//Control states of entire output port
void GPIO_PortOutput(GPIO_TypeDef *port, uint16_t states){
	port -> ODR = states;
}

/* Similar to the function GPIO_Output but uses ODR rather than BSRR
 * Though I don't really know what "toggle" does. I do know BSRR is usually (maybe always) a safer way to write bits out*/
void GPIO_Toggle (Pin_t pin) {
	pin.port -> ODR ^= ~(1 << pin.bit);
}

// --------------------------------------------------------
// Interrupt handling
// --------------------------------------------------------
// Array of callback function pointers
// Bits 0 to 15 (each can select one port GPIOA to GPIOH)
// Rising and falling edge triggers for each
static void (*callbacks[16][2]) (void);
// Register a function to be called when an interrupt occurs
void GPIO_Callback(Pin_t pin, void (*func)(void), PinEdge_t edge){
	callbacks[pin.bit][edge] = func;
	// Enable interrupt generation
	if (edge == RISE){
		EXTI->RTSR1 |= 1 << pin.bit;
	}
	else{ // FALL
		EXTI->FTSR1 |= 1 << pin.bit;
	}
	EXTI->EXTICR[pin.bit / 4] |= GPIO_PORT_NUM(pin.port) << 8*(pin.bit % 4);
	EXTI->IMR1 |= 1 << pin.bit;
	// Enable interrupt vector
	NVIC->IPR[EXTI0_IRQn + pin.bit] = 0;
	__COMPILER_BARRIER();
	NVIC->ISER[(EXTI0_IRQn + pin.bit) / 32] = 1 << ((EXTI0_IRQn + pin.bit) % 32);
	__COMPILER_BARRIER();

}
// Interrupt handler for all GPIO pins
void GPIO_IRQHandler (int i) {
	// Clear pending IRQ
	NVIC->ICPR[(EXTI0_IRQn + i) / 32] = 1 << ((EXTI0_IRQn + i) % 32);
	// Detect rising edge
	if (EXTI->RPR1 & (1 << i)) {
		EXTI->RPR1 = (1 << i); // Service interrupt
		callbacks[i][RISE](); // Invoke callback function
	}
	// Detect falling edge
	if (EXTI->FPR1 & (1 << i)) {
		EXTI->FPR1 = (1 << i); // Service interrupt
		callbacks[i][FALL](); // Invoke callback function
	}
}

/// --------------------------------------------------------
// I/O expander management
// --------------------------------------------------------
// Emulated GPIO registers for I/O expander
GPIO_TypeDef IOX_GPIO_Regs = {0xFFFFFFFF, 0, 0, 0, 0, 0, 0, 0, {0, 0}, 0, 0, 0};


// Transmit/receive data buffers
static uint8_t IOX_txData = 0xFF;
static uint8_t IOX_rxData = 0xFF;

// I2C transfer structures   bus        addr   data      size stop busy next
static I2C_Xfer_t IOX_LEDs = {&LeafyI2C, 0x70, &IOX_txData, 1,  1,  0,  NULL};
static I2C_Xfer_t IOX_PBs = {&LeafyI2C, 0x73, &IOX_rxData,  1,  1,  0,  NULL};

void UpdateIOExpanders(void) {
	// Copy to/from data buffers, with polarity inversion
	IOX_txData = ~(GPIOX->ODR & 0xFF); // LEDs in bits 7:0
	GPIOX->IDR = (~IOX_rxData) << 8; // PBs in bits 15:8
	// Keep requesting transfers to/from I/O expanders
	if (!IOX_LEDs.busy)
		I2C_Request(&IOX_LEDs);
	if (!IOX_PBs.busy)
		I2C_Request(&IOX_PBs);
}
// Dispatch all GPIO IRQs to common handler function
void EXTI0_IRQHandler() { GPIO_IRQHandler(0); }
void EXTI1_IRQHandler() { GPIO_IRQHandler(1); }
void EXTI2_IRQHandler() { GPIO_IRQHandler(2); }
void EXTI3_IRQHandler() { GPIO_IRQHandler(3); }
void EXTI4_IRQHandler() { GPIO_IRQHandler(4); }
void EXTI5_IRQHandler() { GPIO_IRQHandler(5); }
void EXTI6_IRQHandler() { GPIO_IRQHandler(6); }
void EXTI7_IRQHandler() { GPIO_IRQHandler(7); }
void EXTI8_IRQHandler() { GPIO_IRQHandler(8); }
void EXTI9_IRQHandler() { GPIO_IRQHandler(9); }
void EXTI10_IRQHandler() { GPIO_IRQHandler(10); }
void EXTI11_IRQHandler() { GPIO_IRQHandler(11); }
void EXTI12_IRQHandler() { GPIO_IRQHandler(12); }
void EXTI13_IRQHandler() { GPIO_IRQHandler(13); }
void EXTI14_IRQHandler() { GPIO_IRQHandler(14); }
void EXTI15_IRQHandler() { GPIO_IRQHandler(15); }
