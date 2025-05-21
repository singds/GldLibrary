
//===================================================================== INCLUDES
#include "GldCore/GldTimer.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
/* il driver display che usa la libreria */
gldhaltimer_t *gldtimer_Timer = NULL;

//============================================================= GLOBAL FUNCTIONS

void gldtimer_Init(gldhaltimer_t *timer)
{
	gldtimer_Timer = timer;
}

gldtime_t gldtimer_GetMillis(void)
{ // TODO implement
	return 0;
}
/*______________________________________________________________________________ 
 Desc:  Verifica se il timestamp 'tick' � successivo a 'ref'.
 Arg: - <tick>: timestamp da comparare.
 Ret: - <ref>:  timestamp di riferimento.
______________________________________________________________________________*/
bool gldtimer_IsAfter (gldtime_t tick, gldtime_t ref)
{
    return ((int32_t) (ref) - (int32_t) (tick) < 0);
}

/*______________________________________________________________________________ 
 Desc:  Verifica se il timestamp 'tick' � successivo o uguale a 'ref'.
 Arg: - <tick>: timestamp da comparare.
 Ret: - <ref>:  timestamp di riferimento.
______________________________________________________________________________*/
bool gldtimer_IsAfterEq (gldtime_t tick, gldtime_t ref)
{
    return ((int32_t) (ref) - (int32_t) (tick) <= 0);
}

/*______________________________________________________________________________ 
 Desc:  Verifica se il timestamp 'tick' � precedente a 'ref'.
 Arg: - <tick>: timestamp da comparare.
 Ret: - <ref>:  timestamp di riferimento.
______________________________________________________________________________*/
bool gldtimer_IsBefore (gldtime_t tick, gldtime_t ref)
{
    return ((int32_t) (ref) - (int32_t) (tick) > 0);
}

/*______________________________________________________________________________ 
 Desc:  Verifica se il timestamp 'tick' � precedente o uguale a 'ref'.
 Arg: - <tick>: timestamp da comparare.
 Ret: - <ref>:  timestamp di riferimento.
______________________________________________________________________________*/
bool gldtimer_IsBeforeEq (gldtime_t tick, gldtime_t ref)
{
    return ((int32_t) (ref) - (int32_t) (tick) >= 0);
}

//============================================================ PRIVATE FUNCTIONS
