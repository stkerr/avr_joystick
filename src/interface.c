#include "interface.h"
#include "led_map.h"
#include "report.h"

uint8_t remap_main(uint8_t original)
{
	/*
	Due to some incorrent PCB layout, need
	to re-route some signals from their
	logical values to their real values.
	*/
	uint8_t new_value = 0;
	if(original & 0x80)
		new_value |= 0x01;
	if(original & 0x40)
		new_value |= 0x40;
	if(original & 0x20)
		new_value |= 0x04;
	if(original & 0x10)
		new_value |= 0x08;
	if(original & 0x08)
		new_value |= 0x10;
	if(original & 0x04)
		new_value |= 0x20;
	if(original & 0x02)
		new_value |= 0x02;
	if(original & 0x01)
		new_value |= 0x80;

	return new_value;
}

void update_main_target(int8_t direction)
{
	/*
	The LEDs on the board aren't lined up directly to the
	most intuitive pins on the TPIC, so need to translate the
	value here first.
	*/
	uint8_t new_state = remap_main(degrees_to_value(direction));
	TWI_SetState(MAIN_DIRECTION, new_state);
}

void update_other_target(uint8_t count, int* direction)
{
	uint8_t new_state = 0;
	int i = 0;

	for(i = 0; i < count; i++)
	{
		uint8_t temp = remap_other(degrees_to_value(direction[i]));
		new_state |= temp;
	}

	TWI_SetState(OTHER_DIRECTION, new_state);
}

uint8_t other_register = 0;

void set_above(uint8_t value)
{
	if(value)
		other_register |= 0x2;
	else
		other_register &= 0xFD;
}

void set_below(uint8_t value)
{
	if(value)
		other_register |= 0x4;
	else
		other_register &= 0xFB;
}

void set_serviceability(uint8_t value)
{
	if(value)
		other_register |= 0x8;
	else
		other_register &= 0xF7;
}

void set_other_left(uint8_t value)
{
	if(value)
		other_register |= 0x10;
	else
		other_register &= 0xEF;
}

void set_other_right(uint8_t value)
{
	if(value)
		other_register |= 0x20;
	else
		other_register &= 0xDF;
}

void set_main_left(uint8_t value)
{
	if(value)
		other_register |= 0x40;
	else
		other_register &= 0xBF;
}

void set_main_right(uint8_t value)
{
	if(value)
		other_register |= 0x80;
	else
		other_register &= 0x7F;
}

void update_others()
{
	TWI_SetState(MISC_DRIVER, other_register);
}

void update_signal_strength(uint8_t value)
{
	/*
	 * The design has 8 LEDs on the low driver, 7
	 * on the high driver, and the highest bit on the high
	 * driver corresponds to the triangle LED.
	 *
	 * This is easy enough to write as a lookup table below
	 * but can be refactored to an algorithm to compute the values
	 * if space is an issue.
	 *
	 * Rev A notes: The triangle LED is just a normal LED, so
	 * treat the high driver as having 8 LEDS.
	 *
	 * Rev A notes: OTHER_DIRECTION_7 is in line with the signal
	 * strength LEDs, so include that for this revision.
	 * So 17 LEDs total, 1 off state for 18 total states.
	 *
	 * Rev A notes: Note that SIGNAL_STRENGTH_HIGH LEDs 11 & 12 are flipped.
	 */

	uint8_t count = 17;
	uint8_t max = 0xFF;
	uint8_t step = max/count;

	uint8_t state = TWI_GetState(OTHER_DIRECTION);
	if(value < step * 1)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x00);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 2)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x01);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 3)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x03);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 4)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x07);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 5)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x0F);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 6)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x1F);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 7)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x3F);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 8)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0x7F);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 9)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state & 0xFE);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 10)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x00);
	}
	else if(value < step * 11)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x01);
	}
	else if(value < step * 12)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x03);
	}
	else if(value < step * 13)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x07);
	}
	else if(value < step * 14)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x17); // Rev A problem with the LED switched here
	}
	else if(value < step * 15)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x1F); // Rev A problem with the LED switched here
	}
	else if(value < step * 16)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x3F);
	}
	else if(value < step * 17)
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0x7F);
	}
	else
	{
		TWI_SetState(SIGNAL_STRENGTH_LOW, 0xFF);
		TWI_SetState(OTHER_DIRECTION, state | 0x01);
		TWI_SetState(SIGNAL_STRENGTH_HIGH, 0xFF);
	}
}
