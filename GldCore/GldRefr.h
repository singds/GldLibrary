#ifndef GLDREFR_H_INCLUDED
#define GLDREFR_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldMisc/GldCoord.h"
#include "GldCore/GldObj.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES
extern gldobj_t *gldrefr_RootObj;
extern gldobj_t *gldrefr_UserLayer;
extern gldobj_t *gldrefr_SystemLayer;

//============================================================ GLOBAL PROTOTYPES
void gldrefr_InvalidateArea (gldcoord_Area_t *area);

void gldrefr_Init (void);
void gldrefr_Task (void);
int64_t gldrefr_GetTotalInvalidArea (void);

#endif /* GLDREFR_H_INCLUDED */

