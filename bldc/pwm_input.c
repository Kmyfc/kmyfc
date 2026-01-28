
#include "board.h"
#include "bldc_config.h"
#include "motor.h"
#include "pwm_input.h"

#define PWMIN_PIN   P00

pwmin_struct pwmin;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB输入捕获中断
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwmb_isr()interrupt 27
{
    uint16 temp;
	if(PWMB_SR1 & 0x02)//捕获/比较5中断标记
	{
		pwmin.period = (PWMB_CCR1H << 8) + PWMB_CCR1L;	    // CC1捕获周期宽度
		PWMB_SR1 = 0;
        
        // 计算输入PWM信号的频率
        pwmin.frequency = sys_clk / (PWMB_PSCRL + 1) / pwmin.period;
	}
	
	if(PWMB_SR1 & 0x04)//捕获/比较6中断标记
	{
		pwmin.high_value = (PWMB_CCR2H << 8) + PWMB_CCR2L;   // CC2捕获高电平宽度
		PWMB_SR1 = 0;
        
        // 频率在合理的范围内才计算
        if((30 < pwmin.frequency) && (400 > pwmin.frequency))
        {
            // 计算高电平时间 仅在高电平时间为1-2ms内有效 
            // pwmin.high_time = pwmin.high_value * (PWMB_PSCRL + 1) * 1000 / (sys_clk/1000);
            
            // 实际上分频系数设置为33 计数值1000就等于1005us左右，因此忽律他们之间的误差，加快运行速度
            pwmin.high_time = pwmin.high_value;
            
            if((3000 < pwmin.high_time) || (1000 > pwmin.high_time))
            {
                // 高电平时间过长或者过短，则油门设置为0
                pwmin.throttle = 0;
            }
            else
            {
                if(2000 < pwmin.high_time)
                {
                    pwmin.high_time = 2000;
                }
                
                // 计算油门大小
                temp = pwmin.high_time - 1000;
                // 如果输入的油门大小 小于启动占空比则油门设置为0
                if(temp < ((uint32)1000 * BLDC_MIN_DUTY / BLDC_MAX_DUTY))
                {
                    temp = 0;
                }
                pwmin.throttle = temp;
            }
        }
        
        // 更新占空比
        motor.duty = (uint32)pwmin.throttle * BLDC_MAX_DUTY / 1000;
	}
    
    if(PWMB_SR1 & 0x01)
    {
        PWMB_SR1 = 0;
        // 未检测到输入信号则输出油门都清零
        pwmin.throttle = 0;
        motor.duty = 0;
    }
    
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMB输入捕获初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwm_input_init(void)
{
    PWMB_PS = 0x0A;		// 通道引脚切换
    PWMB_CCMR1 = 0x01;	// CC5为输入模式,且映射到TI5FP5上
	PWMB_CCMR2 = 0x02;	// CC6为输入模式,且映射到TI5FP6上
    
	// CC5E 开启输入捕获
	// CC5P 捕获发生在TI5F的上升沿
	// CC6E 开启输入捕获
	// CC6P 捕获发生在TI5F的下降沿
    PWMB_CCER1 = 0x31;
    
    PWMB_PSCRH = 0;		// 分频值
	PWMB_PSCRL = 32;    // 分频值
    PWMB_SMCR = 0x54;	// TS=TI1FP1,SMS=TI1上升沿复位模式
	PWMB_CR1 = 0x01;	// 启动PWMB，向上计数
	PWMB_IER = 0x07;	// 使能CC1、CC2、UIE中断

    pwmin.period = 0;
    pwmin.high_value = 0;
    pwmin.high_time = 0;
}