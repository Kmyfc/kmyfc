#ifndef _pwm_input_h
#define _pwm_input_h



#include "common.h"


typedef struct
{
    uint16 frequency;   // 信号频率
    uint16 period;      // 信号周期
    uint16 high_value;  // 信号高电平计数值
    uint16 high_time;   // 信号高电平时间 us
    uint16 throttle;    // 输出油门大小
}pwmin_struct;



extern pwmin_struct pwmin;

void pwm_input_init(void);

#endif
