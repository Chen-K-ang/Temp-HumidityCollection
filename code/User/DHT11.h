/**********************************
���ߣ���Ƭ�����ֲ�
��վ��https://www.mcuclub.cn/
**********************************/


#ifndef _DHT11_H_
#define _DHT11_H_


/**********************************
����ͷ�ļ�
**********************************/
#include <reg52.h>
#include <intrins.h>


/**********************************
�ض���ؼ���
**********************************/
#define DHT11_NOACK 0
#define DHT11_ACK   1
#define DHT11_STATUS_REG_W 0x06
#define DHT11_STATUS_REG_R 0x07
#define DHT11_MEASURE_TEMP 0x03
#define DHT11_MEASURE_HUMI 0x05
#define DHT11_RESET        0x1e


/**********************************
PIN�ڶ���
**********************************/
sbit DHT11_SCK = P1^3;	   //DHT11ʱ������P1.0
sbit DHT11_DATA = P1^2;	   //DHT11��������P1.1


/**********************************
��������
**********************************/
enum {TEMP, HUMI};

typedef union            //���干��ͬ����  
{
	unsigned int i;
	float f;
}value;


/**********************************
��������
**********************************/
void DHT11_Init(void);             													//DHT11���Ӹ�λ����
void Dht11_Get_Temp_Humi_Value(float *temp, float *humi); //DHT11��ȡ��ʪ��ֵ����


#endif
