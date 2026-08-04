// ultrasonic.h
// Charlie Tejano

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "stm32f4xx.h"
#include <stdint.h>

// automatic emergency braking
typedef enum {
    aeb_off = 0,
    aeb_on,
    aeb_fault
} aeb_state;

// Function to set the state of the automatic emergency braking system
void aeb_set_state(aeb_state state);

// Function prototypes
void  ultrasonicInit(void);
float ultrasonic_GetDistance_cm(void);

// Borrow the global variable from ultrasonic.c to be used in main.c
extern volatile float g_distance_cm;

#endif