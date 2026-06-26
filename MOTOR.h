// MOTOR.h

//Charlie Tejano

// Lab 5 DC Motor Control
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx.h"
#include <stdint.h>


// PWM1  -> PD15
// DIR1A -> PD13
// DIR1B -> PD12

#define MOTOR_PWM_PIN    15
#define MOTOR_DIRA_PIN   13
#define MOTOR_DIRB_PIN   12

// Motor state definitions
typedef enum {
    MOTOR_STOPPED = 0,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
} MotorState_t;

// API
void MOTOR_Init(void);

void MOTOR_SetDutyCycle(uint8_t duty_percent);
uint8_t MOTOR_GetDutyCycle(void);

void MOTOR_Forward(void);
void MOTOR_Backward(void);
void MOTOR_Stop(void);
void MOTOR_EStop(void);

void MOTOR_SetState(MotorState_t state);
MotorState_t MOTOR_GetState(void);

// Optional helpers for the lab
void MOTOR_IncreaseDuty_5(void);
void MOTOR_DecreaseDuty_5(void);

#endif