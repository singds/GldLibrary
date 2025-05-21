#ifndef GLDLLIST_H_INCLUDED
#define GLDLLIST_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

//====================================================================== DEFINES
struct gldllist_s;

typedef struct gldllist_Node_s
{
    struct gldllist_s *Owner;         // lista a cui appartiene questo nodo 
    struct gldllist_Node_s *Next;     /* nodo successivo */
    struct gldllist_Node_s *Prev;     /* nodo precedente */
    void *Element;                    /* elemento del nodo */
} gldllist_Node_t;

typedef struct gldllist_s
{
    gldllist_Node_t *First; /* nodo di testa della lista */
    gldllist_Node_t *Last;  /* nodo di coda della lista */
    uint32_t Size;          /* numero di nodi della lista */
} gldllist_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void gldllist_Init (gldllist_t *llist);
void gldllist_NodeInit (gldllist_Node_t *node);

void gldllist_AddFirst (gldllist_t *llist, gldllist_Node_t *node);
void gldllist_AddLast (gldllist_t *llist, gldllist_Node_t *node);
void gldllist_AddBefore (gldllist_Node_t *ref, gldllist_Node_t *node);
void gldllist_AddAfter (gldllist_Node_t *ref, gldllist_Node_t *node);
void *gldllist_Remove (gldllist_Node_t *node);
void *gldllist_Set (gldllist_Node_t *node, void *element);
void *gldllist_Get (const gldllist_Node_t *node);

/* metodi di ispezione */
uint32_t gldllist_Size (const gldllist_t *llist);
bool gldllist_IsEmpty (const gldllist_t *llist);
gldllist_Node_t *gldllist_GetFirst (const gldllist_t *llist);
gldllist_Node_t *gldllist_GetLast (const gldllist_t *llist);
gldllist_Node_t *gldllist_GetNext (const gldllist_Node_t *node);
gldllist_Node_t *gldllist_GetPrev (const gldllist_Node_t *node);

bool gldllist_IsContained (const gldllist_Node_t *node, const gldllist_t *llist);


#endif /* GLDLLIST_H_INCLUDED */
