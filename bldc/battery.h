#ifndef _battery_h
#define _battery_h

#include "common.h"

extern uint8  battery_low_voltage;
extern uint16 battery_voltage;

void battery_voltage_get(void);
void battery_init(void);

#endif
