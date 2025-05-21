#ifndef GLDTIMER_H_INCLUDED
#define GLDTIMER_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldHal/GldHalTimer.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void gldtimer_Init(gldhaltimer_t *timer);

gldtime_t gldtimer_GetMillis(void);
bool gldtimer_IsAfter (gldtime_t tick, gldtime_t ref);
bool gldtimer_IsAfterEq (gldtime_t tick, gldtime_t ref);
bool gldtimer_IsBefore (gldtime_t tick, gldtime_t ref);
bool gldtimer_IsBeforeEq (gldtime_t tick, gldtime_t ref);

#endif /* GLDTIMER_H_INCLUDED */
