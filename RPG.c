// RPG.c

// Charlie Tejano
#include "stm32f4xx.h"
#include "RPG.h"

volatile int32_t rotary_count = 0;
volatile int8_t rotary_dir = rotary_dir_none;
volatile uint8_t g_estop_flag = 0;

#define ROTARY_OUTA_PIN  1   // PB1
#define ROTARY_OUTB_PIN  0   // PB0
#define ROTARY_SW_PIN   15   // PB15

void rotary_Init(void){

    // Enable GPIOB and SYSCFG clocks
    RCC->AHB1ENR |= (1u<<1);
    RCC->APB2ENR |= (1u<<14); // SYSCFG clock

    // PB0, PB1, PB15 = input mode
    GPIOB->MODER &= ~((3u<<(0*2)) |// 2-bits per pin
                      (3u<<(1*2)) |
                      (3u<<(15*2)));

    // Pull-up on PB0, PB1, PB15
    GPIOB->PUPDR &= ~((3u<<(0*2)) | // Clear pull-up
                      (3u<<(1*2)) |
                      (3u<<(15*2)));

    GPIOB->PUPDR |=  ((1u<<(0*2)) | // Set
                      (1u<<(1*2)) |
                      (1u<<(15*2)));

    // EXTI1 mapped to PB1
    SYSCFG->EXTICR[0] &= ~(15u<<4); // Clear EXTI1 mapping
    SYSCFG->EXTICR[0] |=  (1u<<4); // Bits [7:4]

    // Unmask EXTI1
    EXTI->IMR  |= (1u<<1);

    // Trigger on rising edge only
    EXTI->RTSR |= (1u<<1);
    EXTI->FTSR &= ~(1u<<1);

    // Clear pending flag
    EXTI->PR = (1u<<1);

    //EXTI15 to PortB
    //EXTICR[3] handles pins 12-15
    SYSCFG->EXTICR[3] &= ~(0xFu<<12);
    SYSCFG->EXTICR[3] |=  (0x1u<<12);

    // Trigger on falling edge
    EXTI->FTSR |= (1u<<15);
    EXTI->RTSR &= ~(1u<<15);

    // Unmask EXTI15
    EXTI->IMR |= (1u<<15);

    // Clear pending
    EXTI->PR = (1u<<15);

    NVIC_ClearPendingIRQ(EXTI1_IRQn);//Clear pending interrupt
    NVIC_SetPriority(EXTI15_10_IRQn, 1);//Higher priority than RPG rotat.
    NVIC_EnableIRQ(EXTI15_10_IRQn);//Enable interrupt

    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    NVIC_SetPriority(EXTI1_IRQn, 2);
    NVIC_EnableIRQ(EXTI1_IRQn);
}


//EXTI15
void EXTI15_10_IRQHandler(void){
    if (EXTI->PR & (1u<<15)){
        g_estop_flag = 1; // Stop motor
        EXTI->PR = (1u<<15); // Clear pending register
    }
}

//EXTI1 interrupt handler
void EXTI1_IRQHandler(void){

    if (EXTI->PR & (1u<<1)){ // Pending interrupt

        // OUTB decides direction
        if (GPIOB->IDR & (1u<<0))
        {
            rotary_count--; // If OUTB is high then count down
            rotary_dir = rotary_dir_ccw;
        }
        else
        {
            rotary_count++; // If OUTB is low then count up
            rotary_dir = rotary_dir_cw;
        }

        EXTI->PR = (1u<<1); // Clear pending interrupt
    }
}

//Get rotary count
int32_t rotaryGetCount(void){
    int32_t count;
    __disable_irq(); // Disable interrupts
    count = rotary_count;
    __enable_irq(); // Enable
    return count; // Return a number
}

//Get rotary direction
int8_t rotaryGetDir(void){
    int8_t dir;
    __disable_irq(); // Disable interrupts
    dir = rotary_dir;
    __enable_irq(); // Enable
    return dir;
}

uint8_t rotaryBtnPress(void){
    // Active low
    return !(GPIOB->IDR & (1u<<15)); // When btn is pressed, reverse direction
}

//Reset rotary encoder
void rotaryReset(void){
    __disable_irq();
    rotary_count = 0; // Reset
    rotary_dir = rotary_dir_none; // No direction
    __enable_irq(); // Enable
}

//Get rotary turns
float rotaryGetTurns(uint32_t detents_per_turn) { // Count turns
    int32_t count;

    if (detents_per_turn == 0u){ // If rotary is turned the opposite dir.
        return 0.0f; // Return 0.0f
    }
    __disable_irq();
    count = rotary_count;
    __enable_irq();

    return ((float)count) / ((float)detents_per_turn); // Return turns
}