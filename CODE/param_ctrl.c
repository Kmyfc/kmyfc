#include "param_ctrl.h"

#define FLASH_PARAM_NUM 8
float flash_turn_p, flash_turn_d;
uint8 f_t_pid_dp, f_t_pid_sp, f_t_pid_dd, f_t_pid_sd, f_s_except;
uint8 param_arr[FLASH_PARAM_NUM] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8 menu_key_flag;
uint8 menu_num = 0;
uint8 fan_flag = 0;
void param_ctrl_init()
{
//    gpio_pull_set(P40,PULLUP);
//	gpio_pull_set(P41,PULLUP);
//    gpio_pull_set(P42,PULLUP);
//	gpio_pull_set(P43,PULLUP);
    iap_init();
	//iap_write_bytes(0x00,param_arr,FLASH_PARAM_NUM);
	
	lcd_showstr(100, 0, "--");
	iap_read_bytes(0x00,param_arr,FLASH_PARAM_NUM);
}

void param_ctrl_loop()
{
	uint8 i;
    if(menu_key_flag == 0 && !(P72))
    {
        menu_num++;
        if(menu_num >= FLASH_PARAM_NUM)menu_num = 0;
        menu_key_flag = 1;
    }
    if(menu_key_flag == 0 && !(P71))
    {
        param_arr[menu_num] += 1;
		extern_iap_write_bytes(0x00,param_arr,FLASH_PARAM_NUM);
        menu_key_flag = 1;
    }
    if(menu_key_flag == 0 && !(P73))
    {
        if(param_arr[menu_num])param_arr[menu_num] -= 1;
		extern_iap_write_bytes(0x00,param_arr,FLASH_PARAM_NUM);
        menu_key_flag = 1;
    }
    if(menu_key_flag == 0 && !(P70))
    {
        menu_key_flag = 1;
        if(fan_flag)
        {
			lcd_showstr(100, 0, "--");
            fan_flag = 0;
        }
        else 
        {
			lcd_showstr(100, 0, "!!");
            fan_flag = 1;
        }
    }
	
	iap_read_bytes(0x00,param_arr,FLASH_PARAM_NUM);
    turn_pid_dp = (float)param_arr[0];
    turn_pid_sp = (float)param_arr[1];
    f_s_except = (float)param_arr[2];
    angvel_pid_p = (float)param_arr[3] / 100;
    angvel_pid_d = (float)param_arr[4] / 500;
	speed_pid_p = (float)param_arr[5];
	speed_pid_i = (float)param_arr[6];
	speed_pid_d = (float)param_arr[7];
	lcd_showstr(10, 0, "tp_dp:");lcd_showuint8(60, 0, (uint8)turn_pid_dp);
	lcd_showstr(10, 1, "tp_sp:");lcd_showuint8(60, 1, (uint8)turn_pid_sp);
	lcd_showstr(10, 2, "s_goal:");lcd_showuint8(60, 2, (uint8)f_s_except);
	lcd_showstr(10, 3, "ap_sp:");lcd_showuint8(60, 3, (uint8)(angvel_pid_p * 100));
	lcd_showstr(10, 4, "ap_sd:");lcd_showuint8(60, 4, (uint8)(angvel_pid_d * 500));
	lcd_showstr(10, 5, "sp_sp:");lcd_showuint8(60, 5, (uint8)speed_pid_p);
	lcd_showstr(10, 6, "sp_si:");lcd_showuint8(60, 6, (uint8)speed_pid_i);
	lcd_showstr(10, 7, "sp_sd:");lcd_showuint8(60, 7, (uint8)speed_pid_d);
	for(i = 0; i < FLASH_PARAM_NUM; i++)
	{
		if(i == menu_num)lcd_showchar(0, i*16, '>');
		else lcd_showchar(0, i*16, ' ');
	}
    if((P70) && (P71) && (P72) && (P73))menu_key_flag = 0;

}
