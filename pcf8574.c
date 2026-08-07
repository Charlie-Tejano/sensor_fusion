// pcf8574.c
// Charlie Tejano
#include "pcf8574.h"
#include "LCD.h" // Overlap with LCD.h for LCD command definitions
#include <stdint.h>

#define I2C_TIMEOUT 20000u // 20,000 cycles timeout for I2C operations

// Initialize general purpose I/O expander PCF8574
static void i2c1_init(void)
{
    RCC->AHB1ENR |= (1u << 1);
    RCC->APB1ENR |= (1u << 21);

    // PB7 & PB8 in alternate-function mode (10)
    GPIOB->MODER &= ~((3u << (7u*2u)) | (3u << (8u*2u)));
    GPIOB->MODER |=  ((2u << (7u*2u)) | (2u << (8u*2u)));

    // Open-drain (01); required for I2C
    GPIOB->OTYPER |= ((1u << 7u) | (1u << 8u));

    // Pull-up (01)
    GPIOB->PUPDR &= ~((3u << (7u*2u)) | (3u << (8u*2u)));
    GPIOB->PUPDR |= ((1u << (7u*2u)) | (1u << (8u*2u)));

    // High speed (11)
    GPIOB->OSPEEDR |= ((3u << (7u*2u)) | (3u << (8u*2u)));

    // AF4 = I2C1. PB7 sits in AFRL, PB8 in AFRH
    GPIOB->AFR[0] &= ~(0xFu << (7u*4u));
    GPIOB->AFR[0] |= (0x4u << (7u*4u));
    GPIOB->AFR[1] &= ~(0xFu << ((8u-8u)*4u));
    GPIOB->AFR[1] |= (0x4u << ((8u-8u)*4u));

    I2C1->CR1 |= (1u << 15); // If BUSY flag is set and glitch on the I2C bus, reset the I2C peripheral
    I2C1->CR1 &= ~(1u << 15);

    I2C1->CR2 = 16u; // pclk1 = 16 MHz
    I2C1->CCR = 80u; // CCR = 5000 ns / 62.5 ns = 80
    I2C1->TRISE = 17u; // T_rise = 1000 ns / (62.5 ns) + 1 = 17

    I2C1->CR1 |= (1u << 0); // Enable I2C1
}

static void i2c1_stop(void) 
{
    uint32_t timeout = I2C_TIMEOUT;
    I2C1->CR1 |= (1u << 9); // Generate STOP condition while it's still pending
    while (!(I2C1->CR1 & (1u << 9))) {
        if (--timeout == 0u) {
            return;
        }
    }
}

// Write a byte to the P0-P7 pins of the PCF8574
void pcf8574_write(uint8_t data)
{
    uint32_t timeout;

    timeout = I2C_TIMEOUT;
    while (I2C1->SR2 & (1u << 1)) { // BUSY flag
        if (--timeout == 0u) {
            return;
        }
    }

    I2C1->CR1 |= (1u << 8); // START
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (1u << 0))) { // Start-bit sent
        if (--timeout == 0u) {
            i2c1_stop();
            return;
        }
    }

    I2C1->DR = (uint8_t)(PCF8574_ADDR << 1); // Send address with write bit (0)
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (1u << 1))) {
        if (I2C1->SR1 & (1u << 10)) { // Check for NACK or No Acknowledge
            I2C1->SR1 &= ~(1u << 10);
            i2c1_stop();
            return;
        }
        if (--timeout == 0u)
        {
            i2c1_stop();
            return;
        }
    }

    (void)I2C1->SR1; // Clear ADDR flag by reading SR1 and SR2
    (void)I2C1->SR2;

    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (1u << 7))) { // Wait for TXE
        if (--timeout == 0u) {
            i2c1_stop();
            return;
        }
    } 
    I2C1->DR = data; // Send data byte

    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (1u << 7))) { // Wait for BTF or Byte Transfer Finished
        if (--timeout == 0u) {
            i2c1_stop();
            return;
        }
    }
    I2C1->DR = data; // Send data byte again to ensure it's sent

    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (1u << 2))) { // BTF flag set
        if (--timeout == 0u) {
            break;
        }
        i2c1_stop();
    }
}

// Strobe a nibble out to P4-P7 with EN high, then low
void pcf8574_strobe(uint8_t nibble, uint8_t rs)
{
    uint8_t out = (uint8_t)(nibble >> 4) | rs | PCF8574_BL;

    pcf8574_write(out);
    pcf8574_write((uint8_t)(out | PCF8574_EN));
    delay_us(1);
    pcf8574_write(out);
    delay_us(50);
}

// Upper-nibble & lower-nibble
void sendByte(uint8_t value, uint8_t rs)
{
    pcf8574_strobe((uint8_t)(value << 4), rs);
    pcf8574_strobe((uint8_t)(value & 0x0Fu), rs);
}

// Send instructions to the LCD1602
void pcf8574_sendInstr(uint8_t instr)
{
    sendByte(instr, 0u);
}

void pcf8574_clearDisplay(void)
{
    pcf8574_sendInstr(LCD_CLEAR_DISPLAY);
    delay_us(2000); // 2 milliseconds
}

// Setup for the second LCD simultaneously connected to the PCF8574 I/O expander
void pcf8574_init(void)
{
    i2c1_init();
    delay_ms(50); // Power-on reset delay

    // Initialize 8-bit mode three times ensuring synchronization
    pcf8574_strobe(0x03u, 0u);
    delay_us(5000);
    pcf8574_strobe(0x03u, 0u);
    delay_us(1000);
    pcf8574_strobe(0x03u, 0u);
    delay_us(1000);
    pcf8574_strobe(0x02u, 0u); // Set to 4-bit mode
    delay_us(1000);

    // Function set: 4-bit, 2 lines, 5x8 font
    pcf8574_sendInstr(LCD_FUNCTION_SET);
    pcf8574_sendInstr(LCD_DISPLAY_OFF);
    pcf8574_clearDisplay();
    pcf8574_sendInstr(LCD_ENTRY_MODE);
    pcf8574_sendInstr(LCD_DISPLAY_ON);
}

void pcf8574_placeCursor(uint8_t line)
{
    if (line == 1u) {
        pcf8574_sendInstr(LCD_LINE1_ADDR);
    } else if (line == 2u) {
        pcf8574_sendInstr(LCD_LINE2_ADDR);
    }
}

// Write a character to the LCD1602
void pcf8574_writeChar(const char *c)
{
    if (c == 0) {
        return;
    }
    sendByte((uint8_t)(*c), PCF8574_RS); // Send data with RS high
}

// Write a string to the LCD1602
void pcf8574_writeString(const char *str)
{
    int i = 0;

    while (str[i] != '\0') { // Loop until it reaches null terminator
        if (i == 16) {
            pcf8574_placeCursor(2);
        }
        if (i == 32) {
            break; // Limit to 32 characters; two lines worth of 16 characters each
        }
        pcf8574_writeChar(&str[i]);
        i++;
    }
}