#ifndef GLDOBJ_H_INCLUDED
#define GLDOBJ_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldHal/GldHalDisp.h"
#include "GldMisc/GldCoord.h"
#include "GldMisc/GldLlist.h"

//====================================================================== DEFINES

#define GLDOBJ_NUM_THEARD_MAX 7

#define GLDOBJ_DEF(obj, parent) \
    for (gldobj_t *newThisParent = parent, *thisParent = newThisParent, *newThisObj = (obj), *thisObj = newThisObj; \
         newThisObj != NULL; \
         gldobj_Add (newThisObj, thisParent), newThisObj = NULL)

//#define GLDOBJ_INTEGER_TAG

#ifdef GLDOBJ_INTEGER_TAG
#define GLDOBJ_GET_BY_TAG(objP, ...) \
    gldobj_GetByTag (objP, ##__VA_ARGS__, 0)
#endif

struct gldobj_s;

typedef enum
{
    GLDOBJ_EVENT_DESTROY = 0,           // appena prima che l'oggetto sia effevamente distrutto
    GLDOBJ_EVENT_PARENT_CHANGED,        // quado il padre dell'oggetto viene modificato
    GLDOBJ_EVENT_CHILD_LIST_CHANGED,    // quando viene modificata in qualsiasi modo la lista dei figli
    GLDOBJ_EVENT_COORD_CHANGED,         // quando cambiano le coordinate di un'oggetto
    GLDOBJ_EVENT_PARENT_COORD_CHANGED,  // le coordinate del padre sono cambiate
    GLDOBJ_EVENT_CHILD_COORD_CHANGED,   // le coordinate del figlio sono cambiate
    
    /* button specific event */
    GLDOBJ_EVENT_PRESS,
    GLDOBJ_EVENT_RELEASE,
    GLDOBJ_EVENT_FOCUS_ON,
    GLDOBJ_EVENT_FOCUS_OFF,
    GLDOBJ_EVENT_CLICK,
    GLDOBJ_EVENT_LONG_CLICK,
    GLDOBJ_EVENT_REPEAT_CLICK,
    GLDOBJ_EVENT_DRAG_START,
    GLDOBJ_EVENT_DARG_MOVE,
    GLDOBJ_EVENT_DRAG_END,
} gldobj_Event_e;

typedef enum
{
    /* La funzione draw di un oggetto viene chiamata con GLDOBJ_DRAW_MAIN prima
    di disegnare i suoi figli.
    I figli dell'oggetto possono quindi scrivere sopra tutto quello che è stato
    disegnato dal padre nella fase GLDOBJ_DRAW_MAIN.
    */
    GLDOBJ_DRAW_MAIN,
    /* La funzione draw di un oggetto viene chiamata con GLDOBJ_DRAW_POST dopo
    aver disegnato i suoi figli.
    L'oggetto, durante la fase GLDOBJ_DRAW_POST, ha quindi l'occasione di disegnare
    sopra a ciò che è stato disegnato dai suoi figli.
    */
    GLDOBJ_DRAW_POST,
} gldobj_Draw_e;

typedef void (*gldobj_EventFunc_t) (struct gldobj_s *obj, gldobj_Event_e evt, void *ext);
typedef void (*gldobj_DrawFunc_t) (gldhaldisp_t *disp, struct gldobj_s *obj, gldobj_Draw_e cmd);
typedef void (*gldobj_DestroyFunc_t) (struct gldobj_s *obj);

typedef struct gldobj_Class_s
{
    /* punta alla "superclasse" di questa classe di oggetti.
    NULL se non c'è superclasse. */
    const struct gldobj_Class_s *SuperClass;
    /* il nome per questa classe di oggetti */
    const char *Name;
} gldobj_Class_t;

typedef struct gldobj_s
{
    const gldobj_Class_t *Class; /* classe a cui appartiene l'oggetto */
    /* area grafica (il contenuto dell'oggetto viene rappresentato solo
    all'interno di quest'area) */
    gldcoord_Area_t Area;
    #ifdef GLDOBJ_INTEGER_TAG
    int16_t Tag;                /* codice identificativo dell'oggetto */
    #else
    const char *Tag;            /* una stringa identificativa per l'oggetto.
                                un tag può essere assegnato ad un'oggetto dopo essere
                                stato creato. in un secondo momento, un riferimento
                                all'oggetto può essere recuperato grazie al tag,
                                attraverso la funzione gldobj_GetByTag.
                                */
    #endif
    struct gldobj_s *Parent;    /* puntatore a padre dell'oggetto */
    gldllist_Node_t ChildEntry; /* record di questo oggetto nella catena gerarchica */
    gldllist_t ChildList;       /* lista figli */
    /* Pu� essere interessante aggiungere una piccola lista dinamica di funzioni
    di gestione eventi, per aggiungere dinamicamente la gestione di un evento
    oppure permettere la gestione parziale di alcune funzionalit�. */
    gldobj_DrawFunc_t FuncDraw;       /* funzione rappresentazione */
    gldobj_EventFunc_t FuncEvent;     /* funzione notifica eventi */
    bool Hide : 1;                    /* 1: nascondi l'oggetto (e tutti i suoi figli) */
    /* numero di thread legati a questo oggetto [7 al massimo]. */
    uint8_t NumLinkedThread : 3;
    void *UsrPointer; /* puntatore a disposizione dell'utente */
} gldobj_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *gldobj_Create (uint32_t memSize);
void gldobj_Add (gldobj_t *obj, gldobj_t *parent);
void gldobj_AddBack (gldobj_t *obj, gldobj_t *brother);
void gldobj_AddFront (gldobj_t *obj, gldobj_t *brother);
void gldobj_Remove (gldobj_t *obj);
void gldobj_Invalidate (gldobj_t *obj);
void gldobj_InvalidateArea (gldobj_t *obj, gldcoord_Area_t *area);
void gldobj_Destroy (gldobj_t *obj);

#ifdef GLDOBJ_INTEGER_TAG
void gldobj_SetTag (gldobj_t *obj, int16_t tag);
#else
void gldobj_SetTag (gldobj_t *obj, const char *tag);
#endif
void gldobj_SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y);
void gldobj_SetRelPos (gldobj_t *obj, gldobj_t *ref, gldcoord_t x, gldcoord_t y);
void gldobj_SetSize (gldobj_t *obj, gldcoord_t x_size, gldcoord_t y_size);
void gldobj_SetSizeAs (gldobj_t *obj, gldobj_t *ref);
void gldobj_SetVisible (gldobj_t *obj, bool visible);
void gldobj_Move (gldobj_t *obj, gldcoord_t x_shift, gldcoord_t y_shift);

void gldobj_HAlign (gldobj_t *obj, gldobj_t *ref, gldcoord_HRAlign_e align, gldcoord_t delta_x);
void gldobj_HAlignLL (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignLM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignLR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignML (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignMM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignMR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignRL (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignRM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);
void gldobj_HAlignRR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x);

void gldobj_VAlign (gldobj_t *obj, gldobj_t *ref, gldcoord_VRAlign_e align, gldcoord_t delta_y);
void gldobj_VAlignTT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignTM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignTB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignMT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignMM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignMB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignBT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignBM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);
void gldobj_VAlignBB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y);

void gldobj_AlignCenter (gldobj_t *obj, gldobj_t *ref);
void gldobj_HPosAlign (gldobj_t *obj, gldcoord_t xRef, uint8_t align);
void gldobj_VPosAlign (gldobj_t *obj, gldcoord_t yRef, uint8_t align);
int32_t gldobj_GetChildIdx (gldobj_t *obj);

gldcoord_t gldobj_GetWidth (gldobj_t *obj);
gldcoord_t gldobj_GetHeight (gldobj_t *obj);
#ifdef GLDOBJ_INTEGER_TAG
gldobj_t *gldobj_GetByTag (gldobj_t *root, ...);
#else
gldobj_t *gldobj_GetByTag (gldobj_t *root, const char *tag);
#endif
gldobj_t *gldobj_GetNext (gldobj_t *obj);
gldobj_t *gldobj_GetPrev (gldobj_t *obj);
gldobj_t *gldobj_GetChild (gldobj_t *parent, int16_t num);
bool gldobj_IsInside (gldobj_t *obj, gldobj_t *container);
bool gldobj_TagMatch (gldobj_t *obj, const char *tag);

void gldobj_ToForeground (gldobj_t *obj);
void gldobj_ToBackground (gldobj_t *obj);

void gldobj_FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext);

#endif /* GLDOBJ_H_INCLUDED */
