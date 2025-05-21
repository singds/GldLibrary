//===================================================================== INCLUDES
#include "GldAssets/GldActivity.h"

#include "GldCore/GldTouch.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static inline int32_t ActivityNotify (gldactivity_t *activity, gldactivity_Event_e evt, void *ext);

static void StackPush (void *ptr, size_t size);
static void StackPop (void *ptr, size_t size);

//============================================================= STATIC VARIABLES
// lo stack delle pagine
static uint8_t Stack[GLDCFG_ACTIVITY_STACK_SIZE];
static int32_t StackPtr; // stack pointer [punta alla cima dello stack (prima posizione libera)]
static int32_t FramePtr; // frame pointer [punta all'inizio del frame dell'activity corrente]
/*
    pd = page data
    pd-1 = page data for previous page
    ac = activity object
    ac-1 = activity object for previous page
    
    |    |   <-- StackPtr      |
    |pd  |                     |
    |pd  |                     |
    |pd  |                     |
    |ac  |   <-- FramePtr   <--| current activity
    |fp-1| -----|              |
    |pd-1|      |              |
    |pd-1|      |              |
    |pd-1|      |              |
    |ac-1| <----|           <--| previous activity
*/

static gldllist_t ThreadList;
static bool ThreadListModified;

//============================================================= GLOBAL VARIABLES
gldactivity_t *gldactivity_Curr; /* Current running activity */

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Crea e rendi visibile una nuova activity.
 Arg: - <attr>[in] puntatore ad attributi dell'activity.
        <trace>[in] true se vuoi salvare l'activity corrente nello stack delle
            pagine per poterci tornare successivamente con la funzione
            'gldactivity_Prev'.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_Next (const gldactivity_Attr_t *attr, bool trace)
{
    gldactivity_t *nextAct;
    GLDCFG_ASSERT (attr);
    
    if (gldactivity_Curr != NULL)
    {   // c'� un'activity attiva
        if (trace == true)
        {   /* sospendo activity corrente*/
            ActivityNotify (gldactivity_Curr, GLDACTIVITY_EVENT_SUSPEND, NULL);
            gldobj_Destroy (gldactivity_Curr->RootObj);
            StackPush (&FramePtr, sizeof (FramePtr));
            FramePtr = StackPtr;
        }
        else
        {   /* distruggo l'activity corrente */
            ActivityNotify (gldactivity_Curr, GLDACTIVITY_EVENT_DESTROY, NULL);
            gldobj_Destroy (gldactivity_Curr->RootObj);
            GLDCFG_FREE (gldactivity_Curr);
            gldactivity_Curr = NULL;
            StackPtr = FramePtr;
            /* the visible object tree is destroyed */
        }
    }
    nextAct = GLDCFG_CALLOC (1, sizeof (gldactivity_t));
    if (nextAct != NULL)
    {
        nextAct->Attr = attr;
        nextAct->RootObj = NULL;
        nextAct->UsrParam = NULL;
        
        gldactivity_Curr = nextAct;

        StackPush (&gldactivity_Curr, sizeof (gldactivity_t *));
        ActivityNotify (gldactivity_Curr, GLDACTIVITY_EVENT_CREATE, NULL);
        gldactivity_Curr->RootObj = attr->FuncBuild (gldactivity_Curr); // creo albero oggetti
        
        /* la radice dell'activity dovrebbe essere l'unico oggetto nello user layer */
        gldobj_Add (gldactivity_Curr->RootObj, gldrefr_UserLayer);
    }
}

/*______________________________________________________________________________ 
 Desc:  Crea e rendi visibile l'activity precedente: per activity precedente
        si intende la prima activity nello stack delle pagine.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_Prev (void)
{
    if (FramePtr == 0)
        return; // no back page available
    
    /* distruggo la pagina corrente e ripristino la prima dello stack */
    ActivityNotify (gldactivity_Curr, GLDACTIVITY_EVENT_DESTROY, NULL);
    gldobj_Destroy (gldactivity_Curr->RootObj);
    GLDCFG_FREE (gldactivity_Curr);
    gldactivity_Curr = NULL;
    StackPtr = FramePtr;

    StackPop (&FramePtr, sizeof (FramePtr));
    
    memcpy (&gldactivity_Curr, &Stack[FramePtr], sizeof (gldactivity_t *));
    ActivityNotify (gldactivity_Curr, GLDACTIVITY_EVENT_RESUME, NULL);
    gldactivity_Curr->RootObj = gldactivity_Curr->Attr->FuncBuild (gldactivity_Curr);
    
    /* la radice dell'activity dovrebbe essere l'unico oggetto nello user layer */
    gldobj_Add (gldactivity_Curr->RootObj, gldrefr_UserLayer);
}

/*______________________________________________________________________________
 Desc:  Cancella l'intero contenuto dello stack delle pagine.
    Se chiami <gldactivity_Prev> dopo questa funzione, <gldactivity_Prev> non
    fa nulla, perché non c'è nessuna pagina nello nello stack a cui tornare.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_StackFlush (void)
{
    gldactivity_t *oldActivity;
    while (FramePtr != 0)
    {
        StackPtr = FramePtr;
        StackPop (&FramePtr, sizeof (FramePtr));
        memcpy (&oldActivity, &Stack[FramePtr], sizeof (gldactivity_t *));
        ActivityNotify (oldActivity, GLDACTIVITY_EVENT_DESTROY, NULL);
        GLDCFG_FREE (oldActivity);
    }
    StackPtr = FramePtr;
}

/*______________________________________________________________________________ 
 Desc:  Aggiunge una porzione di memoria allo stack delle pagine.
    Questa funzione può essere chiamata prima di cambiare pagina, in modo da
    salvare alcuni dati che possono essere ripristinati quando si torna nella
    pagina.
    Puoi pushare per esempio alcune variabili che dopo ripristini con <gldactivity_StackPop>.
 Arg: - <ptr> indirizzo della memoria da salvare.
        <size> size in byte della memoria.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_StackPush (void *ptr, size_t size)
{
    GLDCFG_ASSERT ((StackPtr + size) <= sizeof (Stack));
    StackPush (ptr, size);
}

/*______________________________________________________________________________ 
 Desc:  Recupera una porzione di memoria dallo stack delle pagine.
    Questa funzione può essere chiamata quando si ritorna in una pagina, in modo
    da ripristinare alcuni dati che sono stati precedentemente salvati nello stack.
 Arg: - <ptr> indirizzo di memoria, dove recuperare i dati.
        <size> size in byte della memoria da recuperare.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_StackPop (void *ptr, size_t size)
{
    GLDCFG_ASSERT ((StackPtr - (FramePtr + sizeof (gldactivity_t *))) >= size);
    StackPop (ptr, size);
}

/*______________________________________________________________________________ 
 Desc:  Polling che gestisce l'esecuzione dei task associati all'activity.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_Task (void)
{ /* eseguio il task specifico dell'activity corrente */
    if (gldactivity_Curr != NULL)
    { /* eseguo default thread dell'activity */
        gldactivity_Curr->Attr->FuncTask (gldactivity_Curr);
    }

    { /* processo gli altri thread della pagina */
        /* per verificare se la lista subisce modifiche durente l'esecuzione dei
        thread */
        ThreadListModified = false;

        gldllist_Node_t *nodeTh = gldllist_GetFirst (&ThreadList);
        while (nodeTh)
        {
            /* salvo il prossimo. l'esecuzione di un thread potrebbe rimuovere
           s� stesso oppure un'altro thread dalla lista. */
            gldllist_Node_t *nextNodeTh;

            /* verifico che il thread che st� per eseguire non sia stato rimosso */
            if (gldllist_IsContained (nodeTh, &ThreadList))
            {
                gldactivity_Thread_t *thread;

                thread = gldllist_Get (nodeTh);
                if (thread != NULL && thread->Status == GLDACTIVITY_THREAD_RUNNING)
                { /* se ho raggiunto il timestamp eseguo la funzione */
                    if (swtimer_IsAfterEq (swtimer_GetMsActivity ( ), thread->RunDeadline))
                    { /* esecuzione one shot */
                        thread->Status = GLDACTIVITY_THREAD_STOPPED;
                        if (thread->ThreadFunc)
                            thread->ThreadFunc (thread);
                        if (ThreadListModified)
                            break;
                    }
                }
            } else
                break;
            nextNodeTh = gldllist_GetNext (nodeTh);
            nodeTh = nextNodeTh;
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Crea un nuovo thread associato ad un'oggetto.
        Verr� automaticamente distrutto assieme all'oggetto. Ogni oggetto ha un
        contatore con il numero di thread associati. Puoi associare ad ogni
        oggetto un massimo di GLDOBJ_NUM_THEARD_MAX thread.
        Se passi NULL il thread non viene pi� distrutto automaticamente ma devi
        occupartene chiamando la funzione gldactivity_ThreadDestroy.
 Arg: - <obj> Puntatore all'oggetto di riferimento.
        <func> Funzione del thread.
 Ret: - Puntatore al nuovo thread.
______________________________________________________________________________*/
gldactivity_Thread_t *gldactivity_ThreadCreate (gldobj_t *obj, void (*func) (struct gldactivity_Thread_s *))
{
    gldactivity_Thread_t *newTh = NULL;
    GLDCFG_ASSERT (func);

    if (obj == NULL || obj->NumLinkedThread < GLDOBJ_NUM_THEARD_MAX)
    {
        newTh = GLDCFG_CALLOC (1, sizeof (gldactivity_Thread_t));
        if (newTh)
        {
            gldllist_NodeInit (&newTh->ListNode);
            gldllist_Set (&newTh->ListNode, newTh);
            newTh->LinkedObj = obj;
            if (obj)
                obj->NumLinkedThread++;
            newTh->ThreadFunc = func;
            newTh->RunDeadline = 0;
            newTh->Status = GLDACTIVITY_THREAD_STOPPED;
            ThreadListModified = true;
            gldllist_AddLast (&ThreadList, &newTh->ListNode);
        }
    }
    return newTh;
}

/*______________________________________________________________________________ 
 Desc:  Distruggi un thread.
 Arg: - <thread> Rif. al thread da distruggere.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_ThreadDestroy (gldactivity_Thread_t *thread)
{
    GLDCFG_ASSERT (thread);
    if (gldllist_IsContained (&thread->ListNode, &ThreadList))
    {
        ThreadListModified = true;
        gldllist_Remove (&thread->ListNode);
        if (thread->LinkedObj)
            thread->LinkedObj->NumLinkedThread--;
        GLDCFG_FREE (thread);
    }
}

/*______________________________________________________________________________ 
 Desc: Distruggi tutti i thread associati ad uno specifico oggetto.
       Passando NULL distruggi tutti i thread che non hanno oggetto associato,
       cio� creati passando NULL come oggetto di riferimento.
 Arg: - <obj> Rif. all'oggetto.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_ObjThreadsDestroy (gldobj_t *obj)
{
    gldllist_Node_t *nodeTh;
    GLDCFG_ASSERT (obj);
    
    nodeTh = gldllist_GetFirst (&ThreadList);
    while (nodeTh
           /* fintanto che rimangono thread associati all'oggetto */
           && (obj == NULL || obj->NumLinkedThread))
    {
        /* salvo il prossimo prima di distruggere il nodo */
        gldllist_Node_t *nextNodeTh;
        gldactivity_Thread_t *thread;

        nextNodeTh = gldllist_GetNext (nodeTh);
        thread = gldllist_Get (nodeTh);
        if (thread->LinkedObj == obj)
            gldactivity_ThreadDestroy (thread);
        nodeTh = nextNodeTh;
    }
}

/*______________________________________________________________________________ 
 Desc:  Setta l'intervallo in ms della prossima esecuzione.
 Arg: - <thread> Rif. al thread.
        <timeout> Timeout in ms. (es. 1000 per 1 secondo)
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_ThreadStart (gldactivity_Thread_t *thread, swtimer_Time_t timeout)
{
    GLDCFG_ASSERT (thread);
    /* inizializzo la deadline e metto in run il thread */
    thread->RunDeadline = swtimer_GetMsActivity ( ) + timeout;
    thread->Status = GLDACTIVITY_THREAD_RUNNING;
}

/*______________________________________________________________________________ 
 Desc:  Interrompi l'esecuzione di un thread.
 Arg: - <thread> Rif. al thread.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_ThreadStop (gldactivity_Thread_t *thread)
{
    GLDCFG_ASSERT (thread);
    /* metto in stop il thread */
    thread->Status = GLDACTIVITY_THREAD_STOPPED;
}

/*______________________________________________________________________________ 
 Desc:  Il modulo gldObj chiama questa funzione ogni volta che un'oggetto
        viene distrutto.
 Arg: - <obj>[in] Puntatore all'oggetto che st� per essere distrutto.
 Ret: - None.
______________________________________________________________________________*/
void gldactivity_NotifyDestroy (gldobj_t *obj)
{   /* quando un'oggetto viene distrutto, distruggo anche tutti i thread associati */
    gldactivity_ObjThreadsDestroy (obj);
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Inoltra la notifica di un evento ad una particolare activity.
 Arg: - <activity> activity a cui inoltrare la notifica.
        <evt> l'evento da inoltrare.
        <ext> eventuali dati aggiuntivi associati all'evento.
 Ret: - None.
______________________________________________________________________________*/
static inline int32_t ActivityNotify (gldactivity_t *activity, gldactivity_Event_e evt, void *ext)
{
    int32_t ret = 0;
    
    if (activity->Attr->FuncEvent != NULL)
        ret = activity->Attr->FuncEvent (activity, evt, ext);
    return ret;
}

/*______________________________________________________________________________
 Desc:  Aggiunge un blocco di memoria allo stack.
    La memoria specificata viene copiata nello stack.
 Arg: - <ptr> indirizzo di memoria.
        <size> dimensione in byte della memoria.
 Ret: - None.
______________________________________________________________________________*/
static void StackPush (void *ptr, size_t size)
{
    GLDCFG_ASSERT ((StackPtr + size) <= sizeof (Stack));
    if (ptr)
        memcpy (&Stack[StackPtr], ptr, size);
    StackPtr += size;
}

/*______________________________________________________________________________
 Desc:  Rimuove un blocco di memoria dallo stack.
    La memoria specificata viene copiata dallo stack nell'indirizzo specificato.
 Arg: - <ptr> indirizzo di memoria da scrivere.
        <size> dimensione in byte della memoria.
 Ret: - None.
______________________________________________________________________________*/
static void StackPop (void *ptr, size_t size)
{
    GLDCFG_ASSERT ((int32_t)(StackPtr - size) >= 0);
    if (ptr)
        memcpy (ptr, &Stack[StackPtr - size], size);
    StackPtr -= size;
}
