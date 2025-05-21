//===================================================================== INCLUDES
#include "GldCore/GldBoost.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
/* il driver boost che usa la libreria. */
gldhalboost_t *gldboost_Boost = NULL;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Inizializza il modbulo boost impostando il driver da utilizzare.
 Arg: - <boost> il driver boost da utilizzare.
 Ret: - None.
______________________________________________________________________________*/
void gldboost_Init (gldhalboost_t *boost)
{
    gldboost_Boost = boost;
}

//============================================================ PRIVATE FUNCTIONS
