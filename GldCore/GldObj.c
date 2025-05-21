//===================================================================== INCLUDES
#include "GldCore/GldObj.h"

#include "GldAssets/GldActivity.h"
#include "GldCfg.h"
#include "GldCore/GldDisp.h"
#include "GldCore/GldRefr.h"
#include "GldCore/GldTouch.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static bool ObjGetVisibleArea (gldobj_t *obj, gldcoord_Area_t *area);
#if (0)
static void ObjGetObjDrawArea (gldobj_t *obj, gldcoord_Area_t *area);
#endif
#ifdef GLDOBJ_INTEGER_TAG
static gldobj_t *ObjGetByTag (gldobj_t *root, uint8_t listSize, int16_t *tagList);
#else
static gldobj_t *ObjGetByTag (gldobj_t *root, const char *path);
#endif
static void SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y, bool notifyParent);

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = "GldObj",
};

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea un oggetto grafico di base.
    L'oggetto grafico di base contiene per esempio la sua posizione,
    dimensione e visibilità.

    L'oggetto creato è invisibile ed ha dimensione 100x100.
    Difficilmente avrai bisogno di usare questa funzione mentre stai realizzando
    delle pagine grafiche.
    L'oggetto che viene creato da questa funzione realizza la struttura di base
    su cui vengono costruiti gli altri oggetti più complessi dell'interfaccia.
 Arg: - <memSize> la quantità di memoria da allocare per questo oggetto.
    memSize deve essere => di sizeof(gldobj_t).
    Se stai inglobando gldobj_t all'interno di una struttura più complessa
    (vedi per esempio gldlabel_t), potresti voler passare un memSize != 0.
 Ret: - Puntatore al nuovo oggetto creato.
______________________________________________________________________________*/
gldobj_t *gldobj_Create (uint32_t memSize)
{
    gldobj_t *obj;
    
    if (memSize == 0)
        memSize = sizeof (gldobj_t);
    obj = GLDCFG_CALLOC (1, memSize);
    GLDCFG_ASSERT (obj);
    if (obj)
    {
        /* initialize obj components */
        gldllist_NodeInit (&obj->ChildEntry);
        gldllist_Set (&obj->ChildEntry, obj);
        gldllist_Init (&obj->ChildList);
        obj->Parent = NULL;
        obj->Class = &ObjClass;

        obj->FuncDraw = NULL;
        obj->FuncEvent = gldobj_FuncEvent;
        
        obj->Hide = 0;
        
        // dimensione di default di un'oggetto
        obj->Area.X1 = 0;
        obj->Area.Y1 = 0;
        obj->Area.X2 = 100;
        obj->Area.Y2 = 100;
    }
    return obj;
}

/*______________________________________________________________________________
 Desc:  Aggiungi ad un'oggetto un figlio.
        L'ultimo figlio aggiunto è quello più in foreground. Come se buttassi
        dei fogli sopra una scrivania: l'ultimo che aggiungi è sopra a tutti
        gli altri.
 Arg: - <obj>[in] oggetto target, diventerà un figlio. Non deve avere un padre.
        <parent>[in] oggetto padre.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_Add (gldobj_t *obj, gldobj_t *parent)
{
    GLDCFG_ASSERT (obj);
    
    if (parent == NULL || obj->Parent != NULL)
        ; // questo oggetto ha già un padre, oppure non hai specificato un padre
    else {
        /* l'ultimo oggetto aggiunto è quello più in foreground */
        obj->Parent = parent;
        gldllist_AddFirst (&parent->ChildList, &obj->ChildEntry);
        gldobj_Invalidate (obj);
        obj->FuncEvent (obj, GLDOBJ_EVENT_PARENT_CHANGED, NULL);
        parent->FuncEvent (parent, GLDOBJ_EVENT_CHILD_LIST_CHANGED, NULL);
    }
}

/*______________________________________________________________________________
 Desc:  Aggiungi oggetto come fratello di un'altro oggetto.
        Aggiungilo prima di suo fratello, cioe' piu' in background del fratello.
        Il fratello deve avere un padre valido.
 Arg: - <obj>[in] oggetto target da aggiungere.
        <brother>[in] oggetto fratello.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_AddBack (gldobj_t *obj, gldobj_t *brother)
{
    GLDCFG_ASSERT (obj);
    
    if (brother == NULL ||
        obj->Parent != NULL ||
        brother->Parent == NULL)
        ; // questo oggetto ha già un padre, oppure il fratello non ha un padre
    else {
        /* l'ultimo oggetto aggiunto è quello più in foreground */
        gldobj_t *parent = brother->Parent;
        
        obj->Parent = parent;
        gldllist_AddAfter (&brother->ChildEntry, &obj->ChildEntry);
        gldobj_Invalidate (obj);
        obj->FuncEvent (obj, GLDOBJ_EVENT_PARENT_CHANGED, NULL);
        parent->FuncEvent (parent, GLDOBJ_EVENT_CHILD_LIST_CHANGED, NULL);
    }
}

/*______________________________________________________________________________
 Desc:  Aggiungi oggetto come fratello di un'altro oggetto.
        Aggiungilo dopo di suo fratello, cioe' piu' in foreground del fratello.
        Il fratello deve avere un padre valido.
 Arg: - <obj>[in] oggetto target da aggiungere.
        <brother>[in] oggetto fratello.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_AddFront (gldobj_t *obj, gldobj_t *brother)
{
    GLDCFG_ASSERT (obj);
    
    if (brother == NULL ||
        obj->Parent != NULL ||
        brother->Parent == NULL)
        ; // questo oggetto ha già un padre, oppure il fratello non ha un padre
    else {
        /* l'ultimo oggetto aggiunto è quello più in foreground */
        gldobj_t *parent = brother->Parent;
        
        obj->Parent = parent;
        gldllist_AddBefore (&brother->ChildEntry, &obj->ChildEntry);
        gldobj_Invalidate (obj);
        obj->FuncEvent (obj, GLDOBJ_EVENT_PARENT_CHANGED, NULL);
        parent->FuncEvent (parent, GLDOBJ_EVENT_CHILD_LIST_CHANGED, NULL);
    }
}

/*______________________________________________________________________________
 Desc:  Rimuovi un'oggetto dalla lista dei figli di suo padre. Se l'oggetto
        non ha un padre, semplicemente la funzione non ha effetto.
        Attenzione che l'oggetto non viene eliminato, viene semplicemente
        eliminata la sua relazione con il padre.
        Per distriggere (liberare la memoria) di un'oggetto, vedi gldobj_Destroy.
 Arg: - <obj>[in] target object.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_Remove (gldobj_t *obj)
{
    GLDCFG_ASSERT (obj);
    gldobj_t *parent;
    
    parent = obj->Parent;
    if (parent == NULL)
        ; // questo oggetto non ha un padre. non devo rimuovelo
    else {
        gldobj_Invalidate (obj);
        gldllist_Remove (&obj->ChildEntry);
        obj->Parent = NULL;
        obj->FuncEvent (obj, GLDOBJ_EVENT_PARENT_CHANGED, parent);
        parent->FuncEvent (parent, GLDOBJ_EVENT_CHILD_LIST_CHANGED, NULL);
    }
}

/*______________________________________________________________________________ 
 Desc:  Invalidi la rappresentazione di un'oggetto. In particolare viene
        richiesto il refresh dell'area dell'oggetto. L'invalidazione viene
        portata a termine solamente se l'oggetto è effettivamente visibile.
 Arg: - <obj>[in] target object.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_Invalidate (gldobj_t *obj)
{
    gldobj_InvalidateArea (obj, &obj->Area);
}

/*______________________________________________________________________________ 
 Desc:  Invalida una porzione dell'area di un'oggetto. L'area da invalidare
        viene intersecata con l'area dell'oggetto che è effettivamente visibile.
 Arg: - <obj>[in] target object.
        <area>[in] area da invalidare.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_InvalidateArea (gldobj_t *obj, gldcoord_Area_t *area)
{
    /* invalidate only if the object is inside the root view */
    gldcoord_Area_t invalid;
    GLDCFG_ASSERT (obj);
    
    if (obj->FuncDraw != NULL)
    {
        if (gldobj_IsInside (obj, gldrefr_RootObj) &&
            ObjGetVisibleArea (obj, &invalid))
        {   /* invalido l'oggetto soltanto nell'area in cui è effettivamente visibile */
            gldcoord_AreaIntersection (&invalid, &invalid, area);
            gldrefr_InvalidateArea (&invalid);
        }
    }
    else
    {   /* l'oggetto non ha una funzione di disegno. Quindi è trasparente.
        Allora invalido ricorsivamente tutti i suoi figli */
        gldllist_Node_t *node;
        
        node = gldllist_GetFirst (&obj->ChildList);
        while (node)
        {
            gldobj_t *child;
            
            child = gldllist_Get (node);
            gldobj_InvalidateArea (child, area);
            node = gldllist_GetNext (node);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Distrugge un'oggetto. L'oggetto non sarà più utilizzabile.
    La memoria allocata per l'oggetto viene liberata.
    Ogni riferimento all'oggetto distrutto non può più essere utilizzato.
 Arg: - <obj>[in] target object.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_Destroy (gldobj_t *obj)
{
    gldllist_Node_t *node, *next = NULL;
    GLDCFG_ASSERT (obj);
    
    /* mi occupo della distruzione dei thread associati a questo oggetto */
    gldactivity_NotifyDestroy (obj);
    gldtouch_NotifyDestroy (obj);

    /* distruggo prima tutti i figli (ricorsivamente) */
    node = gldllist_GetFirst (&obj->ChildList);
    if (node != NULL)
        next = gldllist_GetNext (node);
    while (node != NULL)
    {
        gldobj_t *child = (gldobj_t *)gldllist_Get (node);
        gldobj_Destroy (child);

        node = next;
        if (node != NULL)
            next = gldllist_GetNext (node);
    }
    
    /* notifico dell'imminente distruzione. Eventuali altre risorse associate
    all'oggetto devono essere liberate ora. */
    obj->FuncEvent (obj, GLDOBJ_EVENT_DESTROY, NULL);
    gldobj_Remove (obj);
    GLDCFG_FREE (obj);
}

#ifdef GLDOBJ_INTEGER_TAG
/*______________________________________________________________________________
 Desc:  Imposta il tag di un oggetto, per poterlo cercare/recuperare con facilità
        in un secondo momento.
 Arg: - <obj>[in] target object.
        <tag>[in] codice identificativo.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetTag (gldobj_t *obj, int16_t tag)
{
    GLDCFG_ASSERT (obj);
    obj->Tag = tag;
}
#else

/*______________________________________________________________________________ 
 Desc:  Imposta il tag di un oggetto, per poterlo cercare/recuperare con facilità
        in un secondo momento.
 Arg: - <obj>[in] target object.
        <tag>[in] stringa di testo. Questa stringa viene salvata internamente
            soltanto come riferimento. Devi accertarti che la stringa che passi
            sia accessibile par tutta la vita dell'oggetto.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetTag (gldobj_t *obj, const char *tag)
{
    GLDCFG_ASSERT (obj);
    obj->Tag = tag;
}
#endif

/*______________________________________________________________________________
 Desc:  Imposta la posizione assoluta dell'oggetto all'interno del display.
        La posizione fa riferimento all'angolo top-left.
        con 0,0 l'oggetto si posiziona nell'angolo in alto a sinistra.
        Attenzione: un'oggetto è visibile soltanto all'interno dell'area in cui
        è visibile suo padre. Questo significa che se posizioni un'oggetto
        fuori dall'area di suo padre, questo non viene renderizzato, o viene
        tagliato.
 Arg: - <obj>[in] target object.
        <x>[in] coordinate angolo top-left.
        <y>[in] coordinate angolo top-left.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y)
{
    SetAbsPos (obj, x, y, true);
}

/*______________________________________________________________________________
 Desc:  Imposta la posizione relativa dell'oggetto. Le coordinate fornite sono
        relative all'angolo top-left dell'oggetto <ref>.
        Spesso, come parametro ref si utilizza il padre dell'oggetto obj.
 Arg: - <obj>[in] target object.
        <ref>[in] oggetto di riferimento.
        <x>[in] coordinate angolo top-left in pixel.
        <y>[in] coordinate angolo top-left in pixel.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetRelPos (gldobj_t *obj, gldobj_t *ref, gldcoord_t x, gldcoord_t y)
{
    GLDCFG_ASSERT (ref);
    gldobj_SetAbsPos (obj, ref->Area.X1 + x, ref->Area.Y1 + y);
}

/*______________________________________________________________________________ 
 Desc:  Modifica la dimensione dell'oggetto.
        Quando ridimensioni un'oggetto rimane costante la posizione dell'
        angolo top-left.
 Arg: - <obj> target object.
        <x_size> pxl dimensione x.
        <y_size> pixel dimensione y.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetSize (gldobj_t *obj, gldcoord_t x_size, gldcoord_t y_size)
{
    GLDCFG_ASSERT (obj);

    /* verifico se l'oggetto sar� da invalidare dopo la modifica */
    if (gldcoord_AreaWidth (&obj->Area) != x_size ||
        gldcoord_AreaHeight (&obj->Area) != y_size)
    {
        gldcoord_Area_t oldArea; // area prima di essere modificata
        gldllist_Node_t *node;
        
        gldobj_Invalidate (obj); /* invalidate the current object area */
        gldcoord_AreaCopy (&oldArea, &obj->Area);

        obj->Area.X2 = obj->Area.X1 + x_size;
        obj->Area.Y2 = obj->Area.Y1 + y_size;
        
        node = gldllist_GetFirst (&obj->ChildList);
        while (node)
        {   /* notifico i suoi figli */
            gldobj_t *child = gldllist_Get (node);
            child->FuncEvent (child, GLDOBJ_EVENT_PARENT_COORD_CHANGED, &oldArea);
            node = gldllist_GetNext (node);
        }

        obj->FuncEvent (obj, GLDOBJ_EVENT_COORD_CHANGED, &oldArea);
        if (obj->Parent)
            obj->Parent->FuncEvent (obj->Parent, GLDOBJ_EVENT_CHILD_COORD_CHANGED, obj);
        gldobj_Invalidate (obj); /* invalidate the new object area */
    }
}

/*______________________________________________________________________________
 Desc:  Imposta la dimensione di un'oggetto esattamente pari alla dimensione
        di un'altro oggetto di riferimento.
 Arg: - <obj> oggetto di cui vuoi impostare la dimensione.
        <ref> oggetto di riferimento.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetSizeAs (gldobj_t *obj, gldobj_t *ref)
{
    GLDCFG_ASSERT (obj);
    GLDCFG_ASSERT (ref);
    
    gldobj_SetSize (obj, gldcoord_AreaWidth (&ref->Area), gldcoord_AreaHeight (&ref->Area));
}

/*______________________________________________________________________________ 
 Desc:  Nascondi o rendi visibile un oggetto.
 Arg: - <obj> target object.
        <visible> true per rendere visibile l'oggetto.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_SetVisible (gldobj_t *obj, bool visible)
{
    GLDCFG_ASSERT (obj);
    
    if (obj->Hide != !visible)
    {
        obj->Hide = !visible;
        gldrefr_InvalidateArea (&obj->Area);
    }
}

/*______________________________________________________________________________ 
 Desc:  Muovi l'oggetto secondo il vettore spostamento specificato.
 Arg: - <obj> puntatore all'oggetto da muovere.
        <x_shift> spostamento sull'asse x.
        <y_shift> spostamento sull'asse y.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_Move (gldobj_t *obj, gldcoord_t x_shift, gldcoord_t y_shift)
{
    gldcoord_t new_x_pos;
    gldcoord_t new_y_pos;
    GLDCFG_ASSERT (obj);

    new_x_pos = obj->Area.X1 + x_shift;
    new_y_pos = obj->Area.Y1 + y_shift;

    gldobj_SetAbsPos (obj, new_x_pos, new_y_pos);
}

/*______________________________________________________________________________ 
 Desc:  Allineamento relativo orizzontale. Allinea l'oggetto 'obj' rispetto
        all'oggetto 'reference', nella direzione orizzontale. I punti di riferimento
        di ciascun oggetto rispetto a cui è possibile fare un allineamento reciproco
        sono left-mid-right.
        es:
            - gldobj_HAlign (obj_p, ref_p, GLDCOORD_HRALIGN_MM, 0);
               posiziona obj_p al centro di ref_p.
               (MID di obj_p è spostato di 0pxl rispetto a MID di ref_p)
            - gldobj_HAlign (obj_p, ref_p, GLDCOORD_HRALIGN_RL, 0);
               posiziona obj_p al fianco destro di ref_p. 
               (LEFT di obj_p è spostato di 0pxl rispetto a RIGHT di ref_p)
 Arg: - <reference>[in] oggetto di riferimento.
        <obj>[in] oggetto da allineare rispetto a 'reference'.
        <delta_x>[in] distanza tra i due punti di riferimento.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlign (gldobj_t *obj, gldobj_t *reference, gldcoord_HRAlign_e halign, gldcoord_t delta_x)
{
    gldcoord_t x_pos;
    GLDCFG_ASSERT (reference);
    GLDCFG_ASSERT (obj);

    switch (halign / 3)
    {
    default:
    case 0: /* reference LEFT */
        x_pos = reference->Area.X1 + delta_x;
        break;

    case 1: /* reference MID */
        x_pos = reference->Area.X1 + gldcoord_AreaWidth (&reference->Area) / 2 + delta_x;
        break;

    case 2: /* reference RIGHT */
        x_pos = reference->Area.X2 + delta_x;
        break;
    }

    switch (halign % 3)
    {
    default:
    case 0: /* obj LEFT */
        break;

    case 1: /* obj MID */
        x_pos -= gldcoord_AreaWidth (&obj->Area) / 2;
        break;

    case 2: /* obj RIGHT */
        x_pos -= gldcoord_AreaWidth (&obj->Area);
        break;
    }

    gldobj_SetAbsPos (obj, x_pos, obj->Area.Y1);
}

/*______________________________________________________________________________
 Desc:  Prende il lato sinistro di obj e lo posiziona delta_x pixel a destra
        del lato sinistro di ref.

         _______________> delta_x
        /               /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignLL (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_LL, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel a destra
        del lato sinistro di ref.

         ____________________> delta_x
        /                   /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignLM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_LM, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il lato destro di obj e lo posiziona delta_x pixel a destra
        del lato sinistro di ref.

         ________________________> delta_x
        /                       /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignLR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_LR, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il lato sinistro di obj e lo posiziona delta_x pixel a destra
        del centro di ref.

             ____________> delta_x
            /           /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignML (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_ML, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel a destra
        del centro di ref.
        Quando delta_x è 0, obj viene posizionato orizzontalmente al centro di
        ref.

             ________________> delta_x
            /               /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignMM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_MM, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il lato destro di obj e lo posiziona delta_x pixel a destra
        del centro di ref.

             ____________________> delta_x
            /                   /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignMR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_MR, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende lato sinistro di obj e lo posiziona delta_x pixel a destra
        del lato destro di ref.

                 ________> delta_x
                /       /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignRL (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_RL, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel a destra
        del lato destro di ref.

                 ____________> delta_x
                /           /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignRM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_RM, delta_x);
}

/*______________________________________________________________________________
 Desc:  Prende il lato destro di obj e lo posiziona delta_x pixel a destra
        del lato destro di ref.

                 ________________> delta_x
                /               /
        ---------       ---------
        -  ref  -       -  obj  -
        -       -       -       -
        ---------       ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HAlignRR (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_x)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_RR, delta_x);
}

/*______________________________________________________________________________ 
 Desc:  Allineamento relativo verticale. Allinea l'oggetto 'obj' rispetto
        all'oggetto 'reference', nella direzione verticale. I punti di riferimento
        di ciascun oggetto rispetto a cui è possibile fare un allineamento reciproco
        sono top-mid-bottom.
        es:
            - gldobj_HAlign (obj_p, ref_p, GLDCOORD_VRALIGN_MM, 0);
               posiziona obj_p al centro di ref_p.
               (MID di obj_p è spostato di 0pxl rispetto a MID di ref_p)
            - gldobj_HAlign (obj_p, ref_p, GLDCOORD_HRALIGN_BT, 0);
               posiziona obj_p al sotto a ref_p. 
               (TOP di obj_p è spostato di 0pxl rispetto a BOTTOM di ref_p)
 Arg: - <obj>[in] oggetto da allineare rispetto a 'reference'.
        <reference>[in] oggetto di riferimento.
        <delta_y>[in] distanza tra i due punti di riferimento.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlign (gldobj_t *obj, gldobj_t *reference, gldcoord_VRAlign_e valign, gldcoord_t delta_y)
{
    gldcoord_t y_pos;
    GLDCFG_ASSERT (reference);
    GLDCFG_ASSERT (obj);

    switch (valign / 3)
    {
    default:
    case 0: /* reference TOP */
        y_pos = reference->Area.Y1 + delta_y;
        break;

    case 1: /* reference MID */
        y_pos = reference->Area.Y1 + gldcoord_AreaHeight (&reference->Area) / 2 + delta_y;
        break;

    case 2: /* reference BOTTOM */
        y_pos = reference->Area.Y2 + delta_y;
        break;
    }

    switch (valign % 3)
    {
    default:
    case 0: /* obj TOP */
        break;

    case 1: /* obj MID */
        y_pos -= gldcoord_AreaHeight (&obj->Area) / 2;
        break;

    case 2: /* obj BOTTOM */
        y_pos -= gldcoord_AreaHeight (&obj->Area);
        break;
    }

    gldobj_SetAbsPos (obj, obj->Area.X1, y_pos);
}

/*______________________________________________________________________________
 Desc:  Prende il lato top di obj e lo posiziona delta_x pixel più in basso
        del lato top di ref.

        ---------   <--
        -  ref  -     | delta_x
        ---------     |
                      |
        ---------   <--
        -  obj  -
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignTT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_TT, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel più in basso
        del lato top di ref.

        ---------   <--
        -  ref  -     | delta_x
        ---------     |
                      |
        ---------     |
        -  obj  -   <--
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignTM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_TM, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il bottom di obj e lo posiziona delta_x pixel più in basso
        del lato top di ref.

        ---------   <--
        -  ref  -     | delta_x
        ---------     |
                      |
        ---------     |
        -  obj  -     |
        ---------   <--
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignTB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_TB, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende lato top di obj e lo posiziona delta_x pixel più in basso
        del centro di ref.

        ---------   
        -  ref  -   <--
        ---------     | delta_x
                      |
        ---------   <--
        -  obj  -
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignMT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_MT, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel più in basso
        del centro di ref.

        ---------   
        -  ref  -   <--
        ---------     | delta_x
                      |
        ---------     |
        -  obj  -   <--
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignMM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_MM, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il bottom di obj e lo posiziona delta_x pixel più in basso
        del centro di ref.

        ---------   
        -  ref  -   <--
        ---------     | delta_x
                      |
        ---------     |
        -  obj  -     |
        ---------   <--
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignMB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_MB, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il top di obj e lo posiziona delta_x pixel più in basso
        del bottom di ref.

        ---------
        -  ref  -
        ---------   <-- delta_x
                      |
        ---------   <--
        -  obj  -
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignBT (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_BT, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il centro di obj e lo posiziona delta_x pixel più in basso
        del bottom di ref.

        ---------
        -  ref  -
        ---------   <--
                      | delta_x
        ---------     |
        -  obj  -   <--
        ---------
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignBM (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_BM, delta_y);
}

/*______________________________________________________________________________
 Desc:  Prende il bottom di obj e lo posiziona delta_x pixel più in basso
        del bottom di ref.

        ---------
        -  ref  -
        ---------   <--
                      | delta_x
        ---------     |
        -  obj  -     |
        ---------   <--
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VAlignBB (gldobj_t *obj, gldobj_t *ref, gldcoord_t delta_y)
{
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_BB, delta_y);
}

/*______________________________________________________________________________
 Desc:  Posiziona questo oggetto <obj> al centro dell'oggetto fornito come
    riferimento <ref>.
Arg: - <obj>: oggetto da spostare.
       <ref>: oggetto di riferimento.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_AlignCenter (gldobj_t *obj, gldobj_t *ref)
{
    gldobj_HAlign (obj, ref, GLDCOORD_HRALIGN_MM, 0);
    gldobj_VAlign (obj, ref, GLDCOORD_VRALIGN_MM, 0);
}

/*______________________________________________________________________________
 Desc:  Allinea un'oggetto orizzontalmente rispetto ad un punto di riferimento.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_HPosAlign (gldobj_t *obj, gldcoord_t xRef, uint8_t align)
{
    gldcoord_t xPos;
    uint8_t halign;
    GLDCFG_ASSERT (obj);

    halign = GLDCOORD_GET_HALIGN (align);

    if (halign == GLDCOORD_ALIGN_HMID) {
        xPos = xRef;
        xPos -= gldobj_GetWidth (obj) / 2;

    } else if (halign == GLDCOORD_ALIGN_LEFT) {
        xPos = xRef;

    } else if (halign == GLDCOORD_ALIGN_RIGHT) {
        xPos = xRef;
        xPos -= gldobj_GetWidth (obj);
    }
    gldobj_SetAbsPos (obj, xPos, obj->Area.Y1);
}

/*______________________________________________________________________________
 Desc:  Allinea un'oggetto verticalmente rispetto ad un punto di riferimento.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_VPosAlign (gldobj_t *obj, gldcoord_t yRef, uint8_t align)
{
    gldcoord_t yPos;
    uint8_t valign;
    GLDCFG_ASSERT (obj);

    valign = GLDCOORD_GET_VALIGN (align);

    if (valign == GLDCOORD_ALIGN_VMID) {
        yPos = yRef;
        yPos -= gldobj_GetHeight (obj) / 2;

    } else if (valign == GLDCOORD_ALIGN_TOP) {
        yPos = yRef;

    } else if (valign == GLDCOORD_ALIGN_BOTTOM) {
        yPos = yRef;
        yPos -= gldobj_GetHeight (obj);
    }
    gldobj_SetAbsPos (obj, obj->Area.X1, yPos);
}

/*______________________________________________________________________________
 Desc:  Ottieni la dimenzione x dell'oggetto.
 Arg: - <obj> l'oggetto.
 Ret: - la sua larghezza.
______________________________________________________________________________*/
gldcoord_t gldobj_GetWidth (gldobj_t *obj)
{
    GLDCFG_ASSERT (obj);
    return gldcoord_AreaWidth (&obj->Area);
}

/*______________________________________________________________________________
 Desc:  Ottieni la dimenzione y dell'oggetto.
 Arg: - <obj> l'oggetto.
 Ret: - la sua altezza.
______________________________________________________________________________*/
gldcoord_t gldobj_GetHeight (gldobj_t *obj)
{
    GLDCFG_ASSERT (obj);
    return gldcoord_AreaHeight (&obj->Area);
}

#ifdef GLDOBJ_INTEGER_TAG
/*______________________________________________________________________________ 
 Desc:  Ricerca l'oggetto che corrisponde alla path specificata.
 Arg: - <root> oggetto radice della ricerca.
        <...> una sequenza di tag separati da virgola (,) che costituiscono
              in questo modo una path. Devi terminare la sequanza con uno 0.
 Es:
    Ipotizzando che tu abbia gia' correttamente taggato gli oggetti
    utilizzando i tag TAG_BTN1, TAG_IMG, TAG_BTN3, TAG_LBL.
    gldobj_GetByTag (gldrefr_UserLayer, TAG_BTN1, TAG_IMG, TAG_END)
        per ottenere l'oggetto immagine contenuto nel bottone 1.
    gldobj_GetByTag (gldrefr_UserLayer, TAG_BTN3, TAG_LBL, TAG_END)
        per ottenere la label contenuta nel bottone 3.
 Ret: - Ritorna il primo oggetto che corrisponde alla path. Altrimenti NULL.
______________________________________________________________________________*/
gldobj_t *gldobj_GetByTag (gldobj_t *root, ...)
{
    gldobj_t *objFound = NULL;
    GLDCFG_ASSERT (root);

    va_list argPtr;
    uint8_t numArgs = 0;
    int16_t tag;
    
    /* calcolo il numero degli alrgomenti opzionali */
    va_start (argPtr, root);
    tag = va_arg (argPtr, int);
    while (tag != 0)
    {
        numArgs++;
        tag = va_arg (argPtr, int);
    }
    va_end (argPtr);
    
    {
        int16_t tagList[numArgs];
        
        va_start (argPtr, root);
        for (uint8_t i = 0; i < numArgs; i++)
            tagList[i] = va_arg (argPtr, int);
        va_end (argPtr);
        objFound = ObjGetByTag (root, numArgs, tagList);
    }

    return objFound;
}
#else

/*______________________________________________________________________________ 
 Desc:  Ricerca l'oggetto che corrisponde alla path specificata.
 Arg: - <root> oggetto radice della ricerca.
        <path> una composizione di tag separati dal carattere '/'.
               es. "bt1/img" "bt3/lbl" per identificare rispettivamente l'oggetto
               immagine contenuto nell'oggetto bottone 1 e l'oggetto label
               contenuto nell'oggetto bottone 3.
 Ret: - Ritorno l'oggetto con il tag corrispondente se esiste, altimenti NULL.
______________________________________________________________________________*/
gldobj_t *gldobj_GetByTag (gldobj_t *root, const char *tag)
{
    GLDCFG_ASSERT (root);
    return ObjGetByTag (root, tag);
}
#endif

/*______________________________________________________________________________ 
 Desc:  Ottieni l'oggetto (fratello) successivo.
 Arg: - <obj> oggetto di cui vuoi ottenere il fratello successivo.
 Ret: - riferimento al fratello successivo di questo oggetto.
______________________________________________________________________________*/
gldobj_t *gldobj_GetNext (gldobj_t *obj)
{
    gldobj_t *next = NULL;
    GLDCFG_ASSERT (obj);
    
    gldllist_Node_t *node;

    /* ottengo il nodo successivo a questo */
    node = gldllist_GetNext (&obj->ChildEntry);
    if (node != NULL)
        next = gldllist_Get (node); /* prendo puntatore ad oggetto */
    return next;
}

/*______________________________________________________________________________ 
Desc:  Ottieni l'oggetto (fratello) precedente.
 Arg: - <obj> oggetto di cui vuoi ottenere il fratello precedente.
 Ret: - riferimento al fratello precedente di questo oggetto.
______________________________________________________________________________*/
gldobj_t *gldobj_GetPrev (gldobj_t *obj)
{
    gldobj_t *prev = NULL;
    gldllist_Node_t *node;
    GLDCFG_ASSERT (obj);

    /* ottengo il nodo precedente a questo */
    node = gldllist_GetPrev (&obj->ChildEntry);
    if (node != NULL)
        prev = gldllist_Get (node); /* prendo puntatore ad oggetto */
    return prev;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'n-esimo figlio dell'oggetto (0 = primo figlio).
        Con indece negativo ottieni il figlio con indice <numer figli + num>,
        comodo per ottenere l'ultimo figlio (passando come indice -1) o per
        ciclare la lista dei figli al contrario.
        Per primo figlio si intende il figlio piu' in background, cioe' quello
        aggiunto meno di recente a suo padre.
 Arg: - <parent> puntatore all'oggetto padre.
        <num> indice del figlio.
 Ret: - Restituisce puntatore all'n-esimo figlio se disponibile, altrimenti NULL.
______________________________________________________________________________*/
gldobj_t *gldobj_GetChild (gldobj_t *parent, int16_t num)
{
    gldobj_t *nChild = NULL;
    GLDCFG_ASSERT (parent);

    gldllist_Node_t *node;

    if (num >= 0)
    { /* ciclo la lista dei figli partendo dalla testa, fino ad ottenere
         l'ennesimo */
        node = gldllist_GetLast (&parent->ChildList);
        while (node != NULL)
        {
            if (num-- == 0)
            { /* n-esimo figlio trovato */
                nChild = gldllist_Get (node);
                break;
            }
            node = gldllist_GetPrev (node);
        }
    } else
    { /* ciclo la lista dei figli partendo dalla coda, fino ad ottenere
         il figlio di indice <numero figli + num> */
        node = gldllist_GetFirst (&parent->ChildList);
        while (node != NULL)
        {
            if (++num == 0)
            { /* n-esimo figlio trovato */
                nChild = gldllist_Get (node);
                break;
            }
            node = gldllist_GetNext (node);
        }
    }
    return nChild;
}

/*______________________________________________________________________________ 
Desc:  Verifica se l'oggetto 'obj' all'interno dell'oggetto 'container': quindi
       pari a container o figlio (di qualsiasi grado) di esso.
       Vuoi verificare se obj é discendente di container.
 Arg: - <obj> l'oggetto.
        <container> il potenziale antenato di obj.
 Ret: - None.
______________________________________________________________________________*/
bool gldobj_IsInside (gldobj_t *obj, gldobj_t *container)
{
    bool is_in = false;
    GLDCFG_ASSERT (obj);
    GLDCFG_ASSERT (container);

    if (obj == container)
        is_in = true;
    else
    {
        /* check against the container's childs */
        for (gldllist_Node_t *node = gldllist_GetFirst (&container->ChildList);
             node != NULL;
             node = gldllist_GetNext (node))
        {
            if (gldobj_IsInside (obj, gldllist_Get (node)))
            {
                is_in = true;
                break;
            }
        }
    }
    return is_in;
}

/*______________________________________________________________________________ 
 Desc:  Porta in primo piano l'oggetto indicato.
        In pratica riordino la lista dei figli contenuta nel padre di questo
        oggetto mettendolo primo nella lista.
 Arg: - <obj>[in] Oggetto da portare in primo piano.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_ToForeground (gldobj_t *obj)
{
    GLDCFG_ASSERT (obj);
    
    if (obj->Parent != NULL)
    { /* verifico che l'oggetto sia effettivamente all'interno della lista
         di figli del padre */
        gldllist_Node_t *node = &obj->ChildEntry;
        gldllist_t *llist = &obj->Parent->ChildList;

        if (gldllist_IsContained (node, llist))
        { /* rimuovo il nodo di questo oggetto dalla lista e lo aggiungo in testa */
            gldllist_Remove (node);
            gldllist_AddFirst (llist, node);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Porta in secondo piano l'oggetto indicato.
        In pratica riordino la lista dei figli all'interno padre di questo
        oggetto mettendolo ultimo nella lista.
 Arg: - <obj>[in] Oggetto da portare in secondo piano.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_ToBackground (gldobj_t *obj)
{
    GLDCFG_ASSERT (obj);
    
    if (obj->Parent != NULL)
    { /* verifico che l'oggetto sia effettivamente all'interno della lista
         di figli del padre */
        gldllist_Node_t *node = &obj->ChildEntry;
        gldllist_t *llist = &obj->Parent->ChildList;

        if (gldllist_IsContained (node, llist))
        { /* rimuovo il nodo di questo oggetto dalla lista e lo aggiungo in coda */
            gldllist_Remove (node);
            gldllist_AddLast (llist, node);
        }
    }
}

/*______________________________________________________________________________
 Desc:  Verifica se l'oggetto <obj> ha il <tag> specificato.
 Arg: - <obj> l'oggetto.
        <tag> il tag da controllare
 Ret: - true se <obj> ha il tag <tag>.
______________________________________________________________________________*/
bool gldobj_TagMatch (gldobj_t *obj, const char *tag)
{
    GLDCFG_ASSERT (obj);
    
    if (obj->Tag && strcmp(obj->Tag, tag) == 0)
        return true;
    return false;
}

/*______________________________________________________________________________
 Desc:  Ottieni in numero di fratelli che precedono questo oggetto, nella lista
        dei figli di suo padre. Con il termine precedono intendo i fratelli di
        questo oggetto che sono piu' in background.
 Arg: - <obj>[in] puntatore oggetto.
 Ret: - None.
______________________________________________________________________________*/
int32_t gldobj_GetChildIdx (gldobj_t *obj)
{
    int32_t idx = 0;
    gldllist_Node_t *node;
    GLDCFG_ASSERT (obj);
    
    node = gldllist_GetNext (&obj->ChildEntry);
    while (node)
    {
        idx ++;
        node = gldllist_GetNext (node);
    }
    return idx;
}

/*______________________________________________________________________________ 
 Desc:  Una funzione di gestione eventi dummy.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void gldobj_FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Ottieni le coordinate dell'area effettivamente visibile dell'oggetto.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static bool ObjGetVisibleArea (gldobj_t *obj, gldcoord_Area_t *area)
{
    bool isVisible = false;

    /* inizzializzo le variabili del ciclo */
    isVisible = true;
    gldcoord_AreaCopy (area, &obj->Area);
#if (0)
    /* ottengo l'area su cui vorrebbe disegnare l'oggetto */
    ObjGetObjDrawArea (obj, area);
#endif

    /* Inerseco l'area ottenuta con tutti i suoi fino alla radice. */
    while (obj != NULL)
    {
        if (obj->Hide == true)
        {
            isVisible = false;
            break;
        } else
        {
            if (gldcoord_AreaIntersection (area, area, &obj->Area) == true)
            { /* almeno una porzione dell'area dell'oggetto interseca
                 quella del padre */
                obj = obj->Parent;
            } else
            { /* l'area dell'oggetto non interseca quella del padre quindi
                 non è visibile */
                isVisible = false;
                break;
            }
        }
    }
    return isVisible;
}

#if (0)
/*______________________________________________________________________________
 Desc:  Ottieni l'area dove questo oggetto, assieme ai suoi figli, intende
        disegnare. Senza considerare il crop introdotto dagli antenati
        dell'oggetto.
 Arg: - <obj>[in] puntatore ad oggetto.
        <area>[out] area rappresentata.
 Ret: - None.
______________________________________________________________________________*/
static void ObjGetObjDrawArea (gldobj_t *obj, gldcoord_Area_t *area)
{
    if (obj->FuncDraw != NULL)
    {   /* se ha una funzione di disegno, considero che tutta l'area
        dell'oggetto verrà disegnata */
        gldcoord_AreaCopy (area, &obj->Area);
    }
    else
    {   /* se non ha una funzione di disegno, ispeziono i suoi figli, e cerco la
        più piccola area che contiene le aree di disegno di tutti i suoi figli.
        queste sono ottenute allo stesso modo, ricorsivamente */
        gldllist_Node_t *node;
        
        memset (area, 0, sizeof (gldcoord_Area_t)); // area inizialmente nulla
        node = gldllist_GetFirst (&obj->ChildList);
        while (node)
        {
            gldcoord_Area_t childArea;
            gldobj_t *child;
            
            child = gldllist_Get (node);
            ObjGetObjDrawArea (child, &childArea);
            
            /* crop child area to this object */
            gldcoord_AreaIntersection (&childArea, &childArea, &obj->Area);
            /* aggiungo l'area di questo figlio all'area totale */
            gldcoord_AreaJoin (area, area, &childArea);
            node = gldllist_GetNext (node);
        }
    }
}
#endif

#ifdef GLDOBJ_INTEGER_TAG
/*______________________________________________________________________________ 
 Desc:  Ricerca dalla radice 'root' la path specificata.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static gldobj_t *ObjGetByTag (gldobj_t *root, uint8_t listSize, int16_t *tagList)
{
    gldobj_t *obj_found = NULL;
    
    if (root->Tag == tagList[0])
    {   /* il tag di questo oggetto corrisponde al primo della lista */
        if (listSize == 1)
        {   /* ho identificato una path completa */
            obj_found = root;
        }
        else
        {
            listSize--;
            tagList++;
        }
    }
    
    if (obj_found == NULL && listSize)
    {   /* cerco nei figli di questo oggetto */
        /* cerco la path rimanente tra i figli dell'oggetto */
        gldllist_Node_t *node;

        node = gldllist_GetFirst (&root->ChildList);
        while (node != NULL)
        {
            obj_found = ObjGetByTag (gldllist_Get (node), listSize, tagList);
            if (obj_found != NULL)
                break;
            node = gldllist_GetNext (node);
        }
    }
    return obj_found;
}
#else

/*______________________________________________________________________________ 
 Desc:  Ricerca dalla radice 'root' la path specificata.
 Arg: - <root> oggetto da cui cominciare la ricerca.
        <path> stringa che descrive la path dell'oggetto da cercare.
 Ret: - Il primo oggetto trovato che corrisponde alla path.
______________________________________________________________________________*/
static gldobj_t *ObjGetByTag (gldobj_t *root, const char *path)
{
    gldobj_t *obj_found = NULL;
    const char *child_path = path;
    
    if (root->Tag != NULL && root->Tag[0] != '\0')
    {   /* verifico se il tag di questo oggetto corrisponde al primo tag
           della path */
        size_t tag_size;
        const char *tag_end;
        
        if ((tag_end = strstr (path, "/")) != 0)
            tag_size = tag_end - path;
        else
            tag_size = strlen (path);
        
        if (strlen (root->Tag) == tag_size &&
            strncmp (root->Tag, path, tag_size) == 0)
        {   /* il tag corrisponde */
            if (tag_end == NULL)
            {   /* se il tag era l'ultimo della path ho identificato l'oggetto */
                obj_found = root;
                child_path = NULL;
            }
            else
            {   /* riduco la path al segmento di tag rimanenti */
                child_path += tag_size + 1;
            }
        }
    }
    
    /* ancora una porzione di path rimanente */
    if (child_path != NULL)
    {   /* cerco la path rimanente tra i figli dell'oggetto */
        gldllist_Node_t *node;
        
        node = gldllist_GetFirst (&root->ChildList);
        while (node != NULL)
        {
            gldobj_t *child = gldllist_Get (node);
            if ((obj_found = ObjGetByTag (child, child_path)) != NULL)
                break;
            
            node = gldllist_GetNext (node);
        }
    }
    
    return obj_found;
}
#endif

/*______________________________________________________________________________
 Desc:  Imposta la posiziona assoluta di un'oggetto.
 Arg: - <obj> l'oggetto di cui impostare la posizione.
        <x> pos. x in pxl.
        <y> pos. y in pxl.
        <notifyParent> se true, il padre di questo oggetto viene notificato
        dello spostamento del figlio.
 Ret: - None.
______________________________________________________________________________*/
static void SetAbsPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y, bool notifyParent)
{
    bool invalid = false;
    gldcoord_t move_x, move_y;
    GLDCFG_ASSERT (obj);

    move_x = x - obj->Area.X1;
    move_y = y - obj->Area.Y1;

    /* verifico se l'oggetto sar� da invalidare dopo la modifica */
    if (move_x != 0 || move_y != 0)
        invalid = true;

    if (invalid == true)
    {
        gldllist_Node_t *node;
        gldcoord_Area_t oldArea; // area prima di essere modificata

        gldobj_Invalidate (obj); /* invalidate the current object area */
        gldcoord_AreaCopy (&oldArea, &obj->Area);

        obj->Area.X1 += move_x;
        obj->Area.X2 += move_x;
        obj->Area.Y1 += move_y;
        obj->Area.Y2 += move_y;

        /* move all the child the same way. */
        node = gldllist_GetFirst (&obj->ChildList);
        while (node)
        {
            gldobj_t *child = gldllist_Get (node);
            SetAbsPos (child, child->Area.X1 + move_x, child->Area.Y1 + move_y, false);
            child->FuncEvent (child, GLDOBJ_EVENT_PARENT_COORD_CHANGED, &oldArea);
            node = gldllist_GetNext (node);
        }
        
        obj->FuncEvent (obj, GLDOBJ_EVENT_COORD_CHANGED, &oldArea);
        if (notifyParent && obj->Parent)
            obj->Parent->FuncEvent (obj->Parent, GLDOBJ_EVENT_CHILD_COORD_CHANGED, obj);
        gldobj_Invalidate (obj); /* invalidate the new object area */
    }
}
