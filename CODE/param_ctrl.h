#ifndef _PARAM_CTRL_H_
#define _PARAM_CTRL_H_

#include "headfile.h"

extern float flash_turn_p, flash_turn_d;
extern uint8 f_t_pid_dp, f_t_pid_sp, f_t_pid_dd, f_t_pid_sd, f_s_except;
extern uint8 fan_flag;
void param_ctrl_init(void);
void param_ctrl_loop(void);
#endif