#ifndef __INTERFACE__
#define __INTERFACE__

#include <stdint.h>

// Update the main target directional LEDs. Value is -90 to 90 degrees.
void update_main_target(int8_t direction);

// Update the secondary target directional LEDs. Value is -90 to 90 degrees.
// May specify multiple alternative targets.
void update_other_target(uint8_t count, int* direction);

// Update the signal strength LEDS.
//Value ranges from 0x00-0xFF and LEDs are lit accordingly.
void update_signal_strength(uint8_t value);

#endif