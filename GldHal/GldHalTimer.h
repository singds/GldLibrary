#ifndef GLDHALTIMER_H_INCLUDED
#define GLDHALTIMER_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"
#include "GldCfg.h"

//====================================================================== DEFINES
typedef struct
{
    gldtime_t (*GetMillis) (void);
} gldhaltimer_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GLDHALTIMER_H_INCLUDED */

