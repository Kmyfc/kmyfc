
#include "zf_tim.h"
#include "zf_gpio.h"
#include "bldc_config.h"
#include "pwm.h"
#include "pwm_input.h"
#include "motor.h"
#include "battery.h"
#include "pit_timer.h"

#define LED_PIN P44

uint32 stime;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED灯光控制
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_control(void)
{
    // LED状态显示
    if(battery_low_voltage)
    {
        // 电池电压过低，LED慢闪 短亮
        if(0 == (stime%100))
        {
            LED_PIN = 0;
        }
        else if(10 == (stime%100))
        {
            LED_PIN = 1;
        }
    }
    else if(motor.restart_delay)
    {
        // 由于堵转导致停止 LED快闪
        if(0 == (stime%5))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(0 == motor.run_flag)
    {
        // 停止 LED慢闪
        if(0 == (stime%100))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(1 == motor.run_flag)
    {
        // 正在启动 LED较慢闪
        if(0 == (stime%50))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(2 == motor.run_flag)
    {
        // 启动后继续维持一段时间 较快闪烁
        if(0 == (stime%10))
        {
            LED_PIN = !LED_PIN;
        }
    }
    else if(3 == motor.run_flag)
    {
        // 正常运行 LED常亮
        LED_PIN = 0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      定时器1中断
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void TM1_Isr() interrupt 3
{
    stime++;
    
    battery_voltage_get();
    
    led_control();
    
    if(motor.restart_delay)
    {
        // 延时启动时间减减
        motor.restart_delay--;
    }
    else
    {
        if(0 == battery_low_voltage)
        {
            if(motor.duty && (0 == motor.run_flag))
            {
                motor_start();
            }
            else if((0 == motor.duty) && (3 == motor.run_flag))
            {
                motor_stop();
            }
        }
        else
        {
            // 电量过低停止运行
            motor_stop();
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      周期定时器初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void pit_timer_init(void)
{
    stime = 0;
    pit_timer_ms(TIM_1, 10);
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      LED初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void led_init(void)
{
    gpio_mode(P4_4, GPO_PP);
    LED_PIN = 0;
}