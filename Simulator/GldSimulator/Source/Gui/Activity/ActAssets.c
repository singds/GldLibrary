//===================================================================== INCLUDES
#include "ActAssets.h"

#include "ActTest1.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static int32_t ActivityEvent (gldactivity_t *activity, gldactivity_Event_e evt, void *ext);
static int32_t ActivityTask (gldactivity_t *activity);
static gldobj_t *ActivityBuild (gldactivity_t *activity);
static void DrawActivityBackground (glddraw_Dev_t *disp, gldobj_t *obj);

//============================================================= STATIC VARIABLES
gldobj_t ObjBackground;

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
static void ObjEvtPagina2 (gldbutton_t *pButton, gldbutton_Event_e evt, void *ext)
{
	if (evt == GLDBUTTON_EVENT_CLICK)
		gldactivity_Next (&acttest1_Page, false);
}

//__________________________________________________________ ACTIVITY DEFINITION
//__________________________________________________________ ACTIVITY DEFINITION

/*______________________________________________________________________________ 
 Desc:  Questa funzione deve rappresentare lo sfondo della pagina.
 Arg: - <obj> puntatore all'oggetto sfondo, di cui questa funzione ne porta a
              termine la rappresentazione.
 Ret: - 0 se nessun errore.
______________________________________________________________________________*/
static void DrawActivityBackground (glddraw_Dev_t *disp, gldobj_t *obj)
{
	/* --->>> Rappresenta qui le componenti fisse della pagina, (es. sfondo, titolo) */
	glddraw_FillArea (disp, obj->Area.X1, obj->Area.Y1, obj->Area.X2, obj->Area.Y2, GLDCOLOR_BLACK);
	glddraw_FsBmp (disp, obj->Area.X1, obj->Area.Y1, GLDCOORD_ALIGN_LT, ".\\Source\\Gui\\Image\\Bmp\\LavanderiaBg.bmp", NULL);
	/* <<<--- Rappresenta qui le componenti fisse della pagina, (es. sfondo, titolo) */
}

/*______________________________________________________________________________ 
 Desc:  Chiamata dal motore grafico per generare gli oggetti della pagina.
 Arg: - <activity> puntatore all'activity di cui generare la gerarchia di oggetti.
 Ret: - Devi restituire l'oggetto radice della pagina.
______________________________________________________________________________*/
static gldobj_t *ActivityBuild (gldactivity_t *activity)
{
	gldobj_Init (&ObjBackground);
	GLDOBJ_DEF (&ObjBackground, NULL)
	{
		gldobj_SetAbsPos (thisObj, 0, 0);
		gldobj_SetSize (thisObj, 800, 480);
		thisObj->FuncDraw = DrawActivityBackground;

		GLDBUTTON_DEF (gldbutton_Create ( ), thisObj)
		{
			gldobj_SetRelPos (thisParent, thisObj, 0, 40);
			gldobj_SetSize (thisObj, 200, 50);
			thisButton->BodyColor = GLDCOLOR_WHITE;
			thisButton->BorderColor = GLDCOLOR_BLACK;
			thisButton->BorderSize = 1;
			thisButton->BodyRadius = 0;
			thisButton->FuncEventSpec = ObjEvtPagina2;

			GLDLABEL_DEF (gldlabel_Create (0), thisObj)
			{
				gldobj_SetAbsArea (thisObj, &thisParent->Area);
				thisLabel->Align = GLDCOORD_ALIGN_LM;
				thisLabel->Font = guifont_Arial14;
				thisLabel->String = "VAI A PAGINA 2";
				thisLabel->Pos.X = 10;
				thisLabel->Pos.Y = 25;
			}
		}
	}
	
	return &ObjBackground;
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
