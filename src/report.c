#include <stdint.h>

uint8_t remap_other(uint8_t original)
{
	/*
	Due to some incorrent PCB layout, need
	to re-route some signals from their
	logical values to their real values.
	*/

	// Other LED #7 is incorrectly located, so never use it

	uint8_t new_value = 0;

	if(original & 0x80)
		new_value |= 0x01;
	if(original & 0x40)
		new_value |= 0x02;
	if(original & 0x20)
		new_value |= 0x04;
	if(original & 0x10)
		new_value |= 0x08;
	if(original & 0x08)
		new_value |= 0x10;
	if(original & 0x04)
		new_value |= 0x20;
	if(original & 0x02)
		new_value |= 0x40;
	if(original & 0x01)
		new_value |= 0x80;

	new_value &= 0xFE;

	return new_value;
}

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