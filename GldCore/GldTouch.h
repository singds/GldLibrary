#ifndef GLDTOUCH_H_INCLUDED
#define GLDTOUCH_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldHal/GldHalTouch.h"
#include "GldMisc/GldCoord.h"
#include "GldCore/GldObj.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES
extern gldhaltouch_t *gldtouch_Touch;
extern gldobj_t *gldtouch_ObjTocco;

//============================================================ GLOBAL PROTOTYPES
void gldtouch_Init (gldhaltouch_t *touch);
void gldtouch_Task (void);
bool gldtouch_GetCoords (gldcoord_Point_t *pos);

void gldtouch_NotifyDestroy (gldobj_t *obj);
void gldtouch_StartDrag (gldobj_t *obj);

#endif /* GLDTOUCH_H_INCLUDED */

