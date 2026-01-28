

#ifndef _conparator_h_
#define _conparator_h_

#include "common.h"





extern uint16 motor_commutation_time;



void comparator_select_a(void);
void comparator_select_b(void);
void comparator_select_c(void);
void comparator_rising(void);
void comparator_falling(void);
uint8 comparator_result_get(void);
void comparator_close_isr(void);
void comparator_init(void);


#endif