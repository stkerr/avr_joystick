#include "switch_type.h"
#include "joystick.h"

/*
 * This functions drives one of six LEDs using three
 * I/O pins, PORTB[4:6].
 */
int temp = 0;

void switch_led_type(enum TYPE type)
{
    type = type & 0x07;

    /*
        Depending on the state selected, configure the PORTB pins
        to set one pin high, one pin low, and one pin Hi-Z.

        DDR = 1 is output, PORTB is the value.
        DDR = 0 is input, PORTB = 0 is Hi-Z.
     */
    switch(type)
    {
        case AIRBORNE:
            DDRB = (1<<6) | (1 << 4) | 0x0F;
            PORTB = (1<<4);
            break;
        case LONG_RANGE:
            DDRB = (1<<6) | (1 << 4) | 0x0F;
            PORTB = (1<<6);
            break;
        case MEDIUM_RANGE:
            DDRB = (1 << 5) | (1 << 4) | 0x0F;
            PORTB = (1<<4);
            break;
        case SHORT_RANGE:
            DDRB = (1<<6) | (1 << 5) | 0x0F;
            PORTB = (1<<5);
            break;
        case EARLY_WARNING:
            DDRB = (1 << 5) | (1 << 4) | 0x0F;
            PORTB = (1<<5);
            break;
        case AWACS:
            DDRB = (1<<6) | (1 << 5) | 0x0F;
            PORTB = (1<<6);
            break;
    }
}
