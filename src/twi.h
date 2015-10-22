#ifndef __TWI__
#define __TWI__

#include <avr/io.h>

void TWI_Init(void);
void TWI_Start(void);
void TWI_Stop(void);
void TWI_Write(uint8_t u8data);
uint8_t TWI_ReadACK(void);
uint8_t TWI_ReadNACK(void);
uint8_t TWI_GetStatus(void);
void TWI_SetState(uint8_t chip_address, uint8_t data);

#endif