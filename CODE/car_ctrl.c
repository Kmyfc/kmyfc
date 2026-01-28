#include "car_ctrl.h"
#include "math.h"

//速度参数
int16 get_speed_l, get_speed_r, goal_speed_l = 0, goal_speed_r = 0, s_goal_speed = 0;
int err_speed_l[3]={0}, err_speed_r[3]={0};
float speed_pid_p = 10, speed_pid_i = 3, speed_pid_d = 10, speed_pid_out_l, speed_pid_out_r;
int16 up_speed = 0;

//角速度参数
int16 get_angvel, goal_angvel = 0;
float angvel_err, last_angvel_err;
float angvel_pid_p = 0.05, angvel_pid_i = 0, angvel_pid_d = 0.01, angvel_pid_out;

//转向参数
uint8 adc_get_l, adc_get_m, adc_get_r;
float turn_err, last_turn_err;
float turn_k1, turn_k2, turn_pid_dp, turn_pid_sp, turn_pid_dd, turn_pid_sd, turn_pid_out;

//初始化
void car_ctrl_init()
{
	motor_init();
	
	icm20602_init_simspi();
	lcd_init();
	adc_init(ADC_P00, ADC_SYSclk_DIV_2);
	adc_init(ADC_P01, ADC_SYSclk_DIV_2);
	adc_init(ADC_P05, ADC_SYSclk_DIV_2);
	pit_timer_ms(TIM_1, 5);
    pwm_init(PWMB_CH1_P74, 50, 500);
}

//设置左右轮目标速度
void set_goal_speed(int16 l, int16 r)
{
	goal_speed_l = l;
	goal_speed_r = r;
}

//速度PID计算
void speed_pid_ctrl()
{
	get_encoder(&get_speed_l, &get_speed_r);
	
    err_speed_l[2]=err_speed_l[1];
    err_speed_l[1]=err_speed_l[0];
    err_speed_l[0]=goal_speed_l-get_speed_l;
	
    speed_pid_out_l+=(err_speed_l[0]-err_speed_l[1])*speed_pid_p+(float)err_speed_l[0]*speed_pid_i+((float)err_speed_l[0]-2*(float)err_speed_l[1]+(float)err_speed_l[2])*speed_pid_d;
    
    speed_pid_out_l = limit(speed_pid_out_l, 10000);
	
    err_speed_r[2]=err_speed_r[1];
    err_speed_r[1]=err_speed_r[0];
    err_speed_r[0]=goal_speed_r-get_speed_r;
	
    speed_pid_out_r+=(err_speed_r[0]-err_speed_r[1])*speed_pid_p+(float)err_speed_r[0]*speed_pid_i+((float)err_speed_r[0]-2*(float)err_speed_r[1]+(float)err_speed_r[2])*speed_pid_d;
    
    speed_pid_out_r = limit(speed_pid_out_r, 10000);
    motor_control((int)speed_pid_out_l,(int)speed_pid_out_r);
}

//设置目标角速度
void set_goal_angvel(int16 angvel)
{
	goal_angvel = angvel;
}

//角速度PID计算
void angvel_pid_ctrl()
{
	get_icm20602_gyro_simspi();
	get_angvel = icm_gyro_x + 15;
	angvel_err = goal_angvel - get_angvel;
	angvel_pid_out = angvel_err * angvel_pid_p + (angvel_err - last_angvel_err) * angvel_pid_d;
	last_angvel_err = angvel_err;
	set_goal_speed(s_goal_speed + angvel_pid_out, s_goal_speed - angvel_pid_out);
}

//转向PID计算
void turn_pid_ctrl()
{
	uint8 temp_m;
    get_icm20602_gyro_simspi();
	
	if(turn_pid_dp)
	{
		if(adc_get_l < 30 && adc_get_m < 30 & adc_get_r < 30)s_goal_speed = 0;
		else s_goal_speed = f_s_except;
	}
	else s_goal_speed = f_s_except;
	
	adc_get_l = adc_once(ADC_P01, ADC_8BIT);
	adc_get_m = adc_once(ADC_P00, ADC_8BIT);
	adc_get_r = adc_once(ADC_P05, ADC_8BIT);
	
	temp_m = adc_get_m;
	if(temp_m > 200)temp_m = 200;
	
    if((adc_get_l + adc_get_r + adc_get_m) > 10)
	{
		turn_err = 200 - temp_m;
		turn_err = turn_err * ((adc_get_l - adc_get_r)/abs(adc_get_l - adc_get_r));
	}
	else turn_err = 0;
	
	
//    turn_err = limit(turn_err, 400);

    turn_k1 = turn_pid_sp;//turn_err * turn_err *  0 / 1000 + turn_pid_sp;
    turn_k2 = 0;//turn_err * turn_err *  turn_pid_dd/100 + turn_pid_sd + get_speed / 50;
    turn_pid_out = turn_k1 * turn_err + turn_k2 * (turn_err - last_turn_err);// - limit(icm_gyro_x, 3000);
    last_turn_err = turn_err;
    
    //turn_pid_out = limit(turn_pid_out, 8000);
	set_goal_angvel(turn_pid_out);
}

//串级控制
void car_ctrl()
{
	turn_pid_ctrl();
	angvel_pid_ctrl();
    speed_pid_ctrl();
}