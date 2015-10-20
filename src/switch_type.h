#ifndef __SWITCH_TYPE__
#define __SWITCH_TYPE__

enum TYPE {
    AIRBORNE = 0,
    LONG_RANGE,
    MEDIUM_RANGE,
    SHORT_RANGE,
    EARLY_WARNING,
    AWACS
};

void switch_led_type(enum TYPE type);

#endif
