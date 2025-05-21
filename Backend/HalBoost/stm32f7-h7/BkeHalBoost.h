#ifndef BKEGLDBOOST_H_INCLUDED
#define BKEGLDBOOST_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldHal/GldHalBoost.h"
#include "stm32h7xx_hal.h"

//====================================================================== DEFINES

typedef struct
{
    DMA2D_HandleTypeDef *Dma2d; // periferica dma2d
} bkehalboost_Init_t;

//============================================================= GLOBAL VARIABLES
extern bkehalboost_Init_t  bkehalboost_Init;
extern gldhalboost_t bkehalboost_Boost;

//============================================================ GLOBAL PROTOTYPES
int32_t bkehalboost_InitModule (void);

#endif /* BKEGLDBOOST_H_INCLUDED */

