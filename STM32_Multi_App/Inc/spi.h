/*
 * spi.h
 *
 *  Created on: Nov 19, 2025
 *      Author: justinlee
 */

#ifndef SPI_H_
#define SPI_H_

#include <stdbool.h>
#include "stm32l5xx.h"
#include "gpio.h"

//SPI bus connectino
typedef struct{
	SPI_TypeDef *iface; //Interface for SPI1-SPI3
	Pin_t pinSCLK; //MCU pin for SCLK/SCK
	Pin_t pinMISO; //MCU pin for MISO/S)
	Pin_t pinMOSI; //MCU pin for MOSI/SI
	Pin_t pinNSS; //MCU pin for NSS/CSB
} SPI_Bus_t;

extern SPI_Bus_t EnvSPI; //SPI bus for Environmental Sensor

typedef enum {RX=1, TX=0} Direction_t;

//SPI transfer record
typedef struct SPI_Xfer_t{
	SPI_Bus_t *bus; //Pointer to SPI bus structure
	Direction_t dir; //Transfer direction
	uint8_t *data; //Pointer to data buffer
	int size; //total number of bytes in transfer
	bool last; //last transfer in combined sequence
	volatile bool busy; //Busy indicator (queued or in progress)
	struct SPI_Xfer_t *next; //Pointer to next transfer in queue
} SPI_Xfer_t;

void SPI_Enable(SPI_Bus_t bus); //Enable SPI bus
void SPI_Request(SPI_Xfer_t *p); //Request new transfer

void ServiceSPIRequests(void);

#endif /* SPI_H_ */
