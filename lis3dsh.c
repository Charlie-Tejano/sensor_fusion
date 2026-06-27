// lis3dsh.c

// Charlie Tejano
#include "lis3dsh.h"
#include "stm32f4xx.h"
#include <stdint.h>

// LIS3DSH Register Addresses
#define CTRL_REG4 0x20u
#define CTRL_REG5 0x24u
#define OUT_X_L   0x28u
#define OUT_X_H   0x29u
#define OUT_Y_L   0x2Au
#define OUT_Y_H   0x2Bu
#define OUT_Z_L   0x2Cu
#define OUT_Z_H   0x2Du

// SPI1 Command, read and write
#define SPI_READ  0x80u

/***LIS3DSH Sensitivity for acceleration in g/LSB \n
 **conversion factor for 6-bit data, 0.06 mg/LSB = 0.00006 g/LSB
*/
#define SPI_SENSITIVITY (0.06f / 1000.0f)

// PE3 - Chip-Select for the LIS3DSH
static void cs_low(void)
{
    GPIOE->BSRR = (1u << (3u + 16u)); // Set PE3 low
}

// Set PE3
static void cs_high(void)
{
    GPIOE->BSRR = (1u << 3u); // Set PE3 high
}

// 8-bit SPI transfer
static uint8_t spi1_transfer(uint8_t data)
{
    while (!(SPI1->SR & (1u << 1))) {} // TXE transmit, wait until empty
    SPI1->DR = out;

    while (!(SPI1->SR & (1u << 0))) {} // Wait RXNE receive not empty
    return (uint8_t)SPI1->DR; // Distribute received data
}

// Write a byte to the LIS3DSH register
static void lis3dsh_write(uint8_t reg, uint8_t data)
{
    cs_low();
    spi1_transfer(reg & 0x7Fu); // Clear the R/W bit for write operation
    spi1_transfer(value);
    cs_high();
}

static uint8_t lis3dsh_read(uint8_t reg)
{
    uint8_t data;
    cs_low();
    spi1_transfer(reg | SPI_READ); // Set the R/W bit for read operation
    data = spi1_transfer(0xFFu); // Send dummy byte to receive data
    cs_high();
    return data;
}

// Line 67, finally enable clocks for the LIS3DSH and SPI1
RCC->AHB1ENR |= (1u << 0) | (1u << 4);
RCC->APB2ENR |= (1u << 12); // SPI1 clock enable

GPIOA->MODER &= ~((3u << (5*2)) | (3u << (6*2)) | (3u << (7*2))); // Clear PA5, PA6, PA7
GPIOA->MODER |= ((2u << (5*2)) | (2u << (6*2)) | (2u << (7*2))); // Alternate function mode (10)

