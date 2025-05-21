//===================================================================== INCLUDES
#include "GldAssets/ObjNumber.h"
#include "GldAssets/GldActivity.h"

#include "GldCfg.h"
#include "GldCore/GldTimer.h"
#include <math.h>

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjNumber"

//=========================================================== PRIVATE PROTOTYPES
void RefreshNumbers (gldobj_t *obj_p);
float GetFloatNumber (objnumber_t *number);
static void ThreadFunc (struct gldactivity_Thread_s *self);
static void DefaultFuncRefresh (gldobj_t *obj);

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};
static gldactivity_Thread_t *Thread;
static const int32_t PotenzeDi10[] = {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
};

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea un'oggetto numero.
    Un numero è sostanzialmente una label con un thread (polling) associato, che
    ad intervalli regolari stampa nella label il valore di una variabile.
    Un riferimento alla variabile da stampare e le configurazioni che definiscono
    come la label deve essere stampata, sono memorizzate all'interno dell'oggetto
    numero.
 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldimage_t).
 Ret: - Il nuovo oggetto numero.
______________________________________________________________________________*/
gldobj_t *objnumber_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objnumber_t, number, objlabel_Create (memSize));

    if (Thread == NULL) {
        Thread = gldactivity_ThreadCreate (NULL, ThreadFunc);
    }
    /* forzo l'esecuzione immediata del thread per aggiornare subito la
    rappresetnazione di questo nuovo dato */
    gldactivity_ThreadStart (Thread, 0);
    
    objlabel_Alloca (obj, 5);
    
    /* Set default values */
    number->ScaleFactor = 1;
    number->Max = INT32_MAX;
    number->Min = INT32_MIN;
    number->Decimali = 0;
    number->MinDigit = 0;
    number->NumPnt = NULL;
    number->NumType = OBJNUMBER_TYPE_U8;
    number->FuncRefresh = DefaultFuncRefresh;
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Disegna l'oggetto numero.
    L'oggetto numero è anche una label, quindi disegnamo semplicemente la label.
 Arg: - <disp> il display driver.
        <obj> la label da disegnare.
        <cmd> la fase di disegno che deve essere portata a termine.
 Ret: - None.
______________________________________________________________________________*/
void objnumber_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objnumber_t, number, L_OBJ_NAME, obj);
    /* draw this object */
    objlabel_Draw (disp, obj, cmd);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void objnumber_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (objnumber_t, number, L_OBJ_NAME, obj);
    /* manage object event */
    objlabel_Event (obj, evt, ext);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t objnumber_GetValue (objnumber_Type_e type, void *ptr)
{
    int32_t value = 0;

    switch (type)
    {
    case OBJNUMBER_TYPE_U8:   value = *((uint8_t *)ptr);   break;
    case OBJNUMBER_TYPE_S8:   value = *((int8_t *)ptr);    break;
    case OBJNUMBER_TYPE_U16:  value = *((uint16_t *)ptr);  break;
    case OBJNUMBER_TYPE_S16:  value = *((int16_t *)ptr);   break;
    case OBJNUMBER_TYPE_U32:  value = *((uint32_t *)ptr);  break;
    case OBJNUMBER_TYPE_S32:  value = *((int32_t *)ptr);   break;
    }
    return value;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void objnumber_SetValue (objnumber_Type_e type, void *ptr, int32_t value)
{
    switch (type)
    {
    case OBJNUMBER_TYPE_U8:   *((uint8_t *)ptr) = value;   break;
    case OBJNUMBER_TYPE_S8:   *((int8_t *)ptr) = value;    break;
    case OBJNUMBER_TYPE_U16:  *((uint16_t *)ptr) = value;  break;
    case OBJNUMBER_TYPE_S16:  *((int16_t *)ptr) = value;   break;
    case OBJNUMBER_TYPE_U32:  *((uint32_t *)ptr) = value;  break;
    case OBJNUMBER_TYPE_S32:  *((int32_t *)ptr) = value;   break;
    }
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Verifica se 'obj_p', e in modo ricorsivo su tutti i suoi figli, � un
        oggetto di tipo numero. In questo caso aggiorna la label associata
        stampandoci dentro il valore corrente del numero.
 Arg: - <obj_p>[in] puntatore ad oggetto radice.
 Ret: - None.
______________________________________________________________________________*/
void RefreshNumbers (gldobj_t *obj)
{
    gldllist_Node_t *node;
    objnumber_t *number = gldassets_CheckType (obj, L_OBJ_NAME);

    /* verifico se questo oggetto e un tipo numero */
    if (number != NULL && number->NumPnt != NULL)
        number->FuncRefresh (obj);

    /* do the same on the child */
    node = gldllist_GetFirst (&obj->ChildList);
    while (node)
    {
        gldobj_t *child = gldllist_Get (node);
        RefreshNumbers (child);
        node = gldllist_GetNext (node);
    }
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la rappresentazione in float dell'oggetto numero.
        Es: un    U8 di valore    25 e    2 decimali restituisce    0.25
            un   U32 di valore  3456 e    1 decimale restituisce  345.6
            un   I16 di valore   145 e    0 decimali restituisce  145.0
 Arg: - <number>[in] puntatore ad oggetto numero.
 Ret: - None.
______________________________________________________________________________*/
float GetFloatNumber (objnumber_t *number)
{
    float value = NAN;
    float divisore = pow (10, number->Decimali);

    switch (number->NumType)
    {
    case OBJNUMBER_TYPE_U8:
        value = *((uint8_t *)number->NumPnt) / divisore;
        break;
    case OBJNUMBER_TYPE_S8:
        value = *((int8_t *)number->NumPnt) / divisore;
        break;
    case OBJNUMBER_TYPE_U16:
        value = *((uint16_t *)number->NumPnt) / divisore;
        break;
    case OBJNUMBER_TYPE_S16:
        value = *((int16_t *)number->NumPnt) / divisore;
        break;
    case OBJNUMBER_TYPE_U32:
        value = *((uint32_t *)number->NumPnt) / divisore;
        break;
    case OBJNUMBER_TYPE_S32:
        value = *((int32_t *)number->NumPnt) / divisore;
        break;
    }

    return value;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void ThreadFunc (struct gldactivity_Thread_s *self)
{
    RefreshNumbers (gldrefr_RootObj);
    gldactivity_ThreadStart (self, GLDCFG_OBJNUMBER_REFRESH_MS);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DefaultFuncRefresh (gldobj_t *obj)
{
    float value;
    int32_t valoreIntero; // valore che racchiude parte intera e parte decimale in un unico numero.
    int32_t parteIntera, parteDecimale;
    GLDASSETS_GET_TYPED (objnumber_t, number, L_OBJ_NAME, obj);
    
    value = GetFloatNumber (number);
    if (number->ScaleFactor != 1)
        value *= number->ScaleFactor;
    valoreIntero = lround (value * PotenzeDi10[number->Decimali]);
    parteIntera = valoreIntero / PotenzeDi10[number->Decimali];
    parteDecimale = valoreIntero % PotenzeDi10[number->Decimali];
    
    if (number->Decimali == 0)
    {
        char format[] = "%00d";
        format[2] = number->MinDigit + '0';
        objlabel_Printf (obj, format, parteIntera);
    } else
    {
        char format[] = "%00d.%00d";
        format[2] = number->MinDigit + '0';
        format[7] = number->Decimali + '0';
        objlabel_Printf (obj, format, parteIntera, parteDecimale);
    }
}
