#ifndef GLDSIMULATOR_H_INCLUDED
#define GLDSIMULATOR_H_INCLUDED

//===================================================================== INCLUDES
#include <stdlib.h>
#include <stdint.h>

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
#ifndef __cplusplus
void gldsimulator_InvalidateArea (int16_t x0, int16_t y0, int16_t x_size, int16_t y_size);
bool gldsimulator_GetTouch (int16_t *x_pos, int16_t *y_pos);
#endif

#endif /* GLDSIMULATOR_H_INCLUDED */
