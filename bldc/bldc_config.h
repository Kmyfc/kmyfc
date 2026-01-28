#ifndef _bldc_config_h_
#define _bldc_config_h_



#define BLDC_MAX_DUTY                   690     // PWM的最大占空比，需要自行根据PWM初始化去计算

#define BLDC_MIN_DUTY                   50      // PWM的最小占空比，根据输入油门大小计算出来的占空比小于此定义则电机停转 至少要大于50

#define BLDC_START_VOLTAGE              1.25f   // 0-1.5 单位V

#define BLDC_MIN_BATTERY                10.0f   // 最小电压，当检测到电压低于此值电机停转

#define BLDC_COMMUTATION_FAILED_MAX     300     // 换相错误最大次数 大于这个次数后认为电机堵转

#define BLDC_START_DELAY                100     // 当遇到堵转后电机停止时间，之后电机会再次尝试启动（单位10ms）
 
#define BLDC_BEEP_ENABLE                1       // 1:使能上电电机鸣叫功能 0：禁用

#define BLDC_BEEP_VOLUME                30      // 电机鸣叫声音大小 0-100






#endif
