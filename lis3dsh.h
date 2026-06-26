// lis3dsh.h

// Charlie Tejano
#ifndef LIS3DSH_H
#define LIS3DSH_H

#include <stdint.h>
#include "stm32f4xx.h"

// g-direction helper
typedef enum {
    DIR_NONE = 0,
    DIR_POSITIVE_X,
    DIR_NEGATIVE_X,
    DIR_POSITIVE_Y,
    DIR_NEGATIVE_Y,
    DIR_POSITIVE_Z,
    DIR_NEGATIVE_Z
} LIS3DSH_Direction;

LIS3DSH_Direction LIS3DSH_Dir(void); // Returns the direction of the g-force detected from the acceerometer

// Prototypes
void LIS3DSH_Init(void);
void LIS3DSH_Read(float *x_dir, float *y_dir, float *z_dir);

#endif // LIS3DSH_H