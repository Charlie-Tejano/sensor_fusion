// lis3dsh.h
// LIS3DSH 3-axis accelerometer driver — bare-metal SPI1, no HAL

// Charlie Tejano
#ifndef LIS3DSH_H
#define LIS3DSH_H

#include <stdint.h>
#include "stm32f4xx.h"

// Prototypes
void LIS3DSH_Init(void);
void LIS3DSH_Read(float *x_dir, float *y_dir, float *z_dir);
void LIS3DSH_LowPower(void);
void LIS3DSH_WakeUp(void);

#endif // LIS3DSH_H