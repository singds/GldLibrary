//===================================================================== INCLUDES
#include "GldMisc/GldLlist.h"

#include "GldCfg.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Inizializza la linked list.
 Arg: - <llist> linked list da inizializzare.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_Init (gldllist_t *llist)
{
    llist->First = NULL;
    llist->Last = NULL;
    llist->Size = 0;
}

/*______________________________________________________________________________ 
 Desc:  Inizializza il nodo della linked list.
 Arg: - <node> nodo da inizializzare.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_NodeInit (gldllist_Node_t *node)
{
    node->Owner = NULL;
    node->Next = NULL;
    node->Prev = NULL;
    node->Element = NULL;
}

/*______________________________________________________________________________ 
 Desc:  Aggiunge un nodo come primo della lista.
 Arg: - <llist> linked list.
        <node> nodo da aggiungere.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_AddFirst (gldllist_t *llist, gldllist_Node_t *node)
{
    GLDCFG_ASSERT (llist);
    GLDCFG_ASSERT (node);
    
    if (node->Owner)
        ; // questo nodo � gi� contenuto in un'altra lista
    else {
        node->Owner = llist;
        node->Prev = NULL;
        node->Next = llist->First;
        if (llist->First != NULL)
        {
            llist->First->Prev = node;
        }
        llist->First = node;
        if (llist->Last == NULL)
        {
            llist->Last = node;
        }
        llist->Size++;
    }
}

/*______________________________________________________________________________ 
 Desc:  Aggiunge un nodo come ultimo della lista.
 Arg: - <llist> linked list.
        <node> nodo da aggiungere.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_AddLast (gldllist_t *llist, gldllist_Node_t *node)
{
    GLDCFG_ASSERT (llist);
    GLDCFG_ASSERT (node);
    
    if (node->Owner)
        ; // questo nodo � gi� contenuto in un'altra lista
    else {
        node->Owner = llist;
        node->Prev = llist->Last;
        node->Next = NULL;
        if (llist->Last != NULL)
        {
            llist->Last->Next = node;
        }
        llist->Last = node;
        if (llist->First == NULL)
        {
            llist->First = node;
        }
        llist->Size++;
    }
}

/*______________________________________________________________________________ 
 Desc:  Aggiungi un nodo alla lista, in posizione precedente al nodo specificato
        come riferimento.
 Arg: - <ref> nodo di riferimento.
        <node> nodo da aggiungere. viene aggiunto prima di <ref>.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_AddBefore (gldllist_Node_t *ref, gldllist_Node_t *node)
{
    GLDCFG_ASSERT (ref);
    GLDCFG_ASSERT (node);
    
    /* il nodo di riferimento deve appartenere a una lista.
    il nuovo nodo non deve essere contenuto in nessuna lista */
    if (node->Owner != NULL || ref->Owner == NULL)
        ;
    else {
        gldllist_t *llist;
        
        llist = ref->Owner;
        if (ref == llist->First)
            gldllist_AddFirst (llist, node);
        else {
            node->Owner = llist;
            node->Next = ref;
            node->Prev = ref->Prev;
            if (ref->Prev)
                ref->Prev->Next = node;
            ref->Prev = node;
            llist->Size++;
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Aggiungi un nodo alla lista, in posizione successiva al nodo specificato
        come riferimento.
 Arg: - <ref> nodo di riferimento.
        <node> nodo da aggiungere. viene aggiunto dopo <ref>.
 Ret: - None.
______________________________________________________________________________*/
void gldllist_AddAfter (gldllist_Node_t *ref, gldllist_Node_t *node)
{
    GLDCFG_ASSERT (ref);
    GLDCFG_ASSERT (node);
    
    /* il nodo di riferimento deve appartenere a una lista.
    il nuovo nodo non deve essere contenuto in nessuna lista */
    if (node->Owner != NULL || ref->Owner == NULL)
        ;
    else {
        gldllist_t *llist;
        
        llist = ref->Owner;
        if (ref == llist->Last)
            gldllist_AddLast (llist, node);
        else {
            node->Owner = llist;
            node->Prev = ref;
            node->Next = ref->Next;
            if (ref->Next)
                ref->Next->Prev = node;
            ref->Next = node;
            llist->Size++;
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Rimuove il nodo dalla lista a cui appartiene. Se non appartiene a
        nessuna lista, la funzione non ha alcun effetto.
 Arg: - <node> nodo da rimuovere.
 Ret: - None.
______________________________________________________________________________*/
void *gldllist_Remove (gldllist_Node_t *node)
{
    void *element = NULL;
    GLDCFG_ASSERT (node);
    
    if (node->Owner == NULL)
        ; // questo nodo non appartiene a nessuna lista. non posso rimuoverlo
    else {
        gldllist_t *llist;
        
        llist = node->Owner;
        if (node == llist->First)
        {   // se sto rimuovendo il primo della lista
            llist->First = node->Next;
            if (llist->First == NULL)
            {
                llist->Last = NULL;
            } else
            {
                llist->First->Prev = NULL;
            }
        } else if (node == llist->Last)
        {   // se sto rimuovendo l'ultimo della lista
            llist->Last = node->Prev;
            if (llist->Last == NULL)
            {
                llist->First = NULL;
            } else
            {
                llist->Last->Next = NULL;
            }
        } else
        {   // sto rimuovendo un elemento centrale
            node->Next->Prev = node->Prev;
            node->Prev->Next = node->Next;
        }
        node->Owner = NULL;
        node->Next = NULL;
        node->Prev = NULL;
        element = node->Element;
        llist->Size--;
    }
    return element;
}

/*______________________________________________________________________________
 Desc:  Imposta il contenuto del nodo. Restutuisce il contenuto precedente.
 Arg: - <node> imposta il contenuto di questo nodo.
        <element> nuovo contenuto del nodo.
 Ret: - Precedente contenuto del nodo.
______________________________________________________________________________*/
void *gldllist_Set (gldllist_Node_t *node, void *element)
{
    void *lastElement;
    GLDCFG_ASSERT (node);
    
    lastElement = node->Element;
    node->Element = element;
    return lastElement;
}

/*______________________________________________________________________________
 Desc:  Ottieni il contenuto di un nodo.
 Arg: - <node> riferimento al nodo.
 Ret: - Contenuto del nodo.
______________________________________________________________________________*/
void *gldllist_Get (const gldllist_Node_t *node)
{
    GLDCFG_ASSERT (node);
    
    return node->Element;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la dimensione della linked list.
 Arg: - <llist> riferimento alla linked list.
 Ret: - Numero di nodi contenuti.
______________________________________________________________________________*/
uint32_t gldllist_Size (const gldllist_t *llist)
{
    GLDCFG_ASSERT (llist);
    
    return llist->Size;
}

/*______________________________________________________________________________ 
 Desc:  Verifica se la linked list � vuota.
 Arg: - <llist> riferimento alla llist.
 Ret: - true se la lista � vuota. false altrimenti.
______________________________________________________________________________*/
bool gldllist_IsEmpty (const gldllist_t *llist)
{
    GLDCFG_ASSERT (llist);
    
    return (gldllist_Size (llist) == 0);
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il primo nodo della linked list.
 Arg: - <llist> riferimento alla llist.
 Ret: - Riferimento al primo nodo.
______________________________________________________________________________*/
gldllist_Node_t *gldllist_GetFirst (const gldllist_t *llist)
{
    GLDCFG_ASSERT (llist);
    
    return llist->First;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'ultimo nodo della linked list.
 Arg: - <llist> riferimento alla llist.
 Ret: - Riferimento all'ultimo nodo.
______________________________________________________________________________*/
gldllist_Node_t *gldllist_GetLast (const gldllist_t *llist)
{
    GLDCFG_ASSERT (llist);
    
    return llist->Last;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni nodo successivo a questo.
 Arg: - <node> nodo di riferimento.
 Ret: - Nodo successivo.
______________________________________________________________________________*/
gldllist_Node_t *gldllist_GetNext (const gldllist_Node_t *node)
{
    GLDCFG_ASSERT (node);
    
    return node->Next;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni nodo precedente a questo.
 Arg: - <node> nodo di riferimento.
 Ret: - Nodo precedente.
______________________________________________________________________________*/
gldllist_Node_t *gldllist_GetPrev (const gldllist_Node_t *node)
{
    GLDCFG_ASSERT (node);
    
    return node->Prev;
}

/*______________________________________________________________________________ 
 Desc:  Verifica se un nodo � contenuto in una certa lista.
 Arg: - <node> nodo da verificare.
        <llist> lista che potrebbe contenere <node>.
 Ret: - true se <llist> contiene il nodo <node>.
______________________________________________________________________________*/
bool gldllist_IsContained (const gldllist_Node_t *node, const gldllist_t *llist)
{
    return (node->Owner == llist);
}

//============================================================ PRIVATE FUNCTIONS
