#ifndef GLDCUSTOM_H_INCLUDED
#define GLDCUSTOM_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"

//====================================================================== DEFINES
#define OBJCUSTOM_DEF(obj, parent) \
    GLDASSETS_DEF (objcustom_t, thisCustom, "ObjCustom", obj, parent)

typedef struct
{
    gldobj_t Obj; /* radice dell'oggetto (comune a tutte le tipologie di oggetto) */

    gldobj_EventFunc_t EventCb; /* funzione notifica eventi */
} objcustom_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *gldcustom_Create (uint32_t memSize);
gldobj_t *gldcustom_Background_Create (gldobj_DrawFunc_t func_draw);

#endif /* GLDCUSTOM_H_INCLUDED */
