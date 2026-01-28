#ifndef _motor_h
#define _motor_h

#include "headfile.h"

#define     limit(x, y)         ((x) > (y) ? (y) : ((x) < -(y) ? -(y) : (x)))


#define MOTORR_PWM1                 (PWMA_CH3P_P64)
#define MOTORR_PWM2                 (PWMA_CH4P_P66)

#define MOTORL_PWM1                 (PWMA_CH2P_P62)
#define MOTORL_PWM2                 (PWMA_CH1P_P60)

#define ENCODER_L_A                 (CTIM0_P34)
#define ENCODER_L_B                 (P35)

#define ENCODER_R_A                 (CTIM3_P04)
#define ENCODER_R_B                 (P53)


void motor_init(void);
void get_encoder(int16* speed_l, int16* speed_r);
void motor_control(int32 duty_l, int32 duty_r);

#endif
