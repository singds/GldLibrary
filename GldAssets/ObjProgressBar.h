#ifndef OBJPROGRESSBAR_H_INCLUDED
#define OBJPROGRESSBAR_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"
#include "GldAssets/ObjButton.h"

//====================================================================== DEFINES
#define OBJPROGRESSBAR_DEF(obj, parent) \
    GLDASSETS_DEF (objprogressbar_t, thisProgressbar, "ObjProgressbar", obj, parent)

typedef enum
{
    OBJPROGRESSBAR_ORI_LR = 0, /* la barra si riempie da sinistra a destra */
    OBJPROGRESSBAR_ORI_RL,     /* la barra si riempie da destra a sinistra */
    OBJPROGRESSBAR_ORI_BT,     /* la barra si riempie dal basso verso l'alto */
    OBJPROGRESSBAR_ORI_TB,     /* la barra si riempie dall'altro verso il basso */
} objprogressbar_Ori_e;

typedef struct
{
    objbutton_t Button; // radice dell'oggetto

    uint16_t Percentuale;             /* percentuale corrente della barra di progresso */
    objprogressbar_Ori_e Orientation; /* orientamento della barra */
    gldcoord_t BodyRadius;            /* raggio bordi della barra */
    gldcoord_t BorderSize;            /* dimensione bordo della barra */
    gldcolor_t BodyColor;             /* colore del corpo principale con percentuale massima [100%] */
    gldcolor_t BorderColor;           /* colore bordo */
} objprogressbar_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *objprogressbar_Create (uint32_t memSize);
void objprogressbar_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);

void objprogressbar_SetBodyClr (gldobj_t *obj, gldcolor_t bodyClr);
void objprogressbar_SetBorderClr (gldobj_t *obj, gldcolor_t border_clr);
void objprogressbar_SetProgress (gldobj_t *obj, uint8_t progress);
uint8_t objprogressbar_GetProgress (gldobj_t *obj);

#endif /* OBJPROGRESSBAR_H_INCLUDED */
