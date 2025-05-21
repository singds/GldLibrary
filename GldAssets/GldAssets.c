//===================================================================== INCLUDES
#include "GldAssets/GldAssets.h"

#include "GldAssets/GldActivity.h"
#include "GldAssets/ObjButton.h"
//s#include "GldAssets/GldChart.h"
#include "GldAssets/ObjCustom.h"
#include "GldAssets/ObjImage.h"
#include "GldAssets/ObjLabel.h"
#include "GldAssets/GldListBox.h"
#include "GldAssets/ObjNumber.h"
#include "GldAssets/ObjProgressBar.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Verifica se l'oggetto fornito appartiene � di una certa classe.
 Arg: - <obj> l'oggetto da controllare.
        <type> stringa che identifica il tipo di oggetto.
 Ret: - NULL se l'oggetto non appartiene a quella classe. Il puntatore
        all'oggetto stesso altrimenti.
______________________________________________________________________________*/
void *gldassets_CheckType (const gldobj_t *obj, const char *type)
{
    void *objTyped = NULL;
    
    if (obj != NULL)
    {
        const gldobj_Class_t *objClass;
        
        objClass = obj->Class;
        while (objClass != NULL && strcmp (objClass->Name, type) != 0)
            objClass = objClass->SuperClass;
        if (objClass != NULL)
            objTyped = (void *)obj;
    }
    return objTyped;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'n-esimo asset del tipo 'type', figlio dell'oggetto specificato.
 Arg: - <obj> l'oggetto di cui prendere il figlio.
        <type> il tipo di figlio da prendere.
        <index> l'indice del figlio del tipo specificato da prendere.
            se type="GldImage" e index = 3, la funzione restituisce la quarta
            immagine figlia di <obj>.
 Ret: - L'oggetto recuperato se esiste, NULL altrimenti.
______________________________________________________________________________*/
void *gldassets_PickChild (const gldobj_t *obj, const char *type, uint16_t index)
{
    void *objFound = NULL;

    gldllist_Node_t *node;

    if (obj != NULL)
    { /* cerco tra i figli di questo oggetto */
        node = gldllist_GetFirst (&obj->ChildList);
        while (node != NULL)
        {
            gldobj_t *child;
            child = (gldobj_t *)gldllist_Get (node);
            /* verifico se il tipo del figlio corrisponde */
            if (gldassets_CheckType (child, type) != NULL)
            {
                if (index-- == 0)
                {
                    objFound = child;
                    break;
                }
            }

            node = gldllist_GetNext (node);
        }
    }
    return objFound;
}

//============================================================ PRIVATE FUNCTIONS
