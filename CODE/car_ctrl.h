#ifndef _car_ctrl_h
#define _car_ctrl_h

#include "headfile.h"

extern int16 get_speed_l, get_speed_r;
extern float turn_pid_dp, turn_pid_sp, turn_pid_dd, turn_pid_sd, turn_pid_out, speed_pid_out,motor_l_out, motor_r_out;
extern float turn_err,turn_k1, turn_k2;
extern uint8 adc_get_l, adc_get_m, adc_get_r;

extern int16 get_angvel, goal_angvel, s_goal_speed;

extern float angvel_pid_p, angvel_pid_i, angvel_pid_d, angvel_pid_out;
extern float speed_pid_p, speed_pid_i, speed_pid_d;

extern float turn_k1, turn_k2, turn_pid_dp, turn_pid_sp, turn_pid_dd, turn_pid_sd, turn_pid_out;
void car_ctrl_init(void);
void speed_pid_ctrl(void);
void turn_pid_ctrl(void);
void car_ctrl(void);
#endif
