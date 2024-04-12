#include "motor.h"
/*	名称：直流电机测速
    说明：数码管显示的正反转状态与转速值应与motor encode器件上显示的值基本一致；
		  测量误差 ≤1RPM 。
*/

void motor_init(void)
{
	IN1 = 1; //电机不转
	IN2 = 1;
	ENA = 0;
		 		

	TMOD &= 0x0F; //设置定时器模式
	TMOD |= 0x01; //设置定时器模式
	TL0 = 0x9C;   //设置定时初值        100us
	TH0 = 0xFF;   //设置定时初值
	TF0 = 0;      //清除TF1标志
	ET0 = 1;      //打开定时器1中断
	EA = 1;       //打开总中断
	TR0 = 1;      //定时器1开始计时
}


