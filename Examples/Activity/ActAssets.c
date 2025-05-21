//===================================================================== INCLUDES
#include "ActAssets.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static int32_t ActivityEvent (gldactivity_t *activity, gldactivity_Event_e evt, void *ext);
static int32_t ActivityTask (gldactivity_t *activity);
static gldobj_t *ActivityBuild (gldactivity_t *activity);
static void DrawActivityBackground (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);

//============================================================= STATIC VARIABLES
static uint16_t NumberVariable = 0;

//============================================================= GLOBAL VARIABLES
/* Questa struttura rappresenta la definizione di una pagina e contiene i suoi
   parametri caratteristici */
const gldactivity_Attr_t actassets_Page =
{
    .FuncEvent = ActivityEvent,
    .FuncTask = ActivityTask,
    .FuncBuild = ActivityBuild,
};

//============================================================= GLOBAL FUNCTIONS

//============================================================ PRIVATE FUNCTIONS

static void ObjEvtButton (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    if (evt == GLDOBJ_EVENT_PRESS)
    {   /* alla pressione del pulsante ne cambio il colore del corpo */
        objbutton_SetBodyClr (obj, GLDCOLOR_GREEN);
    }
    else if (evt == GLDOBJ_EVENT_RELEASE)
    {   /* alla pressione del pulsante ripristino il colore bianco */
        objbutton_SetBodyClr (obj, GLDCOLOR_WHITE);
    }
    else if (evt == GLDOBJ_EVENT_CLICK)
    {   /* esegui una funzione alla pressione del pulsante */
        NumberVariable ++;
    }
}

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
        glddraw_FillArea (disp, obj->Area.X1, obj->Area.Y1, obj->Area.X2, obj->Area.Y2, 0xAAAAAA);
        glddraw_Printf (disp, 10, 10, GLDCOORD_ALIGN_LT, guifont_ArialUniSz14, GLDCOLOR_RED, GLDCOLOR_NONE, "TITOLO PAGINA");
    }
    /* <<<--- Rappresenta qui le componenti fisse della pagina, (es. sfondo, titolo) */
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
        
        //________________________________________________________ LABEL EXAMPLE
        //________________________________________________________ LABEL EXAMPLE
        //________________________________________________________ LABEL EXAMPLE
        OBJLABEL_DEF (objlabel_Create (0), thisObj)
        {
            objlabel_SetPos (thisObj, 10, 30);
            thisLabel->Align = GLDCOORD_ALIGN_LT;
            thisLabel->AutoNewline = true;
            thisLabel->Font = guifont_ArialUniSz14;
            thisLabel->Color = GLDCOLOR_BLACK;
            thisLabel->ColorBg = GLDCOLOR_NONE;
            
            /* stampo sulla label il testo che voglio visualizzare. Posso stampare
               in qualsiasi momento un nuovo testo. La visualizzazione viene
               aggiornata solamente quando il testo cambia. */
            objlabel_SetStaticTxt (thisObj, guitxt_CIAO_MONDO[GUI_LINGUA]);
        }
        
        //_______________________________________________________ BUTTON EXAMPLE
        //_______________________________________________________ BUTTON EXAMPLE
        //_______________________________________________________ BUTTON EXAMPLE
        OBJBUTTON_DEF (objbutton_Create (0), thisObj)
        {
            gldobj_SetSize (thisObj, 100, 50);
            gldobj_SetRelPos (thisObj, thisParent, 10, 50);
            thisButton->BodyRadius = 10;
            thisButton->BorderSize = 1;
            thisButton->BodyColor = GLDCOLOR_WHITE;
            thisButton->BorderColor = GLDCOLOR_BLACK;
            thisButton->FuncEventSpec = ObjEvtButton;
        }
        
        //________________________________________________________ IMAGE EXAMPLE
        //________________________________________________________ IMAGE EXAMPLE
        //________________________________________________________ IMAGE EXAMPLE
        OBJIMAGE_DEF (objimage_Create (0), thisObj)
        {
            objimage_SetImage (thisObj, OBJIMAGE_LOCATION_MEM, guiimg_Linux);
            gldobj_HAlignRR (thisObj, thisParent, 0);
            gldobj_VAlignBB (thisObj, thisParent, 0);
        }
        
        //_______________________________________________________ NUMBER EXAMPLE
        //_______________________________________________________ NUMBER EXAMPLE
        //_______________________________________________________ NUMBER EXAMPLE
        OBJNUMBER_DEF (objnumber_Create (0), thisObj)
        {
            thisNumber->Max = 100;
            thisNumber->Min = 0;
            thisNumber->Decimali = 0;
            thisNumber->NumType = OBJNUMBER_TYPE_U16;
            thisNumber->NumPnt = &NumberVariable;
            
            OBJLABEL_DEF (thisObj, NULL)
            {
                objlabel_SetPos (thisObj, GUI_X_RES - 10, 40);
                thisLabel->Align = GLDCOORD_ALIGN_RT;
                thisLabel->Font = guifont_ArialUniSz14;
            }
        }
        
        /* --->>> Crea qui i tuoi oggetti grafici */
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
        break;
        
        /* alla distruzione della pagina, quando viene definitivamente eliminata
           dallo stack delle pagine */
        case GLDACTIVITY_EVENT_DESTROY:
        break;
        
        /* al passaggio da questa ad un'altra pagina, con la possibilit� di ripristinarla
           es. durente una chiamata a 'gldactivity_Next' con trace = true. */
        case GLDACTIVITY_EVENT_SUSPEND:
        break;
        
        /* alla ripresa di una pagina precedentemente sospesa
           es. dopo la chiamata a 'gldacrivity_Prev' */
        case GLDACTIVITY_EVENT_RESUME:
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
