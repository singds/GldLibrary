#ifndef OBJNUMBER_H_INCLUDED
#define OBJNUMBER_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldAssets/GldAssets.h"
#include "GldAssets/ObjLabel.h"

//====================================================================== DEFINES
#define OBJNUMBER_DEF(obj, parent) \
    GLDASSETS_DEF (objnumber_t, thisNumber, "ObjNumber", obj, parent)

typedef enum
{
    OBJNUMBER_TYPE_U8,      // quando il numero è un uint8_t
    OBJNUMBER_TYPE_S8,      // quando il numero è un int8_t
    OBJNUMBER_TYPE_U16,     // quando il numero è un uint16_t
    OBJNUMBER_TYPE_S16,     // quando il numero è un int16_t
    OBJNUMBER_TYPE_U32,     // quando il numero è un uint32_t
    OBJNUMBER_TYPE_S32,     // quando il numero è un int32_t
} objnumber_Type_e;

typedef struct
{
    objlabel_t Label; /* la radice dell'oggetto number è una label.
                      il numero è in qualche modo un'estensione della label. */

    /* il valore della variabile puntata da NumPnt viene moltiplicato per ScaleFactor
    prima di essere visualizzato */
    float ScaleFactor;
    int32_t Min;                /* minimo valore che pu� assumere il numero */
    int32_t Max;                /* massimo valore che pu� assumere il numero */
    uint8_t Decimali;           /* cifre decimali da visualizzare */
    uint8_t MinDigit;           /* forza la visualizzazione con un certo numero di cifre (es: "%02d" forza 2 cifre) */
    objnumber_Type_e NumType;   /* tipo di numero a cui punta 'NumPnt' */
    void *NumPnt;               /* puntatore al numero */
    /* funzione che viene chiamata periodicamente e aggiorna la label con il
    numero visualizzato */
    void (*FuncRefresh) (gldobj_t *obj);
} objnumber_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *objnumber_Create (uint32_t memSize);
void objnumber_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
void objnumber_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext);

int32_t objnumber_GetValue (objnumber_Type_e type, void *ptr);
void objnumber_SetValue (objnumber_Type_e type, void *ptr, int32_t value);

#endif /* OBJNUMBER_H_INCLUDED */
