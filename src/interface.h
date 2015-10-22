#ifndef __INTERFACE__
#define __INTERFACE__

#include <stdint.h>

void update_main_target(int direction);
void update_other_target(uint8_t count, int* direction);

#endif