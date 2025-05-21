//===================================================================== INCLUDES
#include "ActTemplate.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static int32_t ActivityEvent (gldactivity_t *activity, gldactivity_Event_e evt, void *ext);
static int32_t ActivityTask (gldactivity_t *activity);
static gldobj_t *ActivityBuild (gldactivity_t *activity);
static void DrawActivityBackground (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
/* Questa struttura rappresenta la definizione di una pagina e contiene i suoi
   parametri caratteristici */
const gldactivity_Attr_t acttemplate_Page =
{
    .FuncEvent = ActivityEvent,
    .FuncTask = ActivityTask,
    .FuncBuild = ActivityBuild,
};

//============================================================= GLOBAL FUNCTIONS

//============================================================ PRIVATE FUNCTIONS

//__________________________________________________________ ACTIVITY DEFINITION
//__________________________________________________________ ACTIVITY DEFINITION

/*______________________________________________________________________________ 
 Desc:  Questa funzione deve rappresentare lo sfondo della pagina.
 Arg: - <obj> puntatore all'oggetto sfondo, di cui questa funzione ne porta a
              termine la rappresentazione.
 Ret: - 0 se nessun errore.
______________________________________________________________________________*/
static void DrawActivityBackground (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    /* --->>> Rappresenta qui le componenti fisse della pagina, (es. sfondo, titolo) */
    if (cmd == GLDOBJ_DRAW_MAIN) {
        glddraw_FillArea (disp, 0, 0, GUI_X_RES, GUI_Y_RES, GLDCOLOR_BLACK);
    }
}

/*______________________________________________________________________________ 
 Desc:  Chiamata dal motore grafico per generare gli oggetti della pagina.
 Arg: - <activity> puntatore all'activity di cui generare la gerarchia di oggetti.
 Ret: - Devi restituire l'oggetto radice della pagina.
______________________________________________________________________________*/
static gldobj_t *ActivityBuild (gldactivity_t *activity)
{
    gldobj_t *root;
    GLDOBJ_DEF ((root = gldcustom_Background_Create (DrawActivityBackground)), NULL)
    {   /* --->>> Crea qui i tuoi oggetti grafici */
        (void)thisObj; // avoid compiler warnings
    }
    
    return root;
}

/*______________________________________________________________________________ 
 Desc:  Callback chiamata dal motore grafico per notificare gli eventi dell'activity.
 Arg: - <activity> puntatore all'activity che ha scatenato l'evento.
        <evt> identificativo evento.
        <ext> eventuali dati specifici dell'evento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t ActivityEvent (gldactivity_t *activity, gldactivity_Event_e evt, void *ext)
{
    switch (evt)
    {
    /* alla creazione della pagina
    es. durante una chiamata 'gldactivity_Next' */
    case GLDACTIVITY_EVENT_CREATE:
    /* alla ripresa di una pagina precedentemente sospesa
    es. dopo la chiamata a 'gldacrivity_Prev' */
    case GLDACTIVITY_EVENT_RESUME:
        break;

    /* al passaggio da questa ad un'altra pagina, con la possibilità di ripristinarla
    es. durente una chiamata a 'gldactivity_Next' con trace = true. */
    case GLDACTIVITY_EVENT_SUSPEND:
    /* alla distruzione della pagina, quando viene definitivamente eliminata
    dallo stack delle pagine */
    case GLDACTIVITY_EVENT_DESTROY:
        break;
    }
    return 0;
}

/*______________________________________________________________________________ 
 Desc:  Chiamata in polling dal motore grafico quando � visibile questa pagina.
 Arg: - <activity> puntatore all'activity attualmente visibile.
 Ret: - None.
______________________________________________________________________________*/
static int32_t ActivityTask (gldactivity_t *activity)
{
    return 0;
}
