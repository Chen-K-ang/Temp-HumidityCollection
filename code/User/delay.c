#include "delay.h"

void delay_ms(unsigned int times) //@12.000MHz
{
	unsigned char i, j;
	for (; times > 1; times --) {
		i = 2;
		j = 239;
		do {
			while (--j);
		} while (--i);
	}
}

void delay_10us(unsigned int times) //@12.000MHz
{
	unsigned char i;
	for (; times > 1; times --) {
		_nop_();
		i = 2;
		while (--i);
	}
}

