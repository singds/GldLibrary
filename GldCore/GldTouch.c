//===================================================================== INCLUDES
#include "GldCore/GldTouch.h"

#include "GldCfg.h"
#include "GldAssets/ObjButton.h"
#include "GldCore/GldDraw.h"
#include "GldCore/GldRefr.h"
#include "GldMisc/GldCoord.h"
#include "GldCore/GldTimer.h"
#include <string.h>

//====================================================================== DEFINES
typedef struct
{
    bool TouchState;           /* current touch state (toccato-non toccato) */
    gldcoord_Point_t TouchPos; /* current touch position */
    objbutton_t *ActiveBtn;    /* current active button */

    bool InhibitClick;       /* 1: inibisce l'evento click sull'oggetto attivo */
    bool InhibitLongClick;   /* inibisce l'evento long click sull'oggetto attivo */
    bool InhibitRepeatClick; /* inibisce l'evento repeat click sull'oggetto attivo */

    gldtime_t LongClickDeadline; /* deadline evento long click (raggiunto questo timestamp si scatena l'evento) */

    objbutton_t *DragBtn; /* oggetto trascinato */
    /* ultima posizione di tocco (utilizzata per calcolare lo spostamente del
       nuovo tocco rispetto al precedente nella gestione del drag) */
    gldcoord_Point_t LastTouchPos;
    bool DragDetected; /* a 1 quando e' stata identificato l'inizio di un'azione di trascinamento */
    bool Dragging; /* a 1 quando e' in corso un'azione di trascinamento */

    /* oggetto con trascinamento orizzontale/verticale abilitato, che si trovava
    pi� in foreground durente il primo tocco */
    objbutton_t *DragVBtn;
    objbutton_t *DragHBtn;
    
    enum {
        L_REPEAT_STATUS_START,
        L_REPEAT_STATUS_SLOW,
        L_REPEAT_STATUS_FAST,
    } RepeatStatus;
    uint16_t RepeatCnt;
    gldtime_t RepeatDeadline; /* deadline primo evento repeat click */
} gldtouch_Ctx_t;

#define L_REPEAT_WAIT_INTERVAL          300
#define L_REPEAT_SLOW_INTERVAL          100
#define L_REPEAT_SLOW_NUM               4
#define L_REPEAT_FAST_INTERVAL          30

//=========================================================== PRIVATE PROTOTYPES
bool GetTouch (gldhaltouch_t *drv, gldcoord_Point_t *pos);
static void ManagePress (gldtouch_Ctx_t *ctx);
static void ManageRelease (gldtouch_Ctx_t *ctx);

static objbutton_t *GetTopMost (gldobj_t *obj, const gldcoord_Point_t *pos, bool (*func) (gldobj_t *));
static objbutton_t *GetTopMostI (gldobj_t *obj, const gldcoord_Point_t *pos, bool (*func) (gldobj_t *));

static void ObjDrawTocco (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);
static void StartDrag (gldtouch_Ctx_t *ctx, objbutton_t *button);

//============================================================= STATIC VARIABLES
static gldtouch_Ctx_t Ctx; /* contesto del modulo */

//============================================================= GLOBAL VARIABLES
/* il driver del touchscreen che usa la libreria */
gldhaltouch_t *gldtouch_Touch;
gldobj_t *gldtouch_ObjTocco; /* oggetto che rappresenta graficamente la posizione del tocco */

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Inizializza il modulo touch impostando il driver touch che usa la libreria.
 Arg: - <touch> il driver touch che usa la libreria.
 Ret: - None.
______________________________________________________________________________*/
void gldtouch_Init (gldhaltouch_t *touch)
{
    gldtouch_Touch = touch;
    /* creo l'oggetto che rappresenter� graficamente la posizione del tocco */
    GLDOBJ_DEF (gldtouch_ObjTocco = gldobj_Create (0), gldrefr_SystemLayer)
    {
        thisObj->FuncDraw = ObjDrawTocco;
        thisObj->Hide = true;
        thisObj->Area.X1 = 0;
        thisObj->Area.Y1 = 0;
        gldobj_SetSize (thisObj, GLDCFG_MOSTRA_TOCCO_SIZE, GLDCFG_MOSTRA_TOCCO_SIZE);
    }
}

/*______________________________________________________________________________ 
 Desc:  Polling che gestisce l'acqusizione dei tocchi sul touch.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldtouch_Task (void)
{
    /* verifico se l'oggetto attualmente attivo � ancora valido: appartenente
       ancora alla radice degli oggetti visualizzati. */
    if (Ctx.ActiveBtn != NULL && !gldobj_IsInside (&Ctx.ActiveBtn->Obj, gldrefr_RootObj))
        Ctx.ActiveBtn = NULL; /* annullo l'oggetto attivo */

    if (GetTouch (gldtouch_Touch, &Ctx.TouchPos))
    { /* gestione pressione touch */
        ManagePress (&Ctx);
    } else
    { /* gestione rilascio touch */
        ManageRelease (&Ctx);
    }
}

/*______________________________________________________________________________ 
 Desc:  Ottieni le coordinate del punto del tocco attuale.
 Arg: - <pos>[out] la funzione scriver questa variabile con le coordinate del
    punto di tocco.
 Ret: - true quando c'è un tocco.
______________________________________________________________________________*/
bool gldtouch_GetCoords (gldcoord_Point_t *pos)
{
    memcpy (pos, &Ctx.TouchPos, sizeof (gldcoord_Point_t));
    return Ctx.TouchState;
}

/*______________________________________________________________________________ 
 Desc:  Il modulo gldObj chiama questa funzione ogni volta che un'oggetto
        viene distrutto.
 Arg: - <obj>[in] Puntatore all'oggetto che st� per essere distrutto.
 Ret: - None.
______________________________________________________________________________*/
void gldtouch_NotifyDestroy (gldobj_t *obj)
{
    if (&Ctx.ActiveBtn->Obj == obj)
    { // resetto il riferimento all'oggetto attivo se viene distrutto
        Ctx.ActiveBtn = NULL;
    }
    if (&Ctx.DragBtn->Obj == obj)
    { // resetto riferimento e stato dell'oggetto trascinato se distrutto
        Ctx.DragBtn = NULL;
        Ctx.Dragging = false;
    }
}

/*______________________________________________________________________________ 
 Desc:  L'utilizzatore della libreria chiama questa funzione per marcare l'inizio
    di una gesture drag.
 Arg: - <obj> l'oggetto su cui iniziare il drag.
 Ret: - None.
______________________________________________________________________________*/
void gldtouch_StartDrag (gldobj_t *obj)
{
    GLDASSETS_GET_TYPED (objbutton_t, button, "ObjButton", obj);
    /* soltanto se non ci sono oggetti in trascinamento */
    GLDCFG_ASSERT (!Ctx.Dragging);
    
    if (!Ctx.Dragging) {
        StartDrag (&Ctx, button);
    }
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Ottini le informazioni filtrate su tocco e posizione tocco.
 Arg: - <drv> driver touch.
      - <pos> variabile su cui rendere disponibile le coordinate del tocco.
        questa variabile viene scritta dalla funzione.
 Ret: 1: tocco alla posizione 'pos'.
      0: nessun tocco, il contenuto di 'pos' non viene scritto.
______________________________________________________________________________*/
bool GetTouch (gldhaltouch_t *drv, gldcoord_Point_t *pos)
{
    static gldtime_t timer;
    static bool state = false;
    static int16_t last_x, last_y;

    if (drv != NULL)
    {
        int16_t x, y;
        bool newState;
        gldtime_t currentTick;

        currentTick = gldtimer_GetMillis ( );
        /* ottengo tocco e coordinate dall'hardware,
           filtro lo stato del tocco con un ritardo di 'GLDCFG_TOUCH_DEBOUNCE_TIME' ms */
        if ((newState = drv->GetTouch (&x, &y)) == true)
        { /* salvo le coordinate di questa pressione */
            last_x = x;
            last_y = y;
        }

        if (newState != state)
        {
            if (gldtimer_IsAfterEq (currentTick, timer))
            { /* "mantengo il timer" e scrivo le coordinate del tocco */
                timer = currentTick;
                state = newState;
            }
        }

        if (newState == state)
        { /* ripristino il timer di debounce */
            timer = currentTick + GLDCFG_TOUCH_DEBOUNCE_TIME;
        }

        if (state == true)
        { /* se sono in stato premuto restituisco le coordinate dell'ultima
             pressione rilevata */
            pos->X = last_x;
            pos->Y = last_y;
        }
    }

    return state;
}

/*______________________________________________________________________________ 
 Desc:  Polling chiamato quando il touch è premuto.
 Arg: - <ctx> il contesto del touch.
 Ret: - None.
______________________________________________________________________________*/
static void ManagePress (gldtouch_Ctx_t *ctx)
{
    { /* sposto l'oggetto che rappresenta il tocco nella nuova posizione */
        gldcoord_t x0_pos, y0_pos;

        x0_pos = ctx->TouchPos.X - (gldcoord_AreaWidth (&gldtouch_ObjTocco->Area) >> 1);
        y0_pos = ctx->TouchPos.Y - (gldcoord_AreaHeight (&gldtouch_ObjTocco->Area) >> 1);
        gldobj_SetAbsPos (gldtouch_ObjTocco, x0_pos, y0_pos);
#if (GLDCFG_MOSTRA_TOCCO_ENABLED == 1)
        gldobj_SetVisible (gldtouch_ObjTocco, true);
#endif
    }

    if (ctx->TouchState == false)
    { /* primo contatto con il touch (appena toccato) */
        /* ricerco l'oggetto toccabile pi� foreground che contiene il punto di
           tocco */
        ctx->ActiveBtn = GetTopMost (gldrefr_RootObj, &ctx->TouchPos, objbutton_GetClick);
        /* ricerco l'oggetto trascinabile pi� foreground che contiene il punto
           di tocco */
        ctx->DragVBtn = GetTopMost (gldrefr_RootObj, &ctx->TouchPos, objbutton_GetDragV);
        ctx->DragHBtn = GetTopMost (gldrefr_RootObj, &ctx->TouchPos, objbutton_GetDragH);

        /* init state variables */
        ctx->LastTouchPos = ctx->TouchPos; /* salvo la prima posizione di tocco */
        if (ctx->ActiveBtn) {
            ctx->LongClickDeadline = gldtimer_GetMillis ( ) + ctx->ActiveBtn->LongClickTime * 100;
            ctx->RepeatStatus = L_REPEAT_STATUS_START;
            ctx->RepeatDeadline = gldtimer_GetMillis ( ) + L_REPEAT_WAIT_INTERVAL;
        }

        if (ctx->ActiveBtn != NULL)
        {
            ctx->ActiveBtn->StatePress = true;
            ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_PRESS, NULL); /* press notify */
        }
    } else
    { /* dopo il primo contatto con il touch (quando mantieni premuto). */

        /* ______________________________________ GESTIONE OGGETTO CLICCABILE */
        if (ctx->ActiveBtn != NULL)
        {
            gldcoord_Area_t touchArea;
            
            gldcoord_AreaCopy (&touchArea, &ctx->ActiveBtn->Obj.Area);
            touchArea.X1 -= GLDCFG_TOUCH_AREA_EXPAND;
            touchArea.Y1 -= GLDCFG_TOUCH_AREA_EXPAND;
            touchArea.X2 += GLDCFG_TOUCH_AREA_EXPAND;
            touchArea.Y2 += GLDCFG_TOUCH_AREA_EXPAND;
            
            /* verifico se il punto di tocco � ancora interno all'oggetto attivo */
            if (gldcoord_PointIsIn (ctx->TouchPos.X, ctx->TouchPos.Y, &touchArea) != true)
            {   /* tocco non pi� interno */
                ctx->ActiveBtn->StatePress = false;
                ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_RELEASE, NULL); /* release notify */
                ctx->ActiveBtn = NULL;
            } else
            {   /* tocco ancora interno all'oggetto attivo */
                if (ctx->InhibitLongClick == false)
                {
                    if (ctx->ActiveBtn->LongClick == true &&
                        gldtimer_IsAfterEq (gldtimer_GetMillis ( ), ctx->LongClickDeadline))
                    {
                        /* il long click si scatena una sola volta e quando rimuovi
                        il dito dal display dopo che si � scatenato non viene
                        notificato l'evento click. */
                        ctx->InhibitLongClick = true;
                        ctx->InhibitClick = true;
                        ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_LONG_CLICK, NULL);
                    }
                }
                
                if (ctx->InhibitRepeatClick == false)
                {
                    if (ctx->ActiveBtn &&
                        ctx->ActiveBtn->RepeatClick == true)
                    {
                        if (gldtimer_IsAfterEq (gldtimer_GetMillis ( ), ctx->RepeatDeadline))
                        {
                            gldtime_t interval;
                            
                            if (ctx->RepeatStatus == L_REPEAT_STATUS_START) {
                                interval = L_REPEAT_SLOW_INTERVAL;
                                ctx->InhibitClick = true;
                                ctx->RepeatCnt = L_REPEAT_SLOW_NUM;
                                ctx->RepeatStatus = L_REPEAT_STATUS_SLOW;
                                
                            } else if (ctx->RepeatStatus == L_REPEAT_STATUS_SLOW) {
                                if (ctx->RepeatCnt--)
                                    interval = L_REPEAT_SLOW_INTERVAL;
                                else {
                                    interval = L_REPEAT_FAST_INTERVAL;
                                    ctx->RepeatStatus = L_REPEAT_STATUS_FAST;
                                }
                                
                            } else {
                                interval = L_REPEAT_FAST_INTERVAL;
                            }
                            
                            ctx->RepeatDeadline = gldtimer_GetMillis ( ) + interval;
                            ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_REPEAT_CLICK, NULL);
                        }
                    }
                }
            }
        }

        /* ____________________________________ GESTIONE OGGETTO TRASCINABILE */
        {
            gldcoord_Point_t move;

            /* calcolo la differenza fra la nuova posizione di tocco e quella
               del giro precedente */
            move.X = ctx->TouchPos.X - ctx->LastTouchPos.X;
            move.Y = ctx->TouchPos.Y - ctx->LastTouchPos.Y;
            
            if (!ctx->Dragging)
            {   /* se nessun trascinamento in corso */
                if (!ctx->DragDetected)
                {   /* ancora non ho identificato l'inizio del trascinamento */
                    if (_MAX_ (abs (move.X), abs (move.Y)) > GLDCFG_TOUCH_DRAG_PXL_TRIGGER)
                    {
                        ctx->DragDetected = true;
                        /* hai appena cominciato a trascinare il dito sul display */
                        
                        /* decido se il trascinamento � verticale oppure orizzontale
                        sulla base del primo spostamento. se lo spostamento �
                        maggiore lungo l'asse x, considero un trascinamento orizzontale
                        , altrimenti verticale */
                        objbutton_t *dragTarget;
                        
                        if (abs (move.X) >= abs (move.Y))
                            dragTarget = ctx->DragHBtn;
                        else
                            dragTarget = ctx->DragVBtn;
                        
                        StartDrag (ctx, dragTarget);
                    }
                }
            }
            else
            {   /* stai trascinando il dito sul display */
                
                /* se lo scostamento rispetto all'ultimo tocco supera la
                   soglia minima, notifico l'ogetto del movimento */
                if (_MAX_ (abs (move.X), abs (move.Y)) > GLDCFG_TOUCH_DRAG_PXL_MIN_MOVE)
                {
                    ctx->LastTouchPos = ctx->TouchPos;
                    ctx->DragBtn->Obj.FuncEvent (&ctx->DragBtn->Obj, GLDOBJ_EVENT_DARG_MOVE, &move);
                }
            }
        }
    }

    ctx->TouchState = true;
}

/*______________________________________________________________________________ 
 Desc:  Polling chiamato quando il touch è rilasciato.
 Arg: - <ctx> il contesto del touch.
 Ret: - None.
______________________________________________________________________________*/
static void ManageRelease (gldtouch_Ctx_t *ctx)
{
    /* nascondo l'oggetto che rappresenta la posizione del tocco */
    gldobj_SetVisible (gldtouch_ObjTocco, false);

    if (ctx->TouchState == true)
    {   /* hai appena sollevato il dito dal display */
        if (ctx->ActiveBtn != NULL)
        {
            ctx->ActiveBtn->StatePress = false;
            ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_RELEASE, NULL); /* release notify */

            if (ctx->InhibitClick == false)
                ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_CLICK, NULL); /* click notify */
        }

        if (ctx->Dragging)
            ctx->DragBtn->Obj.FuncEvent (&ctx->DragBtn->Obj, GLDOBJ_EVENT_DRAG_END, &ctx->LastTouchPos);
    }

    /* reset delle variabili di stato */
    ctx->DragHBtn = NULL;
    ctx->DragVBtn = NULL;
    ctx->DragBtn = NULL;
    ctx->ActiveBtn = NULL;
    ctx->TouchState = false;
    ctx->DragDetected = false;
    ctx->Dragging = false;
    ctx->InhibitClick = false;
    ctx->InhibitLongClick = false;
    ctx->InhibitRepeatClick = false;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'oggetto pi� in primo piano che contiene il punto 'pos' e
    risulta positivo attraverso la funzione <func>. Wrapper attorno alla
    funzione <GetTopMostI>.
 Arg: - <obj>[in] oggetto radice da cui iniziare la ricerca.
        <pos>[in] coordinate del punto.
        <fun>[in] funzione di verifica.
 Ret: - Ritorna l'oggetto pi� foreground che ha dragging/click attivo, in base
    alla funzione <func> che fornisci.
______________________________________________________________________________*/
static objbutton_t *GetTopMost (gldobj_t *obj, const gldcoord_Point_t *pos, bool (*func) (gldobj_t *))
{
    objbutton_t *topMost;
    topMost = GetTopMostI (obj, pos, func);
    
    /* se topMost � -1, significa che:
        1) il punto pos appartiene ad un oggetto che impedisce agli oggetti in
        background rispetto a lui, di ricevere eventi di input (InputWall).
        2) l'oggetto al punto 1) risulta negativo alla funzione <func>.
        3) non esiste alcun oggetto pi� in foreground di quello al punto 1), che
        contenga il punto <pos> e risulti positiovo alla funzione <func>.
    */
    if (topMost == (void *)-1)
        topMost = NULL;
    return topMost;
}

/*______________________________________________________________________________ 
 Desc:  Vedi <GetTopMost>.
        Questa è la parte ricorsiva della funzione <GetTopMost>.
 Arg: - None.
 Ret: - NULL se nessun oggetto � stato trovato.
        -1 se il punto appartiene ad un oggetto in foreground che impedisce agli
        oggetti sottostanti di ricevere eventi di input.
______________________________________________________________________________*/
static objbutton_t *GetTopMostI (gldobj_t *obj, const gldcoord_Point_t *pos, bool (*func) (gldobj_t *))
{
    objbutton_t *topMost = NULL;

    if (gldcoord_PointIsIn (pos->X, pos->Y, &obj->Area) && obj->Hide == 0)
    { /* verifico se un figlio pi� foreground pi� gestire il trascinamento */
        gldllist_Node_t *node;
        
        node = gldllist_GetFirst (&obj->ChildList);
        while (node != NULL)
        {
            topMost = GetTopMostI (gldllist_Get (node), pos, func);
            if (topMost != NULL)
                break;
            node = gldllist_GetNext (node);
        }
        
        if (topMost == NULL)
        {   /* se nessuno dei figli � stato identificato tento con questo oggetto */
            objbutton_t *button;
            
            button = gldassets_CheckType (obj, "ObjButton");
            if (button != NULL) {
                if (func (&button->Obj))
                    topMost = button;
                else if (button->InputWall == true)
                    topMost = (void *)-1;
            }
        }
    }
    return topMost;
}

/*______________________________________________________________________________ 
 Desc:  Disegna il punto di tocco.
 Arg: - <disp> il display.
        <obj> l'oggetto che rappresenta il tocco.
        <cmd> la fase di disegno da portare a termine.
 Ret: - None.
______________________________________________________________________________*/
static void ObjDrawTocco (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    if (cmd == GLDOBJ_DRAW_MAIN) {
        gldcoord_t x_center, y_center, radius;

        radius = gldcoord_AreaWidth (&obj->Area) / 2;
        x_center = obj->Area.X1 + radius;
        y_center = obj->Area.Y1 + radius;

        glddraw_FillCircle (disp, x_center, y_center, radius, GLDCOLOR_RED);
        glddraw_Circle (disp, x_center, y_center, radius, GLDCOLOR_BLACK);
    }
}

/*______________________________________________________________________________ 
 Desc:  Marca l'inzio della fase di drag per un'oggetto.
    Quando su un'oggetto viene marcata l'inizio della fase di drag, quell'oggetto
    non scatenerà più altri eventi, come per sempio il long click oppure il click.
 Arg: - <ctx> il contesto del touch.
        <button> il pulsante su cui iniziare la fase di drag.
 Ret: - None.
______________________________________________________________________________*/
static void StartDrag (gldtouch_Ctx_t *ctx, objbutton_t *button)
{
    ctx->DragBtn = button;
    if (ctx->DragBtn != NULL)
    {   /* entro in modalit� drag */
        
        ctx->Dragging = true;
        if (ctx->ActiveBtn != NULL)
        {   /* se c'� un pulsante premuto lo rilascio.
            Solo un'azione fra drag e click pu� essere attiva in un
            certo istante */
            ctx->ActiveBtn->StatePress = false;
            ctx->ActiveBtn->Obj.FuncEvent (&ctx->ActiveBtn->Obj, GLDOBJ_EVENT_RELEASE, NULL);
            ctx->ActiveBtn = NULL;
        }
        
        ctx->LastTouchPos = ctx->TouchPos;
        /* inibisco gli eventi di click dell'eventuale oggetto
           attivo. Voglio che il trascinamento abbia la
           precedenza sul click. Se inizia un trascinamento gli
           altri eventi di tocco (come click o long click)
           vengono annullati */
        ctx->InhibitClick = true;
        ctx->InhibitLongClick = true;
        ctx->InhibitRepeatClick = true;

        /* Notifico l'inizio del trascinamento.
           Come informazione supplementare inoltro la posizione
           di inizio del trascinamento */
        ctx->DragBtn->Obj.FuncEvent (&ctx->DragBtn->Obj, GLDOBJ_EVENT_DRAG_START, &ctx->TouchPos);
    }
}
