#ifndef HARDWARE_MOTOR_H
#define HARDWARE_MOTOR_H

#include <reg52.h>

sbit IN1 = P3^0;
sbit IN2 = P3^1;
sbit ENA = P3^7;

void motor_init(void);

#endif
