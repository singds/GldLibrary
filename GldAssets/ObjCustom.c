//===================================================================== INCLUDES
#include "GldAssets/ObjCustom.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjCustom"

//=========================================================== PRIVATE PROTOTYPES
static void FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext);

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
gldobj_t *gldcustom_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objcustom_t, custom, gldobj_Create (memSize));
    
    custom->Obj.FuncEvent = FuncEvent;
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Crea un'oggetto sfondo che copre l'intera area del display.
 Arg: - <func_draw> la funzione da usare per disegnare lo sfondo.
 Ret: - Il nuovo oggetto che funge da sfondo.
______________________________________________________________________________*/
gldobj_t *gldcustom_Background_Create (gldobj_DrawFunc_t func_draw)
{
    gldcoord_t x1, y1, x_size, y_size;
    gldobj_t *obj;

    /* Creo un'oggetto della dimensione del display attualmente attivo con la
       funzione di visualizzazione specificata */
    x1 = glddisp_Disp->Canvas.X1;
    y1 = glddisp_Disp->Canvas.Y1,
    x_size = gldcoord_AreaWidth (&glddisp_Disp->Canvas);
    y_size = gldcoord_AreaHeight (&glddisp_Disp->Canvas);

    obj = gldcustom_Create (0);
    gldobj_SetAbsPos (obj, x1, y1);
    gldobj_SetSize (obj, x_size, y_size);
    obj->FuncDraw = func_draw;
    return obj;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (objcustom_t, custom, L_OBJ_NAME, obj);

    /* notify user about the event */
    if (custom->EventCb != NULL)
        custom->EventCb (obj, evt, ext);
}
