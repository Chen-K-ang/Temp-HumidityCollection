/**********************************
包含头文件
**********************************/
#include "dht11.h"


/**********************************
函数定义
**********************************/
/****
*******DHT11写字节函数
*****/
char s_write_byte(unsigned char value)
{
	unsigned char i, error = 0;
	for (i = 0x80; i>0; i >>= 1)            //高位为1，循环右移  
	{
		if (i&value) DHT11_DATA = 1;          //和要发送的数相与，结果为发送的位  
		else DHT11_DATA = 0;
		DHT11_SCK = 1;
		_nop_(); _nop_(); _nop_();        		//延时3us   
		DHT11_SCK = 0;
	}
	DHT11_DATA = 1;                         //释放数据线  
	DHT11_SCK = 1;
	error = DHT11_DATA;                     //检查应答信号，确认通讯正常  
	_nop_(); _nop_(); _nop_();
	DHT11_SCK = 0;
	DHT11_DATA = 1;
	return error;                     			//error=1 通讯错误  
}

/****
*******DHT11读字节函数
*****/
char s_read_byte(unsigned char ack)
{
	unsigned char i, val = 0;
	DHT11_DATA = 1;                         //释放数据线  
	for (i = 0x80; i>0; i >>= 1)            //高位为1，循环右移  
	{
		DHT11_SCK = 1;
		if (DHT11_DATA) val = (val | i);      //读一位数据线的值   
		DHT11_SCK = 0;
	}
	DHT11_DATA = !ack;                      //如果是校验，读取完后结束通讯；  
	DHT11_SCK = 1;
	_nop_(); _nop_(); _nop_();         	 		//延时3us   
	DHT11_SCK = 0;
	_nop_(); _nop_(); _nop_();
	DHT11_DATA = 1;                         //释放数据线  
	return val;
}

/****
*******DHT11启动传输函数
*****/
void s_transstart(void)
{
	DHT11_DATA = 1; DHT11_SCK = 0;           //准备  
	_nop_();
	DHT11_SCK = 1;
	_nop_();
	DHT11_DATA = 0;
	_nop_();
	DHT11_SCK = 0;
	_nop_(); _nop_(); _nop_();
	DHT11_SCK = 1;
	_nop_();
	DHT11_DATA = 1;
	_nop_();
	DHT11_SCK = 0;
}

/****
*******DHT11初始化函数
*****/
void DHT11_Init(void)
{
	unsigned char i;
	DHT11_DATA = 1; DHT11_SCK = 0;          //准备  
	for (i = 0; i<9; i++)                  	//DATA保持高，SCK时钟触发9次，发送启动传输，通迅即复位  
	{
		DHT11_SCK = 1;
		DHT11_SCK = 0;
	}
	s_transstart();                   			//启动传输  
}

/****
*******DHT11温湿度检测函数
*****/
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
	unsigned error = 0;
	unsigned int i;

	s_transstart();                   			//启动传输  
	switch (mode)                      			//选择发送命令  
	{
	case TEMP: error += s_write_byte(DHT11_MEASURE_TEMP); break;          //测量温度  
	case HUMI: error += s_write_byte(DHT11_MEASURE_HUMI); break;          //测量湿度  
	default: break;
	}
	for (i = 0; i<65535; i++) if (DHT11_DATA == 0) break;         //等待测量结束  
	if (DHT11_DATA) error += 1;                              			// 如果长时间数据线没有拉低，说明测量错误   
	*(p_value) = s_read_byte(DHT11_ACK);           								//读第一个字节，高字节 (MSB)  
	*(p_value + 1) = s_read_byte(DHT11_ACK);          						//读第二个字节，低字节 (LSB)  
	*p_checksum = s_read_byte(DHT11_NOACK);        								//read CRC校验码  
	return error;                   												// error=1 通讯错误  
}

/****
*******DHT11温湿度值标度变换及温度补偿函数
*****/
void calc_sth10(float *p_humidity, float *p_temperature)
{
	const float C1 = -4.0;              // 12位湿度精度 修正公式  
	const float C2 = +0.0405;           // 12位湿度精度 修正公式  
	const float C3 = -0.0000028;        // 12位湿度精度 修正公式  
	const float T1 = +0.01;             // 14位温度精度 5V条件  修正公式  
	const float T2 = +0.00008;          // 14位温度精度 5V条件  修正公式  

	float rh = *p_humidity;             // rh:      12位 湿度   
	float t = *p_temperature;           // t:       14位 温度  
	float rh_lin;                     	// rh_lin: 湿度 linear值  
	float rh_true;                    	// rh_true: 湿度 ture值  
	float t_C;                        	// t_C   : 温度 ℃  

	t_C = t*0.01 - 40;                  //补偿温度  
	rh_lin = C3*rh*rh + C2*rh + C1;     //相对湿度非线性补偿  
//	rh_true = (t_C - 25)*(T1 + T2*rh) + rh_lin;   //相对湿度对于温度依赖性补偿
	rh_true = rh_lin; 
	if (rh_true > 100)
		rh_true = 100;      //湿度最大修正  
	if (rh_true < 0.1)
		rh_true = 0.1;      //湿度最小修正  

	*p_temperature = t_C;               //返回温度结果  
	*p_humidity = rh_true;              //返回湿度结果  
}


/****
*******DHT11获取温湿度值函数
*****/
void Dht11_Get_Temp_Humi_Value(float *temp, float *humi)
{
//	unsigned int temp, humi;
	unsigned char error;            		//用于检验是否出现错误
	unsigned char checksum;         		//CRC
	value humi_val, temp_val;        		//定义两个共同体，一个用于湿度，一个用于温度

	error = 0;                       		//初始化error=0，即没有错误  
	error += s_measure((unsigned char*)&temp_val.i, &checksum, TEMP); //温度测量  
	error += s_measure((unsigned char*)&humi_val.i, &checksum, HUMI); //湿度测量  
	if (error != 0) DHT11_Init();                 			//如果发生错误，系统复位  
	else
	{
		humi_val.f = (float)humi_val.i;                   //转换为浮点数  
		temp_val.f = (float)temp_val.i;                   //转换为浮点数  
		calc_sth10(&humi_val.f, &temp_val.f);            	//修正相对湿度及温度  
		*temp = temp_val.f;
		*humi = humi_val.f - 0.9;
	}
}

