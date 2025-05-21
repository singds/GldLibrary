#ifndef GLDBUTTON_H_INCLUDED
#define GLDBUTTON_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"

//====================================================================== DEFINES
#define OBJBUTTON_DEF(obj, parent) \
    GLDASSETS_DEF (objbutton_t, thisButton, "ObjButton", obj, parent)

typedef struct
{   /* radice dell'oggetto (comune a tutte le tipologie di oggetto) */
    gldobj_t Obj;

    /* Funzione getsione eventi per classe.
    Puoi usare questa funzione di gestione eventi per accomunare comportamenti
    comuni di una classe di pulsanti. */
    gldobj_EventFunc_t FuncEventClass;
    /* Funzione gestione eventi specifica.
    Generalemente diversa per ciascun pulsante. */
    gldobj_EventFunc_t FuncEventSpec;
    bool StatePress : 1;         /* 1 quando l'oggetto � premuto */
    bool StateFocus : 1;         /* 1 quando l'oggetto � in focus */
    bool Click : 1;              /* 1: abilita l'evento 'click' */
    bool LongClick : 1;          /* 1: abilita l'evento 'long click' (pressione prolungata) */
    bool DragV : 1;              /* 1: abilita il 'drag and drop' nella direzione verticale */
    bool DragH : 1;              /* 1: abilita il 'drag and drop' nella direzione orizzontale */
    bool InputWall : 1;          /* 1: blocca il gli eventi di input per gli oggetti sotto a questo */
    bool RepeatClick : 1;        /* 1: abilita l'evento 'repeat click' (eventi multipli quando tieni premuto) */
    uint8_t LongClickTime;       /* tempo evento long click [decimi s] */
    uint8_t BodyRadius;          /* raggio bordi del box */
    uint8_t BorderSize;          /* dimensione bordo box */
    gldcolor_t BodyColor;        /* colore main body */
    gldcolor_t BorderColor;      /* colore bordo */
    /* Parametri a disposizione dell'utente, da utilizzare libreamente. Anche
    ogni oggetto ha un puntatore ad uso libero dell'utente. */
    uint32_t UsrData[3];
} objbutton_t;

//============================================================= GLOBAL VARIABLES
/* viene richiamata ad ogni evento di un qualsiasi pulsante. E' stata aggiunta
   brutalmente per supportare il beep alla pressione di un tasto.
   (una soluzione pi� pulita sarebbe gradita) */
extern gldobj_EventFunc_t objbutton_GlobalEvent;

//============================================================ GLOBAL PROTOTYPES
gldobj_t *objbutton_Create (uint32_t memSize);
gldobj_t *objbutton_CreateTransparent (uint32_t memSize);
void objbutton_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
void objbutton_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext);

void objbutton_SetFocus (gldobj_t *obj, bool focus);
void objbutton_SetBodyClr (gldobj_t *obj, gldcolor_t body_clr);
void objbutton_SetBorderClr (gldobj_t *obj, gldcolor_t border_clr);
void objbutton_SetBodyRadius (gldobj_t *obj, gldcoord_t body_radius);
void objbutton_SetBorderSize (gldobj_t *obj, gldcoord_t border_size);
bool objbutton_GetClick (gldobj_t *obj);
bool objbutton_GetDragV (gldobj_t *obj);
bool objbutton_GetDragH (gldobj_t *obj);

#endif /* GLDBUTTON_H_INCLUDED */
