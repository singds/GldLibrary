#ifndef GLDDISP_H_INCLUDED
#define GLDDISP_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldHal/GldHalDisp.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES
extern gldhaldisp_t *glddisp_Disp;

//============================================================ GLOBAL PROTOTYPES
void glddisp_Init (gldhaldisp_t * disp);
void glddisp_SetClipArea (gldhaldisp_t *disp, const gldcoord_Area_t *area);

#endif /* GLDDISP_H_INCLUDED */

