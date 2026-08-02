// ultrasonic.h
// Charlie Tejano

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f4xx.h"
#include <stdint.h>

// Function prototypes
void  ultrasonicInit(void);
float ultrasonic_GetDistance_cm(void);

// Borrow the global variable from ultrasonic.c to be used in main.c
extern volatile float g_distance_cm;

#endif