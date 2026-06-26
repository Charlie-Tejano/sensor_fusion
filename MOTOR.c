// MOTOR.c

// Lab 5

//Charlie Tejano
#include "MOTOR.h"

//module variables
volatile uint8_t g_motorDuty = 0;
volatile MotorState_t g_motorState = MOTOR_STOPPED;

//helper functions
void MOTOR_GPIO_Init(void);
void MOTOR_TIM4_PWM_Init(void);
void MOTOR_UpdatePWM(uint8_t duty_percent);

// Initializes GPIO + PWM for DC motor
void MOTOR_Init(void)
{
    MOTOR_GPIO_Init();
    MOTOR_TIM4_PWM_Init();

    // Safe startup condition required by lab:
    // motor stopped, duty cycle = 0%
    MOTOR_Stop();
    MOTOR_SetDutyCycle(0);
}

// Initialize GPIO pins
void MOTOR_GPIO_Init(void) {

    //Enable GPIOD clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;

    // PD12 as general-purpose output

    // PD13 as general-purpose output
    GPIOD->MODER &= ~((3u << (MOTOR_DIRA_PIN * 2)) |
                      (3u << (MOTOR_DIRB_PIN * 2)));

    GPIOD->MODER |=  ((1u << (MOTOR_DIRA_PIN * 2)) |
                      (1u << (MOTOR_DIRB_PIN * 2)));

    // Push-pull
    GPIOD->OTYPER &= ~((1u << MOTOR_DIRA_PIN) |
                       (1u << MOTOR_DIRB_PIN));

    // Medium/high speed
    GPIOD->OSPEEDR &= ~((3u << (MOTOR_DIRA_PIN * 2)) |
                        (3u << (MOTOR_DIRB_PIN * 2)));

    GPIOD->OSPEEDR |=  ((2u << (MOTOR_DIRA_PIN * 2)) |
                        (2u << (MOTOR_DIRB_PIN * 2)));

    // No pull-up/pull-down
    GPIOD->PUPDR &= ~((3u << (MOTOR_DIRA_PIN * 2)) |
                      (3u << (MOTOR_DIRB_PIN * 2)));

    GPIOD->MODER &= ~(3u << (MOTOR_PWM_PIN * 2));
    GPIOD->MODER |=  (2u << (MOTOR_PWM_PIN * 2));

    GPIOD->OTYPER &= ~(1u << MOTOR_PWM_PIN); // Push-pull

    GPIOD->OSPEEDR &= ~(3u << (MOTOR_PWM_PIN * 2));
    GPIOD->OSPEEDR |=  (2u << (MOTOR_PWM_PIN * 2)); // Medium/high speed

    GPIOD->PUPDR &= ~(3u << (MOTOR_PWM_PIN * 2)); // No pull

    GPIOD->AFR[1] &= ~(15u << ((MOTOR_PWM_PIN - 8u) * 4u));
    GPIOD->AFR[1] |=  (2u  << ((MOTOR_PWM_PIN - 8u) * 4u));
}

// Initialize TIM4 CH4 for PWM on PD15
void MOTOR_TIM4_PWM_Init(void) {

    // Enable TIM4 clock
    RCC->APB1ENR |= (1u << 2);
    (void)RCC->APB1ENR; // Force bus to update

    // Stop timer during configuration
    TIM4->CR1 = 0;

    // Prescaler and auto-reload
    TIM4->PSC = 15; // Prescaler value
    TIM4->ARR = 999; // Auto-reload value

    // Start with 0% duty cycle
    TIM4->CCR4 = 0;

    // PWM Mode 1 on Channel 4
    TIM4->CCMR2 &= ~(7u << 12);
    TIM4->CCMR2 |=  (6u << 12); // PWM mode 1
    TIM4->CCMR2 |=  (1u << 11); // Preload Enable

    // Enable CH4 output
    TIM4->CCER |= (1u << 12);

    // Enable auto-reload preload
    TIM4->CR1 |= (1u << 7);

    // Force update event to load registers
    TIM4->EGR |= (1u << 0);

    // Enable timer
    TIM4->CR1 |= (1u << 0);
}

// Internal helper to update PWM
void MOTOR_UpdatePWM(uint8_t duty_percent) {
    uint32_t ccr;

    if (duty_percent > 100u) { // Cap at 100%
        duty_percent = 100u;
    }

    // CCR = duty% of (ARR+1)
    uint32_t arr = TIM4->ARR;
    ccr = ((uint32_t)(arr + 1u) * duty_percent) / 100u; // 1000 steps

    if (ccr > arr) { // Limit CCR to ARR
        ccr = arr;
    }

    TIM4->CCR4 = ccr;
}

// Set duty cycle from 0 to 100%
void MOTOR_SetDutyCycle(uint8_t duty_percent) {
    if (duty_percent > 100u) { // Cap at 100%
        duty_percent = 100u;
    }

    g_motorDuty = duty_percent; // Update at each increment or decrement
    MOTOR_UpdatePWM(g_motorDuty); // Update PWM output

    // If duty cycle becomes 0, treat as stopped
    if (g_motorDuty == 0u) {
        g_motorState = MOTOR_STOPPED;

        // Duty Cycle = 0%, DIRA=0, DIRB=0
        GPIOD->BSRR = (1u << (MOTOR_DIRA_PIN + 16u)) |
                      (1u << (MOTOR_DIRB_PIN + 16u));
    }
}

// Get duty cycle
uint8_t MOTOR_GetDutyCycle(void) {
    return g_motorDuty;
}

// Motor forward

// Forward rotation - DIRA=1, DIRB=0
void MOTOR_Forward(void) {

    // If duty is zero, keep motor stopped
    if (g_motorDuty == 0u) {
        MOTOR_Stop();
        return;
    }

    // DIRA = 1, DIRB = 0
    GPIOD->BSRR = (1u << MOTOR_DIRA_PIN) |
                  (1u << (MOTOR_DIRB_PIN + 16u));

    g_motorState = MOTOR_FORWARD;
}

// Backward rotation - DIRA=0, DIRB=1
void MOTOR_Backward(void) {

    // If duty is zero, keep motor stopped
    if (g_motorDuty == 0u) {
        MOTOR_Stop();
        return;
    }

    // DIRA = 0, DIRB = 1
    GPIOD->BSRR = (1u << (MOTOR_DIRA_PIN + 16u)) |
                  (1u << MOTOR_DIRB_PIN);

    g_motorState = MOTOR_BACKWARD;
}

// Brake
void MOTOR_Stop(void) {

    // DIRA = 0, DIRB = 0
    GPIOD->BSRR = (1u << (MOTOR_DIRA_PIN + 16u)) |
                  (1u << (MOTOR_DIRB_PIN + 16u));

    MOTOR_UpdatePWM(0);
    g_motorDuty  = 0;
    g_motorState = MOTOR_STOPPED;
}

// Emergency stop
void MOTOR_EStop(void) {
    MOTOR_Stop();
}

// Set state directly
void MOTOR_SetState(MotorState_t state) {
    switch (state) {
        case MOTOR_FORWARD:
            MOTOR_Forward();
            break;

        case MOTOR_BACKWARD:
            MOTOR_Backward();
            break;

        case MOTOR_STOPPED:
        default:
            MOTOR_Stop();
            break;
    }
}

// Get current state
MotorState_t MOTOR_GetState(void) {
    return g_motorState;
}

void MOTOR_IncreaseDuty_5(void) {
    if (g_motorDuty <= 95u) {
        MOTOR_SetDutyCycle((uint8_t)(g_motorDuty + 5u)); // Increase by 5%
    }
    else {
        MOTOR_SetDutyCycle(100u); // Cap at 100%
    }
}

// Decrease duty cycle by 5%
void MOTOR_DecreaseDuty_5(void) {
    if (g_motorDuty >= 5u) {
        MOTOR_SetDutyCycle((uint8_t)(g_motorDuty - 5u)); // Decrease by 5%
    }
    else {
        MOTOR_SetDutyCycle(0u); // Cap at 0%
    }
}