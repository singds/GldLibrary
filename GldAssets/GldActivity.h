#ifndef GLDACTIVITY_H_INCLUDED
#define GLDACTIVITY_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"
#include "SwTimer.h"

//====================================================================== DEFINES
struct gldactivity_s;
struct gldactivity_Attr_s;

typedef enum
{
    GLDACTIVITY_EVENT_CREATE = 0,
    GLDACTIVITY_EVENT_DESTROY,
    GLDACTIVITY_EVENT_SUSPEND,
    GLDACTIVITY_EVENT_RESUME,
} gldactivity_Event_e;

typedef enum
{
    GLDACTIVITY_THREAD_STOPPED,
    GLDACTIVITY_THREAD_RUNNING,
} gldactivity_ThreadState_e;

typedef struct gldactivity_Attr_s
{ /* attributi caratteristici activity */
    int32_t (*FuncEvent) (struct gldactivity_s *activity, gldactivity_Event_e evt, void *ext);
    int32_t (*FuncTask) (struct gldactivity_s *activity);
    gldobj_t *(*FuncBuild) (struct gldactivity_s *activity);
} gldactivity_Attr_t;

typedef struct gldactivity_Thread_s
{
    gldllist_Node_t ListNode;
    gldobj_t *LinkedObj; /* oggetto associato a questo thread */
    gldactivity_ThreadState_e Status;
    swtimer_Time_t RunDeadline; /* timestamp della prossima esecuzione di questo thread */
    void (*ThreadFunc) (struct gldactivity_Thread_s *);
} gldactivity_Thread_t;

typedef struct gldactivity_s
{
    gldobj_t *RootObj; /* Oggetto radice della pagina */

    const gldactivity_Attr_t *Attr; /* attributi activity */
    void *UsrParam;                 /* puntatore a parametri disponibili per l'utente */
} gldactivity_t;

//============================================================= GLOBAL VARIABLES
extern gldactivity_t *gldactivity_Curr;

//============================================================ GLOBAL PROTOTYPES
void gldactivity_Next (const gldactivity_Attr_t *attr, bool trace);
void gldactivity_Prev (void);

void gldactivity_StackPush (void *ptr, size_t size);
void gldactivity_StackPop (void *ptr, size_t size);
void gldactivity_StackFlush (void);

void gldactivity_Task (void);

/* I thread vengono distrutti quando viene distrutto l'oggetto associato */
gldactivity_Thread_t *gldactivity_ThreadCreate (gldobj_t *, void (*func) (struct gldactivity_Thread_s *));
void gldactivity_ThreadDestroy (gldactivity_Thread_t *thread);
void gldactivity_ObjThreadsDestroy (gldobj_t *obj);
void gldactivity_ThreadStart (gldactivity_Thread_t *thread, swtimer_Time_t timeout);
void gldactivity_ThreadStop (gldactivity_Thread_t *thread);

void gldactivity_NotifyDestroy (gldobj_t *obj);

#endif /* GLDACTIVITY_H_INCLUDED */
