// main.c

// Charlie Tejano
#include "LCD.h"
#include "pcf8574.h"
#include "stm32f4xx.h"
#include "MOTOR.h"
#include "ultrasonic.h"
#include "lis3dsh.h"
#include "RPG.h"
#include <stdint.h>
#include <stdio.h>

// Distance, duty mapping
static uint8_t DIST_to_duty(float dist_cm)
{
    // Measure distance and map to duty cycle
    if (dist_cm < 30.0f) {
        return 40u;
    }
    if (dist_cm < 60.0f) {
        return 60u;
    }
    return 80u; 
}

// Display distance and duty cycle on LCD
static void LCD_showUltrasonic(float dist_cm, uint8_t duty)
{
    char line1[17];
    char line2[17];
    snprintf(line1, sizeof(line1), "DIST:%6.1f cm ", dist_cm);
    snprintf(line2, sizeof(line2), "DUTY: %3u%%  FWD", (unsigned)duty);
    LCD_placeCursor(1);
    LCD_printString(line1);
    LCD_placeCursor(2);
    LCD_printString(line2);
}

// Main function
int main(void)
{
    LCD_port_init();
    LCD_init();
    pcf8574_init();
    lis3dsh_init();
    MOTOR_Init();
    ultrasonicInit();
    rotary_Init();
    MOTOR_Stop();
    MOTOR_SetDutyCycle(40u);
    MOTOR_Forward();
    
    while(1) {
        // Use e-stop flag to indicate a fault condition
        if (g_estop_flag) {
            aeb_set_state(aeb_fault);
            LCD_placeCursor(1);
            LCD_printString("AEB: Fault      ");
            LCD_placeCursor(2);
            LCD_printString("ESTOP...      ");
            delay_ms(25);
            continue;
        }

        pcf8574_placeCursor(1);
        pcf8574_writeString("Traction Control  ");
        pcf8574_placeCursor(2);
        pcf8574_writeString("Active            ");
        
        // Read distance from ultrasonic sensor
        float dist_cm = ultrasonic_GetDistance_cm();
        if (dist_cm <= 0.0f) { // Less than 0 cm is not valid
            delay_ms(50);
            continue;
        }
        
        // Emergency braking if distance is less than 10 cm
        if (dist_cm < 10.0f) {
            aeb_set_state(aeb_on);
            LCD_placeCursor(1);
            LCD_printString("AEB: Initiated  ");
            LCD_placeCursor(2);
            LCD_printString("DIST: <10cm   ");
            continue;
            }

            // If distance is greater than 10 cm, resume forward motion
            aeb_set_state(aeb_off);

        // Map distance to duty cycle and update motor speed
        uint8_t duty = DIST_to_duty(dist_cm);
        MOTOR_SetDutyCycle(duty);
        LCD_showUltrasonic(dist_cm, duty);
				delay_ms(25);
    }
}