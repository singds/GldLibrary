#ifndef GLDASSETS_H_INCLUDED
#define GLDASSETS_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include <string.h>
#include <stdio.h>

#include "GldCfg.h"
#include "GldCore/GldObj.h"
#include "GldCore/GldDraw.h"
#include "GldCore/GldDisp.h"
#include "GldCore/GldRefr.h"
#include "GldMisc/GldColor.h"
#include "GldRes/GldTxt.h"

//====================================================================== DEFINES
/*
Crea la variabile <name>, di tipo puntatore a <type>, e gli assegna il puntatore
<obj> se <obj> punta ad un'oggetto con classe <objClass>.
Se <obj> non è un'oggetto della classe <objClass>, un assert dovrebbe interrompere
l'esecuzione del codice con un breakpoint. Questo dovrebbe accadere solo in fase
di sviluppo, quando hai fatto un errore di programmazione.
*/
#define GLDASSETS_GET_TYPED(type, name, objClass, obj) \
    type *name = gldassets_CheckType ((obj), (objClass)); \
    (void)name; /* avoid warnings */ \
    GLDCFG_ASSERT(name)


#define GLDASSETS_ALLOCATE(type, name, create) \
    if (memSize == 0) \
        memSize = sizeof (type); \
    gldobj_t *obj = create; \
    GLDCFG_ASSERT (obj); \
    type *name = (type *)obj; \
    (void)name; /* avoid warnings */ \
     \
    if (ObjClass.SuperClass == NULL) \
        ObjClass.SuperClass = obj->Class; \
    obj->Class = &ObjClass


#define GLDASSETS_DEF(type, varname, objname, obj, parent) \
    GLDOBJ_DEF (obj, parent)       \
    for (type *varname = gldassets_CheckType (thisObj, objname); varname != NULL; varname = NULL)

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void *gldassets_CheckType (const gldobj_t *obj, const char *type);
void *gldassets_PickChild (const gldobj_t *obj, const char *type, uint16_t index);

#endif /* GLDASSETS_H_INCLUDED */

