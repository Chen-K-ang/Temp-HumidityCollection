/**********************************
作者：单片机俱乐部
网站：https://www.mcuclub.cn/
**********************************/


#ifndef _DHT11_H_
#define _DHT11_H_


/**********************************
包含头文件
**********************************/
#include <reg52.h>
#include <intrins.h>


/**********************************
重定义关键词
**********************************/
#define DHT11_NOACK 0
#define DHT11_ACK   1
#define DHT11_STATUS_REG_W 0x06
#define DHT11_STATUS_REG_R 0x07
#define DHT11_MEASURE_TEMP 0x03
#define DHT11_MEASURE_HUMI 0x05
#define DHT11_RESET        0x1e


/**********************************
PIN口定义
**********************************/
sbit DHT11_SCK = P1^3;	   //DHT11时钟引脚P1.0
sbit DHT11_DATA = P1^2;	   //DHT11数据引脚P1.1


/**********************************
变量定义
**********************************/
enum {TEMP, HUMI};

typedef union            //定义共用同类型  
{
	unsigned int i;
	float f;
}value;


/**********************************
函数声明
**********************************/
void DHT11_Init(void);             													//DHT11连接复位函数
void Dht11_Get_Temp_Humi_Value(float *temp, float *humi); //DHT11获取温湿度值函数


#endif
