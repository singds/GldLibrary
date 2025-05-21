//===================================================================== INCLUDES
#include "App.h"

#include "Gui.h"
#include "Gui/BkeHalDisp.h"
#include "Gui/BkeHalTouch.h"
#include "Gui/Font/GuiFont.h"
#include "Gui/Activity/ActAssets.h"

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
	gldassets_InitModule ( );

	guifont_Arial14 = gldtxt_FontOpen (&fontarialsz14bpp1_Data);

	bkehaldisp_Init ( );
	glddisp_Disp = &bkehaldisp_Display;
	glddisp_Init ( );

	gldtouch_Touch = &bkehaltouch_Touch;
	gldtouch_Init ( );

	gldactivity_Next (&actassets_Page, false);
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
	gldassets_Task ( );
}

//============================================================ PRIVATE FUNCTIONS

