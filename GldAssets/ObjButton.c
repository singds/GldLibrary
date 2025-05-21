//===================================================================== INCLUDES
#include "GldAssets/ObjButton.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjButton"

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};

//============================================================= GLOBAL VARIABLES
gldobj_EventFunc_t objbutton_GlobalEvent = NULL;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea un pulsante.
    Di default un pusante è un quadrato bianco di dimensione 100x100px, con
    bordo nero da 1px e bordi arrotondati con raggio 10px.
 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldbutton_t).
 Ret: - puntatore al nuovo pulsante.
______________________________________________________________________________*/
gldobj_t *objbutton_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE(objbutton_t, button, gldobj_Create (memSize));
    obj->FuncDraw = objbutton_Draw;
    obj->FuncEvent = objbutton_Event;
    
    button->StateFocus = false;
    button->StatePress = false;
    button->Click = 1;
    button->LongClick = 0;
    button->DragV = 0;
    button->DragH = 0;
    button->InputWall = 0;
    button->LongClickTime = 10; // 1 sec
    button->BodyColor = GLDCOLOR_WHITE;
    button->BorderColor = GLDCOLOR_BLACK;
    button->BodyRadius = 10;
    button->BorderSize = 1;
    return &button->Obj;
}

/*______________________________________________________________________________
 Desc:  Crea un pulsante con i colori del corpo e del contorno già settati a
    trasparente. Questa è un'utlity function comoda da avere.
 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldbutton_t).
 Ret: - puntatore al nuovo pulsante.
______________________________________________________________________________*/
gldobj_t *objbutton_CreateTransparent (uint32_t memSize)
{
    gldobj_t *obj = objbutton_Create (memSize);
    if (obj != NULL) {
        GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
        button->BodyColor = GLDCOLOR_NONE;
        button->BorderColor = GLDCOLOR_NONE;
    }
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Disegna un pulsante.
    Il pulsante è un box con un contorno e angoli eventualmente arrotondati.
 Arg: - <disp> il display driver.
        <obj> il pulsante da disegnare.
        <cmd> la fase di disegno da portare a termine.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
    
    /* perform the object drawing */
    gldcoord_Area_t *area = &button->Obj.Area;

    if (cmd == GLDOBJ_DRAW_POST) {
        if (button->BorderColor != GLDCOLOR_NONE && button->BorderSize > 0)
            glddraw_EmptyRound (disp, area->X1, area->Y1, area->X2, area->Y2, button->BorderSize, button->BodyRadius, button->BorderColor);
    }
    if (cmd == GLDOBJ_DRAW_MAIN) {
        gldcoord_t x1, x2, y1, y2, radius;

        x1 = area->X1 + button->BorderSize;
        y1 = area->Y1 + button->BorderSize;
        x2 = area->X2 - button->BorderSize;
        y2 = area->Y2 - button->BorderSize;

        radius = 0;
        if (button->BodyRadius > button->BorderSize)
            radius = button->BodyRadius - button->BorderSize;

        if (button->BodyColor != GLDCOLOR_NONE)
            glddraw_FillRound (disp, x1, y1, x2, y2, radius, button->BodyColor);
    }
}

/*______________________________________________________________________________
 Desc:  Gestisce gli eventi del pulsante.
 Arg: - <obj> il pulsante su cui l'evento e stato scatenato.
        <evt> l'evento.
        <ext> eventuali dati aggiuntivi associati all'evento.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);

    /* notify user about the event */
    if (objbutton_GlobalEvent != NULL) // global event function
        objbutton_GlobalEvent (obj, evt, ext);
    if (button->FuncEventClass != NULL) // class event function
        button->FuncEventClass (obj, evt, ext);
    if (button->FuncEventSpec != NULL) // specific event function
        button->FuncEventSpec (obj, evt, ext);
}

/*______________________________________________________________________________ 
 Desc:  Abilita/disabilita il fuoco dell'oggetto.
 Arg: - <obj> oggetto su cui abilitare / disabilitare il fuoco.
        <focus> true per abilitare / false per disabilitare il fuoco su questo
        oggetto.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_SetFocus (gldobj_t *obj, bool focus)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
    
    if (button->StateFocus != focus)
    {   /* notifico l'oggetto dell'attivazione / disattivazione del focus */
        button->StateFocus = focus;
        if (focus == true)
            obj->FuncEvent (obj, GLDOBJ_EVENT_FOCUS_ON, NULL);
        else
            obj->FuncEvent (obj, GLDOBJ_EVENT_FOCUS_OFF, NULL);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta il colore principale del bottone.
    Il colore del corpo del rettangolo.
 Arg: - <obj> il pulsante.
        <body_clr> il colore del corpo.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_SetBodyClr (gldobj_t *obj, gldcolor_t body_clr)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);

    if (button->BodyColor != body_clr)
    { /* modifico il colore e invalido la rappresentazione del bottone */
        button->BodyColor = body_clr;
        gldobj_Invalidate (&button->Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta il colore del bordo del bottone.
 Arg: - <obj> il pulsante.
        <border_clr> il colore del contorno.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_SetBorderClr (gldobj_t *obj, gldcolor_t border_clr)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);

    if (button->BorderColor != border_clr)
    { /* modifico il colore e invalido la rappresentazione del bottone */
        button->BorderColor = border_clr;
        gldobj_Invalidate (&button->Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta il raggio dei bordi del bottone.
 Arg: - <obj> il pulsante.
        <body_radius> il raggio del bordo.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_SetBodyRadius (gldobj_t *obj, gldcoord_t body_radius)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);

    if (button->BodyRadius != body_radius)
    {
        button->BodyRadius = body_radius;
        gldobj_Invalidate (&button->Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta la dimensione dei bordi del bottone.
 Arg: - <obj> il pulsante.
        <border_size> la dimensione in pixel del bottone.
 Ret: - None.
______________________________________________________________________________*/
void objbutton_SetBorderSize (gldobj_t *obj, gldcoord_t border_size)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);

    if (button->BodyRadius != border_size)
    {
        button->BorderSize = border_size;
        gldobj_Invalidate (&button->Obj);
    }
}

/*______________________________________________________________________________
 Desc:  Verifica se la funzione click del pulsante è abilitata.
 Arg: - <obj> pulsante di cui verificare se il click è abilitato.
 Ret: - true se il click è abilitato per questo oggetto.
______________________________________________________________________________*/
bool objbutton_GetClick (gldobj_t *obj)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
    return button->Click;
}

/*______________________________________________________________________________
 Desc:  Verifica se la funzione drag verticale del pulsante è abilitata.
 Arg: - <obj> pulsante di cui verificare se il drag verticale è abilitato.
 Ret: - None.
______________________________________________________________________________*/
bool objbutton_GetDragV (gldobj_t *obj)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
    return button->DragV;
}

/*______________________________________________________________________________
 Desc:  Verifica se la funzione drag orizzontale del pulsante è abilitata.
 Arg: - <obj> pulsante di cui verificare se il drag orizzontale è abilitato.
 Ret: - None.
______________________________________________________________________________*/
bool objbutton_GetDragH (gldobj_t *obj)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, L_OBJ_NAME, obj);
    return button->DragH;
}

//============================================================ PRIVATE FUNCTIONS
