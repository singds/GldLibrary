#ifndef OBJTEMPLATE_H_INCLUDED
#define OBJTEMPLATE_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"

//====================================================================== DEFINES
#define OBJTEMPLATE_DEF(obj, parent) \
    GLDASSETS_DEF (objtemplate_t, thisTemplate, "ObjTemplate", obj, parent)
    
typedef struct
{
    gldobj_t Obj; // root obj
    
    uint8_t DummyData;
} objtemplate_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *objtemplate_Create (uint32_t memSize);
void objtemplate_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
void objtemplate_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext);

#endif /* OBJTEMPLATE_H_INCLUDED */
