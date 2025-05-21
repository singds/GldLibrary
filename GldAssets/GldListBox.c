//===================================================================== INCLUDES
#include "GldAssets/GldListBox.h"

#include "GldAssets/GldButton.h"
#include "GldAssets/GldCustom.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "GldListBox"

//=========================================================== PRIVATE PROTOTYPES

static void ObjDrawListBox (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
static void ObjEvtListBox (gldobj_t *obj, gldobj_Event_e evt, void *ext);
static void ObjEvtSpecBox (gldobj_t *obj, gldobj_Event_e evt, void *ext);
static void ObjEvtSpecPrev (gldobj_t *obj, gldobj_Event_e evt, void *ext);
static void ObjEvtSpecNext (gldobj_t *obj, gldobj_Event_e evt, void *ext);

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
gldobj_t *gldlistbox_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (gldlistbox_t, listBox, gldbutton_Create (memSize));
    obj->FuncDraw = NULL;
    listBox->Button.Click = true;
    listBox->Button.FuncEventSpec = ObjEvtListBox;
    
    /* Init the scroll object */
    GLDCUSTOM_DEF (gldcustom_Create (0), obj)
    {
        listBox->ObjHolder = thisObj;
        thisObj->FuncDraw = ObjDrawListBox;
        
        listBox->ObjScroll = gldcustom_Create (0);
        gldobj_Add (listBox->ObjScroll, listBox->ObjHolder);
    }
    
    /* creo il pulsante prev */
    GLDBUTTON_DEF (gldbutton_Create (0), obj)
    {
        listBox->ObjButtonPrev = thisObj;
        thisButton->FuncEventSpec = ObjEvtSpecPrev;
    }

    /* creo il pulsante next */
    GLDBUTTON_DEF (gldbutton_Create (0), obj)
    {
        listBox->ObjButtonNext = thisObj;
        thisButton->FuncEventSpec = ObjEvtSpecNext;
    }

    /* Set default values */
    listBox->HolderColor = 0x00787878;
    listBox->BoxColor = GLDCOLOR_WHITE;
    listBox->BoxYSize = 36;
    listBox->BoxYSpace = 4;
    listBox->NumVisibleBox = 4;
    
    /* metto l'oggetto in primo piano: in questo modo "si impossessa" di tutti
       gli eventi tattili degli oggetti sottostanti */
    gldobj_ToForeground (obj);
    return obj;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldlistbox_SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y)
{
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj);
    
    gldobj_SetAbsPos (listBox->ObjHolder, x, y);
    gldobj_SetAbsPos (listBox->ObjScroll, x, y);
    gldobj_SetAbsPos (listBox->ObjButtonPrev, x, y);
    gldobj_SetAbsPos (listBox->ObjButtonNext, x + (gldcoord_AreaWidth (&listBox->ObjScroll->Area) / 2), y);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldlistbox_SetWidth (gldobj_t *obj, gldcoord_t width)
{
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj);
    
    gldobj_SetSize (listBox->ObjHolder, width, listBox->BoxYSize);
    gldobj_SetSize (listBox->ObjScroll, width, listBox->BoxYSize);
    gldobj_SetSize (listBox->ObjButtonPrev, (width / 2), listBox->BoxYSize);
    gldobj_SetAbsPos (listBox->ObjButtonNext, listBox->ObjHolder->Area.X1 + (width / 2), listBox->ObjHolder->Area.Y1);
    gldobj_SetSize (listBox->ObjButtonNext, (width - (width / 2)), listBox->BoxYSize);
}

/*______________________________________________________________________________ 
 Desc:  Crea una selezione della listbox. La selezione è a tutti gli effetti un
        bottone caratterizzato da dimensione x come la listbox e dimensione y
        come impostata nelle proprietà della listbox. La posizione e visibilità
        delle selezioni vengono aggiustate con la chiamata 'gldlistbox_Open'.
 Arg: - <parent>[in] padre di questa selezione: deve essere una listbox.
 Ret: - Puntatore a selezione (oggetto button).
______________________________________________________________________________*/
gldobj_t *gldlistbox_BoxCreate (gldobj_t *obj)
{   /* the parent must be a listbox */
    gldobj_t *objBox = NULL;
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj);

    objBox = gldbutton_Create (0);
    GLDBUTTON_DEF (objBox, listBox->ObjScroll)
    {
        gldobj_SetRelPos (thisObj, thisParent, 0, 0);
        gldobj_SetSize (thisObj, gldcoord_AreaWidth (&listBox->ObjScroll->Area), listBox->BoxYSize);
        thisButton->BodyRadius = 0;
        thisButton->BorderSize = 0;
        thisButton->BodyColor = listBox->BoxColor;
        thisButton->FuncEventSpec = ObjEvtSpecBox;
    }
    return objBox;
}

/*______________________________________________________________________________ 
 Desc:  Apre una listbox mettendo a "fuoco" l'ennesima selezione (focus_index).
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void gldlistbox_Open (gldobj_t *obj, uint8_t focus_index)
{
    gldobj_t *obj_holder, *obj_scroll;
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj);

    obj_holder = listBox->ObjHolder;
    obj_scroll = listBox->ObjScroll;
    
    gldcoord_t x0_pos, y0_pos, holder_y_size, scroll_y_size;
    gldobj_t *child;
    uint8_t visible_child_num;

    visible_child_num = gldllist_Size (&obj_scroll->ChildList);            /* numero totale di figli */
    visible_child_num = _MIN_ (visible_child_num, listBox->NumVisibleBox); /* limito a numero massimo impostato */

    /* dimensiono l'holder dei box */
    holder_y_size = visible_child_num * (listBox->BoxYSize + listBox->BoxYSpace);
    holder_y_size += listBox->BoxYSpace;
    obj_holder->Area.Y2 = obj_holder->Area.Y1 + holder_y_size;

    /* dimensiono lo scroll dei box */
    scroll_y_size = gldllist_Size (&obj_scroll->ChildList) * (listBox->BoxYSize + listBox->BoxYSpace);
    scroll_y_size += listBox->BoxYSpace;
    obj_scroll->Area.Y2 = obj_scroll->Area.Y1 + scroll_y_size;

    /* Posiziono correttamente i box da visualizzare.
       Sono considerati box della listbox tutti i figli dell'oggetto
       ObjScroll. */
    x0_pos = obj_scroll->Area.X1;
    y0_pos = obj_scroll->Area.Y1 + listBox->BoxYSpace;

    child = gldobj_GetChild (obj_scroll, 0);
    while (child != NULL)
    {
        gldobj_SetAbsPos (child, x0_pos, y0_pos);
        y0_pos += listBox->BoxYSize + listBox->BoxYSpace;

        child = gldobj_GetNext (child);
    }

    { /* miuovo lo scroll per portare al centro l'indice a fuoco */
        gldcoord_t y_shift;

        y_shift = -(listBox->NumVisibleBox / 2) + focus_index;
        y_shift *= listBox->BoxYSize + listBox->BoxYSpace;
        /* lo scrolling della lista ai suoi estremi */
        y_shift = _MIN_ (y_shift, (scroll_y_size - holder_y_size));
        y_shift = _MAX_ (y_shift, 0);

        gldobj_Move (obj_scroll, 0, -y_shift);
    }

    { /* abilito i pulsanti precedente e successivo solamente quando se
         i box visualizzabili sono meno di quelli presenti. */
        if (listBox->NumVisibleBox >= gldllist_Size (&obj_scroll->ChildList))
        {
            gldobj_SetVisible (listBox->ObjButtonPrev, false);
            gldobj_SetVisible (listBox->ObjButtonNext, false);
        }
    }
    
    /* refresh della listbox */
    gldobj_Invalidate (listBox->ObjHolder);

    { /* posiziono i pulsanti next prev */
        gldobj_t *next, *prev;

        next = listBox->ObjButtonNext;
        prev = listBox->ObjButtonPrev;

        gldobj_SetAbsPos (prev, prev->Area.X1, obj_holder->Area.Y2);
        gldobj_SetAbsPos (next, next->Area.X1, obj_holder->Area.Y2);
    }
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void ObjDrawListBox (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj->Parent);
    
    /* perform the object drawing */
    if (cmd == GLDOBJ_DRAW_MAIN) {
        gldcoord_t x1, x2, y1, y2;

        x1 = obj->Area.X1;
        y1 = obj->Area.Y1;
        x2 = obj->Area.X2;
        y2 = obj->Area.Y2;

        if (listBox->HolderColor != GLDCOLOR_NONE)
            glddraw_FillArea (disp, x1, y1, x2, y2, listBox->HolderColor);
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void ObjEvtListBox (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj);

    if (evt == GLDOBJ_EVENT_CLICK)
        gldobj_Destroy (obj);
    if (evt == GLDOBJ_EVENT_PARENT_CHANGED)
    {
        if (obj->Parent)
        {
            obj->Area.X1 = obj->Parent->Area.X1;
            obj->Area.Y1 = obj->Parent->Area.Y1;
            obj->Area.X2 = obj->Parent->Area.X2;
            obj->Area.Y2 = obj->Parent->Area.Y2;
            gldobj_Invalidate (obj);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void ObjEvtSpecBox (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (gldbutton_t, button, "GldButton", obj);

    if (evt == GLDOBJ_EVENT_CLICK)
    { /* identifico l'indice di questa selezione */
        GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj->Parent->Parent->Parent);
        uint16_t index = 0;
        gldobj_t *selection = gldobj_GetChild (listBox->ObjScroll, 0);

        while (selection != NULL)
        {
            if (selection == obj)
            { /* coincide con l'oggetto premuto */
                if (listBox->FuncSelect != NULL)
                {
                    /* chiudo la listbox solamente se l'utente ritorna un valora
                       pari a 0 */
                    if (listBox->FuncSelect (obj, index) == 0)
                        gldobj_Destroy (&listBox->Button.Obj);
                }
                break;
            }
            index++;
            selection = gldobj_GetNext (selection);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void ObjEvtSpecPrev (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (gldbutton_t, button, "GldButton", obj);

    if (evt == GLDOBJ_EVENT_CLICK)
    {
        GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj->Parent);
        if (listBox->ObjScroll->Area.Y1 < listBox->ObjHolder->Area.Y1)
        { /* muovo la lista di oggetti solamente se altri successivi devono
             essere visualizzati */
            gldcoord_t y_move = listBox->BoxYSize + listBox->BoxYSpace;
            gldobj_Move (listBox->ObjScroll, 0, y_move);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void ObjEvtSpecNext (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (gldbutton_t, button, "GldButton", obj);

    if (evt == GLDOBJ_EVENT_CLICK)
    {
        GLDASSETS_GET_TYPED (gldlistbox_t, listBox, L_OBJ_NAME, obj->Parent);
        if (listBox->ObjScroll->Area.Y2 > listBox->ObjHolder->Area.Y2)
        { /* muovo la lista di oggetti solamente se altri successivi devono
             essere visualizzati */
            gldcoord_t y_move = listBox->BoxYSize + listBox->BoxYSpace;
            gldobj_Move (listBox->ObjScroll, 0, -y_move);
        }
    }
}
