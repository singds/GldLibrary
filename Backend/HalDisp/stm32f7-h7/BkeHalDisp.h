#ifndef BKEHALDISP_H_INCLUDED
#define BKEHALDISP_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldHal/GldHalDisp.h"
#include "stm32h7xx_hal.h"

//====================================================================== DEFINES

typedef enum
{
    BKEHALDISP_EVENT_SET_VISIBLE_FRAME,
    BKEHALDISP_EVENT_SET_WORKING_FRAME,
} bkehaldisp_Event_e;

typedef struct
{
    DMA2D_HandleTypeDef *Dma2d; // periferica dma2d
    uintptr_t FbAddress[2]; // framebuffer pointer
    void (*FuncEvent) (gldhaldisp_t * disp, bkehaldisp_Event_e evt, void *arg);
} bkehaldisp_Init_t;

//============================================================= GLOBAL VARIABLES
extern bkehaldisp_Init_t bkehaldisp_Init;
extern gldhaldisp_t bkehaldisp_Display;

extern uint8_t bkehaldisp_VisibleFrame;
extern uint8_t bkehaldisp_WorkingFrame;

//============================================================ GLOBAL PROTOTYPES
int32_t bkehaldisp_InitModule (void);

void bkehaldisp_SetWorkingFrame (uint8_t num);
void bkehaldisp_SetVisibleFrame (uint8_t num);
void bkehaldisp_CleanFrame (uint8_t num);
void bkehaldisp_InvalidateFrame (uint8_t num);

#endif /* BKEHALDISP_H_INCLUDED */

