#include "LCD1602.h"
#include "motor.h"
#include "DHT11.h"
#include "delay.h"

#define ABS(x) (((x) > 0) ? (x) : (-x))

#define TEMP_VAL_SET 60.0
#define HUMI_VAL_SET 3.5

sbit KEY1 = P2^0; //开始
sbit KEY2 = P2^1; //设置
sbit KEY3 = P2^2; //加
sbit KEY4 = P2^3; //减

sbit heating = P1^0; //电阻丝
sbit BUZZER = P1^6;  //蜂鸣器

unsigned char set_flag = 0;   //0：正常显示 1：设置温度   2：设置湿度
unsigned char mode_flag = 1;  //0：开启     1：关闭烘干   

int motor_val = 0;   //电机pwm比较值
int heating_val = 0; //电阻丝pwm比较值

float temp_value = 0; //传感器采样值
float humi_value = 0;

float temp_value_set = TEMP_VAL_SET;   //烘干预定值
float humi_value_set = HUMI_VAL_SET;

unsigned char str[5];

bit display(void);
void key_function(void);
void manage(void);

void main()
{	
	//初始化
	BUZZER = 0;
	LCD1602_init();
	DHT11_Init();
	motor_init();
	delay_ms(10);

	while (1) {
		display();
		key_function();
		if (!mode_flag)
			manage();
	}
}

//温湿度采集并显示
bit display(void)
{	
	unsigned char len = 0;

	if (mode_flag) {
		BUZZER = 0;
		IN1 = 1; IN2 = 1;//电机不转
		heating_val = 0;
		LCD1602_show_str(0, 0, "Stop");
		return 0;
	}
	IN1 = 0; IN2 = 1; //电机正转
	Dht11_Get_Temp_Humi_Value(&temp_value, &humi_value);
	LCD1602_show_str(0, 1, "T:");
	LCD1602_show_str(0, 0, "H:");

	//设置模式
	if (set_flag) {
		LCD1602_show_str(2, 0, "                    ");
		LCD1602_show_str(2, 1, "                    ");
		sprintf(str, "%.1f", temp_value_set);
		LCD1602_show_str(2, 1, str);
		if (set_flag == 1) {
			LCD1602_show_str(7, 1, "<--");
		} else {
			LCD1602_show_str(7, 0, "<--");	
		}
		sprintf(str, "%.1f", humi_value_set);
		LCD1602_show_str(2, 0, str);
		return 0;
	}
	
	//正常显示模式
	sprintf(str, "%.1f", temp_value);
	LCD1602_show_str(2, 1, str);
	sprintf(str, "%.1f", (humi_value));
	LCD1602_show_str(2, 0, str);

	LCD1602_show_str(7, 1, "res:");
	LCD1602_show_str(7, 0, "m:");
	
	len = LCD1602_integer_to_str(heating_val, str);
	while (len < 3) str[len++] = ' ';
	str[len] = '\0';
	LCD1602_show_str(12, 1, str);
	
	len = LCD1602_integer_to_str(motor_val, str);
	while (len < 3) str[len++] = ' ';
	str[len] = '\0';
	LCD1602_show_str(12, 0, str);
	
	return 1;
}

//按键控制
void key_function(void)
{
	static bit KEY1_flag = 1, KEY2_flag = 1, KEY3_flag = 1, KEY4_flag = 1;
	unsigned char temp = 0;
	
	if (KEY1 == 0) {    //开启烘干
		delay_ms(20);
		if (KEY1_flag) {
			KEY1_flag = 0;
			mode_flag++;
			if(mode_flag > 1)
				mode_flag = 0;
			LCD1602_write_cmd(LCD_SCREEN_CLR);  //清屏
		}
	} else
		KEY1_flag = 1;
	
	if (KEY2 == 0) {    //设置
		delay_ms(10);
		if (KEY2_flag) {
			KEY2_flag = 0;
			set_flag++;
			if(set_flag > 2)
				set_flag = 0;
		}
	} else
		KEY2_flag = 1;

	if (KEY3 == 0) {    //加
		delay_ms(10);
		if (KEY3_flag) {
			KEY3_flag = 0;
			if (set_flag == 1) //温度
				temp_value_set++;
			else if (set_flag == 2) //湿度
				humi_value_set += 0.1; 
		}
	} else
		KEY3_flag = 1;
	
	if (KEY4 == 0) {    //减
		delay_ms(10);
		if (KEY4_flag) {
			KEY4_flag = 0;
			if (set_flag == 1) //温度
				temp_value_set--;
			else if (set_flag == 2) //湿度
				humi_value_set -= 0.1; 
		}
	} else
		KEY4_flag = 1;
}

//pwm控制输出
void manage(void)
{
	if ((temp_value - temp_value_set) > -1) {   //实际温度较高
		heating_val = 0;
		heating = 1;
	} else {
		heating_val = temp_value_set - temp_value;
		heating = 0;
	}
	
	if (humi_value_set > humi_value)
		motor_val = 0;
	else {
		motor_val = ABS((humi_value_set - humi_value) * 200);
		if (motor_val > 200) motor_val = 200;
		else if (motor_val < 10) motor_val = 0;
	}
	
	if (((temp_value - temp_value_set) > 10.0f) || (humi_value_set - humi_value) > 1.0f)
		BUZZER = 1;
	else
		BUZZER = 0;
}

void timer0_ISR(void) interrupt 1
{
	static unsigned char count = 0;
	static unsigned char count2 = 0;
	TR0 = 0;
	count++; count2++;
	
	//pwm比较输出
	if (count < motor_val)
		ENA = 1;
	else
		ENA = 0;
	
	if(count == 200)//20ms
		count = 0;

	TL0 = 0x9C;
	TH0 = 0xFF; //重装初值
	
	TR0 = 1;
}
