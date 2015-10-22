#include "interface.h"
#include "led_map.h"

uint8_t degrees_to_value(int degrees)
{
	/*
	Use the following values to illuminate target lights.

	80-90 => Only 90
	60-80 => 50 & 90
	45-60 => Only 50
	35-45 => 30 & 50
	25-35 => Only 30
	15-25 => 10 & 30
	5-15 => Only 10
	-5-5 => -10 & 10
	-15-5 => Only -10
	-25-15 => -10 & -30
	-35-25 => Only -30
	-45-35 => -30 & -50
	-60-45 => Only -50
	-80-60 => -50 & -90
	-80-90 => Only -90
	*/
	if(degrees >= 80)
		return 0x01;
	else if(degrees >= 60)
		return 0x03;
	else if(degrees >= 45)
		return 0x02;
	else if(degrees >= 35)
		return 0x06;
	else if(degrees >= 25)
		return 0x04;
	else if(degrees >= 15)
		return 0x0C;
	else if(degrees >= 5)
		return 0x08;
	else if(degrees >= -5)
		return 0x18;
	else if(degrees >= -15)
		return 0x10;
	else if(degrees >= -25)
		return 0x30;
	else if(degrees >= -35)
		return 0x20;
	else if(degrees >= -45)
		return 0x60;
	else if(degrees >= -60)
		return 0x40;
	else if(degrees >= -80)
		return 0xC0;
	else
		return 0x80;
}

void update_main_target(int direction)
{
	/*
	The LEDs on the board aren't lined up directly to the
	most intuitive pins on the TPIC, so need to translate the
	value here first.
	*/
	TWI_SetState(MAIN_DIRECTION, degrees_to_value(direction));
}

void update_other_target(uint8_t count, int* direction)
{
	uint8_t new_state = 0;
	int i = 0;

	for(i = 0; i < count; i++)
	{
		new_state |= degrees_to_value(direction[i]);
	}

	TWI_SetState(OTHER_DIRECTION, new_state);
}