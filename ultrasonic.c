// ultrasonic.c

#include "ultrasonic.h"
#include "stm32f4xx.h"
#include <stdint.h>

// State variables
volatile uint8_t s_edge_state = 0u;
volatile uint32_t s_rise_tick = 0u;

volatile float g_distance_cm = 0.0f;

void ultrasonic_TIM3(void);
void ultrasonicInit(void)
{
	
RCC->AHB1ENR |= (1u << 1);
	
// PB4 - TRIG - AF2 (TIM3_CH1)
    GPIOB->MODER &= ~(3u << (4u*2u));
    GPIOB->MODER |= (2u << (4u*2u));
	
    GPIOB->OTYPER &= ~(1u << 4u);
	
    GPIOB->PUPDR &= ~(3u << (4u*2u));
	
    GPIOB->AFR[0] &= ~(0xFu << (4u*4u));
    GPIOB->AFR[0] |=  (0x2u << (4u*4u));

// PB5 - ECHO - AF2 (TIM3_CH2)
    GPIOB->MODER &= ~(3u << (5u*2u));
    GPIOB->MODER |= (2u << (5u*2u));
	
    GPIOB->PUPDR &= ~(3u << (5u*2u));
	
    GPIOB->AFR[0] &= ~(0xFu << (5u*4u)); // clear 4-bits
    GPIOB->AFR[0] |= (2u << (5u*4u));

    ultrasonic_TIM3();
}

float ultrasonic_GetDistance_cm(void) // Distance in cm
{
	return g_distance_cm;
	}

void ultrasonic_TIM3(void) // Timer 3 Init
{
RCC->APB1ENR |= (1u << 1);
	
TIM3->CR1 = 0x0000u; // Clear
	
TIM3->PSC = 159u;
TIM3->ARR = 74999u;
	
TIM3->CCMR1 &= ~(0x00FFu);
TIM3->CCMR1 |= (6u << 4);
TIM3->CCMR1 |= (1u << 3);
	
TIM3->CCR1 = 1u;
	
TIM3->CCER &= ~(1u << 1);
TIM3->CCER |= (1u << 0);
	
TIM3->CCMR1 &= ~(0xFF00u);

TIM3->CCMR1 |= (1u << 8u); // CC2S = 01
    TIM3->CCER &= ~((1u << 5u) | (1u << 7u)); // CC2P=0, CC2NP=0
    TIM3->CCER |= (1u << 4u); // CC2E

    TIM3->CR1 |= (1u << 7u); // ARPE

    TIM3->SR  = 0x0000u;
    TIM3->EGR = (1u << 0u); // UG
    TIM3->SR  = 0x0000u;

    TIM3->DIER |= (1u << 2u); // CC2IE

    NVIC_SetPriority(TIM3_IRQn, 1u);
    NVIC_EnableIRQ(TIM3_IRQn);

    TIM3->CR1 |= (1u << 0u); // Channel enable
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1u << 6)) { // Wait for capture
        TIM3->SR &= ~(1u << 6);
    }
        if (s_edge_state == 0u) { // Rising edge detected
            s_rise_tick = TIM3->CCR2; 

            TIM3->CCER |=  (1u << 5u); // Enable Capture
            TIM3->CCER &= ~(1u << 7u); // Disable Capture
            s_edge_state = 1u;
        } else {
            uint32_t fall_tick = TIM3->CCR2; // Falling edge detected
            uint32_t pulse_ticks = fall_tick - s_rise_tick; // Pulse width

            if ((pulse_ticks >= 12u) && (pulse_ticks <= 3800u)) { // Read only valid pulses
                g_distance_cm = (float)pulse_ticks * 0.1715f; // Convert to cm
            }

            TIM3->CCER &= ~(1u << 5u); // Disable Capture
            s_edge_state = 0u; // Reset edge state
        }
}

