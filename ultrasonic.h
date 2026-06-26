// ultrasonic.h
// Charlie Tejano

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f4xx.h"
#include <stdint.h>

void  ultrasonicInit(void);
float ultrasonic_GetDistance_cm(void);

extern volatile float g_distance_cm;

#endif