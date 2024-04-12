/**********************************
����ͷ�ļ�
**********************************/
#include "dht11.h"


/**********************************
��������
**********************************/
/****
*******DHT11д�ֽں���
*****/
char s_write_byte(unsigned char value)
{
	unsigned char i, error = 0;
	for (i = 0x80; i>0; i >>= 1)            //��λΪ1��ѭ������  
	{
		if (i&value) DHT11_DATA = 1;          //��Ҫ���͵������룬���Ϊ���͵�λ  
		else DHT11_DATA = 0;
		DHT11_SCK = 1;
		_nop_(); _nop_(); _nop_();        		//��ʱ3us   
		DHT11_SCK = 0;
	}
	DHT11_DATA = 1;                         //�ͷ�������  
	DHT11_SCK = 1;
	error = DHT11_DATA;                     //���Ӧ���źţ�ȷ��ͨѶ����  
	_nop_(); _nop_(); _nop_();
	DHT11_SCK = 0;
	DHT11_DATA = 1;
	return error;                     			//error=1 ͨѶ����  
}

/****
*******DHT11���ֽں���
*****/
char s_read_byte(unsigned char ack)
{
	unsigned char i, val = 0;
	DHT11_DATA = 1;                         //�ͷ�������  
	for (i = 0x80; i>0; i >>= 1)            //��λΪ1��ѭ������  
	{
		DHT11_SCK = 1;
		if (DHT11_DATA) val = (val | i);      //��һλ�����ߵ�ֵ   
		DHT11_SCK = 0;
	}
	DHT11_DATA = !ack;                      //�����У�飬��ȡ������ͨѶ��  
	DHT11_SCK = 1;
	_nop_(); _nop_(); _nop_();         	 		//��ʱ3us   
	DHT11_SCK = 0;
	_nop_(); _nop_(); _nop_();
	DHT11_DATA = 1;                         //�ͷ�������  
	return val;
}

/****
*******DHT11�������亯��
*****/
void s_transstart(void)
{
	DHT11_DATA = 1; DHT11_SCK = 0;           //׼��  
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
*******DHT11��ʼ������
*****/
void DHT11_Init(void)
{
	unsigned char i;
	DHT11_DATA = 1; DHT11_SCK = 0;          //׼��  
	for (i = 0; i<9; i++)                  	//DATA���ָߣ�SCKʱ�Ӵ���9�Σ������������䣬ͨѸ����λ  
	{
		DHT11_SCK = 1;
		DHT11_SCK = 0;
	}
	s_transstart();                   			//��������  
}

/****
*******DHT11��ʪ�ȼ�⺯��
*****/
char s_measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
{
	unsigned error = 0;
	unsigned int i;

	s_transstart();                   			//��������  
	switch (mode)                      			//ѡ��������  
	{
	case TEMP: error += s_write_byte(DHT11_MEASURE_TEMP); break;          //�����¶�  
	case HUMI: error += s_write_byte(DHT11_MEASURE_HUMI); break;          //����ʪ��  
	default: break;
	}
	for (i = 0; i<65535; i++) if (DHT11_DATA == 0) break;         //�ȴ���������  
	if (DHT11_DATA) error += 1;                              			// �����ʱ��������û�����ͣ�˵����������   
	*(p_value) = s_read_byte(DHT11_ACK);           								//����һ���ֽڣ����ֽ� (MSB)  
	*(p_value + 1) = s_read_byte(DHT11_ACK);          						//���ڶ����ֽڣ����ֽ� (LSB)  
	*p_checksum = s_read_byte(DHT11_NOACK);        								//read CRCУ����  
	return error;                   												// error=1 ͨѶ����  
}

/****
*******DHT11��ʪ��ֵ��ȱ任���¶Ȳ�������
*****/
void calc_sth10(float *p_humidity, float *p_temperature)
{
	const float C1 = -4.0;              // 12λʪ�Ⱦ��� ������ʽ  
	const float C2 = +0.0405;           // 12λʪ�Ⱦ��� ������ʽ  
	const float C3 = -0.0000028;        // 12λʪ�Ⱦ��� ������ʽ  
	const float T1 = +0.01;             // 14λ�¶Ⱦ��� 5V����  ������ʽ  
	const float T2 = +0.00008;          // 14λ�¶Ⱦ��� 5V����  ������ʽ  

	float rh = *p_humidity;             // rh:      12λ ʪ��   
	float t = *p_temperature;           // t:       14λ �¶�  
	float rh_lin;                     	// rh_lin: ʪ�� linearֵ  
	float rh_true;                    	// rh_true: ʪ�� tureֵ  
	float t_C;                        	// t_C   : �¶� ��  

	t_C = t*0.01 - 40;                  //�����¶�  
	rh_lin = C3*rh*rh + C2*rh + C1;     //���ʪ�ȷ����Բ���  
//	rh_true = (t_C - 25)*(T1 + T2*rh) + rh_lin;   //���ʪ�ȶ����¶������Բ���
	rh_true = rh_lin; 
	if (rh_true > 100)
		rh_true = 100;      //ʪ���������  
	if (rh_true < 0.1)
		rh_true = 0.1;      //ʪ����С����  

	*p_temperature = t_C;               //�����¶Ƚ��  
	*p_humidity = rh_true;              //����ʪ�Ƚ��  
}


/****
*******DHT11��ȡ��ʪ��ֵ����
*****/
void Dht11_Get_Temp_Humi_Value(float *temp, float *humi)
{
//	unsigned int temp, humi;
	unsigned char error;            		//���ڼ����Ƿ���ִ���
	unsigned char checksum;         		//CRC
	value humi_val, temp_val;        		//����������ͬ�壬һ������ʪ�ȣ�һ�������¶�

	error = 0;                       		//��ʼ��error=0����û�д���  
	error += s_measure((unsigned char*)&temp_val.i, &checksum, TEMP); //�¶Ȳ���  
	error += s_measure((unsigned char*)&humi_val.i, &checksum, HUMI); //ʪ�Ȳ���  
	if (error != 0) DHT11_Init();                 			//�����������ϵͳ��λ  
	else
	{
		humi_val.f = (float)humi_val.i;                   //ת��Ϊ������  
		temp_val.f = (float)temp_val.i;                   //ת��Ϊ������  
		calc_sth10(&humi_val.f, &temp_val.f);            	//�������ʪ�ȼ��¶�  
		*temp = temp_val.f;
		*humi = humi_val.f - 0.9;
	}
}

