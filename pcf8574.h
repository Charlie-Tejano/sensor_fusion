// pcf8574.h
// Charlie Tejano
#ifndef PCF8574_H
#define PCF8574_H

#include "stm32f4xx.h"
#include <stdint.h>

/*** 
 ** I2C address of the PCF8574; Binary address: 0100111x,
    A2:A0 are high (111)
 */
#define PCF8574_ADDR 0x27u
#define PCF8574_RW (1u << 1)
#define PCF8574_RS (1u << 0)
#define PCF8574_EN (1u << 2)
#define PCF8574_BL (1u << 3)

// Prototypes
void pcf8574_init(void);
void pcf8574_write(uint8_t data); // Write a byte of data to the PCF8574
void pcf8574_placeCursor(uint8_t line);
void pcf8574_writeString(const char *str);
void pcf8574_sendInstr(uint8_t instr);
void pcf8574_strobe(uint8_t nibble, uint8_t rs);
void sendByte(uint8_t value, uint8_t rs);
void pcf8574_clearDisplay(void);
void delay_us(uint32_t us); // Delay in microseconds

#endif