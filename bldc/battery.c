
#include "zf_adc.h"
#include "bldc_config.h"
#include "battery.h"

uint8  battery_low_voltage;
uint16 battery_voltage;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电池电压获取
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void battery_voltage_get(void)
{
    uint8 adc_value;
    uint16 pin_voltage;
    static uint16 low_power_num;
    
    adc_value = adc_once(ADC_P03, ADC_8BIT);
    pin_voltage = (uint32)adc_value * 5000 / 256;       // 将ADC值转换为实际的电压
    battery_voltage = (uint32)pin_voltage * 57 / 10;    // 根据硬件分压电阻的值计算电池电压
    
    if((BLDC_MIN_BATTERY*1000 > battery_voltage) && (0 == battery_low_voltage))
    {
        low_power_num++;
        if(100 < low_power_num)
        {
            battery_low_voltage = 1;
            
        }
    }
    else
    {
        low_power_num = 0;
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      电池电压检测初始化
//  @param      void                        
//  @return     void          
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void battery_init(void)
{
    battery_low_voltage = 0;
    
    adc_init(ADC_P03, ADC_SYSclk_DIV_2);
    
    // 初始化的时候先采集一次电压
    battery_voltage_get();
}