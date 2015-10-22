#ifndef __LED_MAPS__
#define __LED_MAPS__

#include "twi.h"
 
enum chip_drivers {
	SIGNAL_STRENGTH_LOW = 0x0,
	SIGNAL_STRENGTH_HIGH = 0x1,
	MISC_DRIVER = 0x2,
	OTHER_DIRECTION = 0x3,
	MAIN_DIRECTION = 0x4
};

#define CLEAR_ALL_LIGHTS \
	TWI_SetState(SIGNAL_STRENGTH_LOW, 0); \
	TWI_SetState(SIGNAL_STRENGTH_HIGH, 0); \
	TWI_SetState(MISC_DRIVER, 0); \
	TWI_SetState(OTHER_DIRECTION, 0); \
	TWI_SetState(MAIN_DIRECTION, 0); \
	set_lock(0);

void inline set_lock(uint8_t value)
{
	if(value)
		PORTB |= 1;
	else
		PORTB &= 0xFE; 
}

#endif