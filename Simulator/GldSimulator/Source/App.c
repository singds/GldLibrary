//===================================================================== INCLUDES
#include "App.h"

#include "BkeHalDisp.h"
#include "BkeHalTouch.h"
#include "BkeHalTimer.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void app_InitModule (void)
{
	gldrefr_Init ( );

	bkehaldisp_Init ( );
	glddisp_Init (&bkehaldisp_Display);
	gldtouch_Init (&bkehaltouch_Touch);
	gldtimer_Init (NULL);

	//gldactivity_Next (&actassets_Page, false);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void app_Task (void)
{
	gldrefr_Task ( );
	gldtouch_Task ( );
}

//============================================================ PRIVATE FUNCTIONS

