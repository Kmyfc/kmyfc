
#ifndef _motor_h_
#define _motor_h_

#include "common.h"




typedef struct
{
    uint8   step;                   // 电机运行的步骤
    uint16  duty;                   // PWM占空比       用户改变电机速度时修改此变量
    uint16  duty_register;          // PWM占空比寄存器 用户不可操作
    vuint8  run_flag;               // 电机正在运行标志位 0:已停止 1：正在启动 2：切入闭环之后再维持一段运行时间 3：启动完成正在运行
    uint8   degauss_flag;           // 0；消磁完成  2：换相完成，正在延时消磁
    float   motor_start_delay;      // 开环启动的换相的延时时间
    uint16  motor_start_wait;       // 开环启动时，换相时间已经降低到最小值后，统计换相的次数
    uint16  restart_delay;          // 电机延时重启
    uint16  commutation_failed_num; // 换相错误次数
    uint16  commutation_time[6];    // 最近6次换相时间
    uint32  commutation_time_sum;   // 最近6次换相时间总和
    uint32  commutation_num;        // 统计换相次数
}motor_struct;


extern motor_struct motor;


void motor_power_on_beep(uint16 volume);
void motor_stop(void);
void motor_start(void);
void motor_init(void);






#endif 