#include "motor.h"
#include "math.h"


void motor_init(void)
{
	pwm_init(MOTORL_PWM1, 17000, 0);
	pwm_init(MOTORL_PWM2, 17000, 0);
	pwm_init(MOTORR_PWM1, 17000, 0);
	pwm_init(MOTORR_PWM2, 17000, 0);

    ctimer_count_init(ENCODER_L_A);	//初始化定时器0作为外部计数
	ctimer_count_init(ENCODER_R_A);	//初始化定时器3作为外部计数
//    gpio_pull_set(ENCODER_L_B,PULLUP);
//	gpio_pull_set(ENCODER_R_B,PULLUP);
    //pit_ms_init(PIT_CH0, 5);
}

void get_encoder(int16* speed_l, int16* speed_r)
{
	*speed_l = (int16)ctimer_count_read(ENCODER_L_A);
	*speed_r = (int16)ctimer_count_read(ENCODER_R_A);

	//计数器清零
	ctimer_count_clean(ENCODER_L_A);
	ctimer_count_clean(ENCODER_R_A);

	//采集方向信息
	if(0 == ENCODER_L_B)    
	{
		*speed_l = -*speed_l;
	}
	if(1 == ENCODER_R_B)    
	{
		*speed_r = -*speed_r;
	}
}


void motor_control(int32 duty_l, int32 duty_r)
{
    //对占空比限幅
	duty_l = (int)limit((float)duty_l, 10000);
	duty_r = (int)limit((float)duty_r, 10000);
    
    if(duty_l >= 0)											// 左侧正转
    {
        pwm_duty(MOTORL_PWM1, duty_l); 
        pwm_duty(MOTORL_PWM2, 0);                  
            
    }
    else													// 左侧反转
    {
        pwm_duty(MOTORL_PWM1, 0); 
        pwm_duty(MOTORL_PWM2, -duty_l);             
    }
    
    if(duty_r >= 0)											// 右侧正转
    {
        pwm_duty(MOTORR_PWM1, duty_r); 
        pwm_duty(MOTORR_PWM2, 0);                  
    }
    else													// 右侧反转
    {
        pwm_duty(MOTORR_PWM1, 0); 
        pwm_duty(MOTORR_PWM2, -duty_r);             
    }
}
