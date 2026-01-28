

#include "zf_pwm.h"
#include "zf_gpio.h"
#include "zf_delay.h"
#include "zf_exti.h"
#include "zf_tim.h"
#include "comparator.h"
#include "bldc_config.h"
#include "pwm.h"
#include "battery.h"
#include "motor.h"


motor_struct motor;
uint8 timer4_isr_flag;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机step加一
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_next_step(void)
{
    motor.step++;
    while(6 <= motor.step)
    {
        motor.step -= 6;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机换相函数
//  @param      void 		    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_commutation(void)
{
    switch(motor.step)
    {
        case 0:
        {
            pwm_a_bn_output();
            if(3 == motor.run_flag) 
            {
                comparator_falling();
            }

        }break;
        case 1:
        {
            pwm_a_cn_output();
            if(3 == motor.run_flag) 
            {
                comparator_rising();
            }

        }break;
        case 2:
        {
            pwm_b_cn_output();
            if(3 == motor.run_flag) 
            {
                comparator_falling();
            }

        }break;
        case 3:
        {
            pwm_b_an_output();
            if(3 == motor.run_flag) 
            {
                comparator_rising();
            }

        }break;
        case 4:
        {
            pwm_c_an_output();
            if(3 == motor.run_flag) 
            {
                comparator_falling();
            }

        }break;
        case 5:
        {
            pwm_c_bn_output();
            if(3 == motor.run_flag) 
            {
                comparator_rising();
            }

        }break;
        default:break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      查找最大 最小值（PWM中断内使用）
//  @param      *dat            需要查找的数据目的地址
//  @param      length 	        数据长度
//  @param      *max 	        保存最大值的地址
//  @param      *min 	        保存最小值的地址
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_max_min_pwm(uint16 *dat, uint8 length, uint16 *max, uint16 *min)
{
    uint8 i;
    uint16 temp;

    *min = 0xffff;
    *max = 0;
    for(i=0; i<length; i++)
    {
        temp = dat[i];
        if(temp > *max)
        {
            *max = temp;
        }
        if(temp < *min)
        {
            *min = temp;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      查找最大 最小值（比较器中断内使用）
//  @param      *dat            需要查找的数据目的地址
//  @param      length 	        数据长度
//  @param      *max 	        保存最大值的地址
//  @param      *min 	        保存最小值的地址	    
//  @return     				
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_max_min_comparator(uint16 *dat, uint8 length, uint16 *max, uint16 *min)
{
    uint8 i;
    uint16 temp;

    *min = 0xffff;
    *max = 0;
    for(i=0; i<length; i++)
    {
        temp = dat[i];
        if(temp > *max)
        {
            *max = temp;
        }
        if(temp < *min)
        {
            *min = temp;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器4重新配置
//  @param      void
//  @return     uint16          返回当前计时器的时间
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
uint16 tim4_reconfig(void)
{
    uint16 temp;
    // 获取换相时间
    T4T3M &= ~0x80; // 停止定时器
    if(timer4_isr_flag)
    {
        timer4_isr_flag = 0;
        temp = 65535;
    }
    else
    {
        temp = T4H;
        temp = (temp << 8) | T4L;
    }
    T4L = 0;
    T4H = 0;
    T4T3M |= 0x80;  // 开启定时器
    
    return temp;
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器3重新配置
//  @param      us              下一次触发定时器中断的时间 单位微秒
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void tim3_reconfig(uint32 us)
{
    // 主频30M 定时器12T模式  定时器值=0xffff - us/(1/(30/12))  即定时器值=0xffff - us*5/2
    // 后续将主频提升为33.1776MHZ，由于相较于30M差距并不明显，因此这里不再重新计算转换系数
    us = us*5/2;
    if(0xffff < us)
    {
        us = 0xffff;
    }
    
    us = 0xffff - us;
    
    T4T3M &= ~0x08;         // 停止定时器
    T3L = (uint8)us;
    T3H = (uint8)(us >> 8); 	
    T4T3M |= 0x08;          // 开启定时器
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器0重新配置
//  @param      time            下一次触发定时器中断的时间
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void timer0_reconfig(uint16 time)
{
    TR0 = 0; 
    TL0 = (uint8)time;
    TH0 = (uint8)(time >> 8); 	
    TR0 = 1; 
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器4中断函数
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM4_Isr(void) interrupt 20
{
    T4T3M &= ~0x80; // 关闭定时器
	TIM4_CLEAR_FLAG;
    
    // 换相超时
    timer4_isr_flag = 1;
    
    if((2 == motor.run_flag) || (3 == motor.run_flag))
    {
        // 正在运行的时候 进入此中断应该立即关闭输出
        motor_stop();
        motor.restart_delay = BLDC_START_DELAY;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器3中断函数
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM3_Isr(void) interrupt 19
{
	TIM3_CLEAR_FLAG;        // 清除中断标志
	T4T3M &= ~0x08;         // 停止定时器

    if(500 < motor.motor_start_delay)
    {
        motor.motor_start_delay -= motor.motor_start_delay / 100;//128
    }
    else
    {
        motor.motor_start_wait++;
        if(10 < motor.motor_start_wait)
        {
            // 强制启动失败后停止运行
            motor_stop();
            motor.restart_delay = BLDC_START_DELAY;
        }
    }

    if(1 == motor.run_flag)
    {
        motor.degauss_flag = 0;
        motor_next_step();
        motor_commutation();
        // 重新配置定时器
        tim3_reconfig((uint16)motor.motor_start_delay);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器0中断函数
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM0_Isr(void) interrupt 1
{
    // 关闭定时器
    TR0 = 0;

    if(2 == motor.degauss_flag)
    {
        // 完成消磁
        motor.degauss_flag = 0;
    }
}
//-------------------------------------------------------------------------------------------------------------------
//  @brief      PWMA中断函数
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pwma_isr(void) interrupt 26            // PWM中断函数, 根据PWM初始化设置，中断会在PWM高电平的中间触发
{
    static uint8 last_comparator_result = 0;
    static uint8 comparator_result = 0;
    
    uint16 temp;
    uint16 max, min, average;

    if(PWMA_SR1 & 0x10)
    {
        PWMA_SR1 = 0;       // 清除标志位
        last_comparator_result = comparator_result;
        comparator_result = comparator_result_get();

        if(last_comparator_result != comparator_result)
        {
            if(0 == motor.degauss_flag)
            {
                // 获取换相时间
                temp = tim4_reconfig();

                // 去掉最早的数据
                motor.commutation_time_sum -= motor.commutation_time[motor.step];
                
                // 保存换相时间
                motor.commutation_time[motor.step] = temp;
                
                // 叠加新的换相时间，求6次换相总时长
                motor.commutation_time_sum += temp;
                
                // 最近6次换相最大值与最小值
                motor_max_min_pwm(motor.commutation_time, 6, &max, &min);
                
                // 6次换相的平均值
                average = motor.commutation_time_sum / 6;
                
                // 电机在进行启动的时候不做其他处理
                if(2 == motor.run_flag)
                {
                    motor.commutation_num++;
                    // 如果平均值小于最大值的一半 或者 平均值大于最小值的两倍 则认为堵转
                    if(((max >> 1) > average) || ((min << 1) < average) || (15000 < max) || (100 > min))
                    {
                        // 检测到堵转事件
                        motor.commutation_failed_num++;
                    }
                    else if(motor.commutation_failed_num)
                    {
                        // 没有检测到堵转事件 记录值减1
                        motor.commutation_failed_num--;
                    }
                    
                    if(BLDC_COMMUTATION_FAILED_MAX < motor.commutation_failed_num)
                    {
                        // 堵转次数达到一定次数
                        motor_stop();
                        motor.restart_delay = BLDC_START_DELAY;
                    }
                    else
                    {
                        if(200 < motor.commutation_num)
                        {
                            // 关闭PWM中断
                            pwm_isr_close();
                            
                            // 启动完成，后续换相的时候会打开比较器中断
                            motor.run_flag = 3;
                            
                            // 清除比较器标志位
                            CMPCR1 &= ~0x40;
                        }
                        
                        motor.degauss_flag = 2;
                        motor_next_step();
                        motor_commutation();
                        
                        // 设置一定的延时，延时到之后才判断过零信号
                        timer0_reconfig(65535 - average/4);
                    }
                }
                else if(1 == motor.run_flag)
                {
                    // 过零检测到的换相时间在合理的范围内时切入闭环控制
                    // 这里的闭环控制并不是表示PID闭环，而是表示电机换相不再根据延时换相，而是根据检测到的过零信号来进行换相
                    
                    // 当转速大于一定，并且换相的最大与最小之间的差小于和的25%
                    if((max < 5000) && ((max - min) < (max + min)>>2) && (min>500))
                    {
                        motor.run_flag = 2;     // 进入闭环并持续运行一段时间 等待稳定
                        T4T3M &= ~0x08;         // 停止定时器3
                        motor_next_step();
                        motor_commutation();
                    }
                }
            }
        }
    }
    
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      比较器中断函数
//  @param      void            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void comparator_isr(void) interrupt 21		// 比较器中断函数, 检测到反电动势过0事件
{
    uint16 temp;
    uint16 max, min, average;
    
    CMPCR1 &= ~0x40;	// 需软件清除中断标志位
    
    if(0 == motor.degauss_flag && 3 == motor.run_flag)
    {
        // 获取换相时间
        temp = tim4_reconfig();

        // 去掉最早的数据
        motor.commutation_time_sum -= motor.commutation_time[motor.step];
        
        // 保存换相时间
        motor.commutation_time[motor.step] = temp;
        
        // 叠加新的换相时间，求6次换相总时长
        motor.commutation_time_sum += temp;
        
        // 最近6次换相最大值与最小值
        motor_max_min_comparator(motor.commutation_time, 6, &max, &min);
        
        // 6次换相的平均值
        average = motor.commutation_time_sum / 6;

        motor.commutation_num++;
        // 如果平均值小于最大值的一半 或者 平均值大于最小值的两倍 则认为堵转
        if(((max >> 1) > average) || ((min << 1) < average) || (15000 < max) || (100 > min))
        {
            // 检测到堵转事件
            motor.commutation_failed_num++;
        }
        else if(motor.commutation_failed_num)
        {
            // 没有检测到堵转事件 记录值减1
            motor.commutation_failed_num--;
        }
        
        if(BLDC_COMMUTATION_FAILED_MAX < motor.commutation_failed_num)
        {
            // 堵转次数达到一定次数
            motor_stop();
            motor.restart_delay = BLDC_START_DELAY;
        }                                                                                                                                                                                                                                                       
        else
        {
            if(0 == (motor.commutation_num%6))
            {
                if(motor.duty > motor.duty_register)
                {
                    motor.duty_register += 1;
                    if(BLDC_MAX_DUTY < motor.duty_register)
                    {
                        motor.duty_register = BLDC_MAX_DUTY;
                    }
                }
                else
                {
                    motor.duty_register = motor.duty;
                }
                pwm_center_duty_update(motor.duty_register);
            }
            
            // 修改PWM占空比
            motor.degauss_flag = 2;
            motor_next_step();
            motor_commutation();
            // 设置一定的延时，延时到之后才判断过零信号
            timer0_reconfig(65535 - average/4);
        }
    }
}

#define MUSIC_NUM   15
uint16  frequency_spectrum[6] = {0, 523, 587, 659, 698, 783};
uint8   music_frequency[MUSIC_NUM] = {3,3,4,5,5,4,3,2,1,1,2,3,3,2,2};
uint16  music_wait_time = 250;
//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机上电鸣叫
//  @param      volume          鸣叫音量大小            
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_power_on_beep(uint16 volume)
{
    int8 i;
    uint16 beep_duty;

    beep_duty = volume;
    
    // 保护限制，避免设置过大烧毁电机
    if(100 < beep_duty)
    {
        beep_duty = 100;
    }

    PWM_A_H_PIN = 0;
	PWM_A_L_PIN = 0;
	PWM_B_H_PIN = 0;
	PWM_B_L_PIN = 0;
	PWM_C_H_PIN = 0;
	PWM_C_L_PIN = 0;
    
    PWM_B_L_PIN = 1;
    for(i = 0; i < MUSIC_NUM; i++)
    {
        pwm_init(PWMA_CH1P_P20, frequency_spectrum[music_frequency[i]], beep_duty);
        //pwm_init(PWMA_CH3P_P24, frequency_spectrum[music_frequency[i]], beep_duty);
        delay_ms(music_wait_time);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机停止
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_stop(void)
{
    motor.run_flag = 0;
    pwm_center_duty_update(0);
    pwm_brake();
    comparator_close_isr();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机启动
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_start(void)
{
    uint16 i;
    
    motor.run_flag = 1;       // 正在启动电机
    
    // 转动之前先刹车
    pwm_brake();
    delay_ms(100);
    
    // 清除一些必要的变量
    motor.motor_start_wait = 0;
    motor.commutation_num = 0;
    for(i=0; i<6; i++)
    {
        motor.commutation_time[i] = 0;
    }
    motor.commutation_time_sum = 0;
    motor.commutation_failed_num = 0;
    motor.degauss_flag = 0;
    
    // 关闭输出
    pwm_close_output();
    // 打开PWM中断
    pwm_isr_open();

    // 转子预定位
    motor.step = 0;
    // 预定位的时候适当缩小占空比
    motor.duty_register = ((float)BLDC_START_VOLTAGE * BLDC_MAX_DUTY / ((float)battery_voltage/1000))*3/4; 
    pwm_center_duty_update(motor.duty_register);
    motor_commutation();
    delay_ms(800);
    
    // 设置开环启动的占空比
    motor.duty_register = ((float)BLDC_START_VOLTAGE * BLDC_MAX_DUTY / ((float)battery_voltage/1000));
    pwm_center_duty_update(motor.duty_register);
    
    // 设置首次换相间隔
    motor.motor_start_delay = 5000;
    
    // 配置定时器并开始换相使得电机转动
    tim3_reconfig((uint16)motor.motor_start_delay);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电机初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void motor_init(void)
{
    // 变量清零
    motor.duty = 0;
    motor.duty_register = 0;
    motor.run_flag = 0;
    motor.degauss_flag = 0;
    motor.motor_start_delay = 0;
    motor.motor_start_wait = 0;
    motor.restart_delay = 0;
    motor.commutation_time_sum = 0;
    motor.commutation_num = 0;

    // T0 T1 12T模式
    AUXR &= ~(0X03 << 6);
    
    // T3 T4 12T模式
    T4T3M &= ~0x22; 
    
    // 使能定时器4、3中断
    IE2 |= 0x60;
    
    // 使能定时器0中断
    ET0 = 1;
    
    IP = 0;
    IPH = 0;
    IP2 = 0;
    IP2H = 0;
    
    // 设置比较器与PWM的中断为最高优先级
    IP |= 1<<7;
    IPH |= 1<<7;
    
    IP2 |= 1<<5;
    IP2H |= 1<<5;
    
#if (1 == BLDC_BEEP_ENABLE)
    // 电机鸣叫表示初始化完成
    motor_power_on_beep(BLDC_BEEP_VOLUME);  
#endif
}