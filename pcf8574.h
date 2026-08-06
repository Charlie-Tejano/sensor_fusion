// pcf8574.h
// Charlie Tejano
#ifndef PCF8574_H
#define PCF8574_H

#include "stm32f4xx.h"
#include <stdint.h>

#define PCF8574_ADDR 0x27u // I2C address of the PCF8574

// Prototypes
void pcf8574_init(void);
void pcf8574_placeCursor(uint8_t line);
void pcf8574_writeChar(const char *c);

#endif