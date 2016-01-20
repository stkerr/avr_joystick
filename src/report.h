#ifndef __REPORT__
#define __REPORT__

#include <stdint.h>

/* Type Defines: */
/** Type define for the joystick HID report structure, for creating and sending HID reports to the host PC.
 *  This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
 */

#ifdef _WIN32
#pragma pack(push, 1)
#endif
typedef struct
{
	int8_t UpdateMask; // If the given bits are set, update the following fields
	int8_t  MiscDriver;
	uint8_t  SignalStrength;
	int8_t  OtherDirection;
	int8_t  MainDirection;
	uint8_t RadarType;
	uint8_t padding[26];
} 
#ifdef _WIN32
USB_JoystickReport_Data_t;
#pragma pack(pop)
#else
__attribute__((packed)) USB_JoystickReport_Data_t;
#endif


enum TYPE {
    AIRBORNE = 0,
    LONG_RANGE,
    MEDIUM_RANGE,
    SHORT_RANGE,
    EARLY_WARNING,
    AWACS,
    OFF
};/**/

// Use these to mask what values to udate
#define UPDATE_MISC (1<<7)
#define UPDATE_SIGNAL_STRENGTH (1<<6)
#define UPDATE_OTHER_DIRECTION (1<<5)
#define UPDATE_MAIN_DIRECTION (1<<4)
#define UPDATE_RADAR_TYPE (1<<03)
#define UPDATE_CLEAR_ALL (1<<1)

// Use these defines for the miscellaneous LEDs
#define LOCK_LED 0x01
#define ABOVE_LED 0x02
#define BELOW_LED 0x04
#define SERVICABILITY_LED 0x08
#define OTHER_LEFT_LED 0x10
#define OTHER_RIGHT_LED 0x20
#define MAIN_LEFT_LED 0x40
#define MAIN_RIGHT_LED 0x80

// helper routine to convert a degree to a hex mask
uint8_t degrees_to_value(int degrees);

/*
 * Rev A notes: Due to PCB errors, you must call this on the results
 * of the hex value returned by degrees_to_value() to ensure that the
 * intended LEDs illuminate.
 */
uint8_t remap_other(uint8_t original);

#endif