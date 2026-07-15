// main.c

// Charlie Tejano
#include "LCD.h"
#include "stm32f4xx.h"
#include "MOTOR.h"
#include "ultrasonic.h"
#include "lis3dsh.h"
#include <stdint.h>
#include <stdio.h>

#define SPI_DEBUG 1 // SPI debug flag, set to 1 to enable debug output

// Distance, duty mapping
static uint8_t DIST_to_duty(float dist_cm)
{
    if (dist_cm < 10.0f) { // Read more than 10 cm
        return 20u;
    }
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
    MOTOR_Init();
		ultrasonicInit();

    // Debug mode for SPI communication with LIS3DSH
    #if SPI_DEBUG
    LCD_placeCursor(1);
    LCD_printString("SPI Debug Mode...");

    while(1) {
        float x_dir, y_dir, z_dir;
        char dbg[17];

        lis3dsh_read_dir(&x_dir, &y_dir, &z_dir); // Read X, Y, Z axis data from LIS3DSH accelerometer
        
        snprintf(dbg, sizeof(dbg), "X%5.2f Y%5.2f", x_dir, y_dir); // Display X and Y axis data on LCD
        LCD_placeCursor(2);
        LCD_printString(dbg);
        delay_ms(100); // Update every 100 ms
    }
    #endif

    while(1) {
        uint8_t id = lis3dsh_whoami(); // Read WHO_AM_I register from LIS3DSH
        char dbg[17];

        snprintf(dbg, sizeof(dbg), "WHO_AM_I: 0x%02X", id); // Display WHO_AM_I value on LCD
        LCD_placeCursor(1);
        LCD_printString(dbg);
        delay_ms(100); // Update every 100 ms
    }
    
    MOTOR_Stop(); // Stop motor right away

    LCD_placeCursor(1);
    LCD_printString("Digital Sensor Fusion ");
    LCD_placeCursor(2);
    LCD_printString("Initializing... ");
    delay_ms(1000);

    // Start forward at 40% as our certain speed
    MOTOR_SetDutyCycle(40u);
    MOTOR_Forward();

    while (1) {
        float dist_cm = ultrasonic_GetDistance_cm(); // Get distance from ultrasonic sensor

        if (dist_cm <= 0.0f) { // Less than 0 cm is not valid
            delay_ms(50);
            continue;
        }

        uint8_t duty = DIST_to_duty(dist_cm);
        MOTOR_SetDutyCycle(duty); // Set motor duty cycle based on distance
        LCD_showUltrasonic(dist_cm, duty); // Show distance and duty cycle on LCD
        delay_ms(100);
    }
}