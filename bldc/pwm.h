
#ifndef _pwm_h_
#define _pwm_h_


#include "common.h"

#define PWM_A_H_PIN     P20
#define PWM_A_L_PIN     P21


#define PWM_B_H_PIN     P22
#define PWM_B_L_PIN     P23

#define PWM_C_H_PIN     P24
#define PWM_C_L_PIN     P25


void pwm_brake(void);
void pwm_close_output(void);
void pwm_a_bn_output(void);
void pwm_a_cn_output(void);
void pwm_b_cn_output(void);
void pwm_b_an_output(void);
void pwm_c_an_output(void);
void pwm_c_bn_output(void);
void pwm_isr_close(void);
void pwm_isr_open(void);
void pwm_center_duty_update(uint16 duty);
void pwm_center_init(void);

#endif
