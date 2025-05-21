//===================================================================== INCLUDES
#include "ObjTemplate.h"

#include "Activity.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjTemplate"

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
gldobj_t *objtemplate_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objtemplate_t, templt, gldobj_Create (memSize));

    /* set specific obj fields */

    return obj;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void objtemplate_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objtemplate_t, templt, L_OBJ_NAME, obj);

    /* draw this object */
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void objtemplate_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (objtemplate_t, templt, L_OBJ_NAME, obj);

    /* manage object event */
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static gldobj_t *GetRoot (gldobj_t *obj)
{
    while (obj != NULL && gldassets_CheckType (obj, L_OBJ_NAME) == false)
        obj = obj->Parent;
    return obj;
}
