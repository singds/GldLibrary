#ifndef GLDLISTBOX_H_INCLUDED
#define GLDLISTBOX_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldAssets/GldAssets.h"
#include "GldAssets/ObjButton.h"

//====================================================================== DEFINES
#define GLDLISTBOX_DEF(obj, parent) \
    GLDOBJ_DEF (obj, parent)        \
    for (gldlistbox_t *thisListbox = gldassets_CheckType (thisObj, &gldlistbox_Class); thisListbox != NULL; thisListbox = NULL)

typedef struct
{
    /* area che maschera l'interazione con gli oggetti sottostanti della pagina
       e gestisce la chiusura della listbox */
    objbutton_t Button;
    
    gldobj_t *ObjHolder; /* contenitore dei box della lista */
    gldobj_t *ObjScroll;
    gldobj_t *ObjButtonPrev; /* pulsante per retrocedere nella lista */
    gldobj_t *ObjButtonNext; /* pulsante per avanzare nella lista */
    /* funzione chiamata al click di un box della lista.
       <obj> puntatore alla selezione premuta.
       <index> indice di questa selezione nella lista.
       restituisci un valore != 0 se non vuoi chiudere la listbox dopo la selezione.
    */
    int32_t (*FuncSelect) (gldobj_t *obj, uint16_t index);
    gldcolor_t HolderColor; /* colore contenitore dei box selezionabili */
    gldcolor_t BoxColor;    /* colore del singolo box */
    gldcoord_t BoxYSize;    /* dimensione del singolo box */
    gldcoord_t BoxYSpace;   /* spazio (y) fra un box e il successivo */
    uint8_t NumVisibleBox;  /* numero di box visibili contemporaneamente */
} gldlistbox_t;

//============================================================= GLOBAL VARIABLES
extern gldobj_Class_t gldlistbox_Class;

//============================================================ GLOBAL PROTOTYPES
gldobj_t *gldlistbox_Create (uint32_t memSize);

void gldlistbox_SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y);
void gldlistbox_SetWidth (gldobj_t *obj, gldcoord_t width);
gldobj_t *gldlistbox_BoxCreate (gldobj_t *obj);
void gldlistbox_Open (gldobj_t *obj, uint8_t focus_index);

#endif /* GLDLISTBOX_H_INCLUDED */
