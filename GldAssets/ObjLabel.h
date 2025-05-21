#ifndef OBJLABEL_H_INCLUDED
#define OBJLABEL_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"
#include "GldCfg.h"
#include <stdarg.h>

//====================================================================== DEFINES
#define OBJLABEL_DEF(obj, parent) \
    GLDASSETS_DEF (objlabel_t, thisLabel, "ObjLabel", obj, parent)

typedef struct
{
    gldobj_t Obj;               /* l'oggetto base.
                                la label è in qualche modo un'espansione dell'oggetto base.
                                */

    const char *TxtStatic;      /* testo statico */
    char *TxtDynamic;           /* testo dinamico allocato */
    uint16_t TxtDynamicSize;    /* dimensione in byte dell'array dinamico allocato */
    gldcoord_Point_t Pos;       /* riferimento posizionamento testo */
    uint8_t Align : 4;          /* allineamento rispetto posizione di riferimento */
    bool AutoNewline : 1;       /* 1: abilita la gestione del caporiga automatico */
    bool TxtIsStatic : 1;       /* 1: il testo rappresentato dalla label � statico */
    gldfont_t *Font;            /* font */
    gldcolor_t Color;           /* colore */
    gldcolor_t ColorBg;         /* colore background */
} objlabel_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *objlabel_Create (uint32_t memSize);
void objlabel_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
void objlabel_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext);

void *objlabel_Alloca (gldobj_t *obj, uint16_t size);
int32_t objlabel_Printf (gldobj_t *obj, const char *format, ...);
int32_t objlabel_VPrintf (gldobj_t *obj, const char *format, va_list va);
void objlabel_SetStaticTxt (gldobj_t *obj, const char *txt);
const char *objlabel_GetTxt (gldobj_t *obj);
void objlabel_SetPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y);
void objlabel_SetColor (gldobj_t *obj, gldcolor_t color);

#endif /* OBJLABEL_H_INCLUDED */
