#ifndef GLDHALTOUCH_H_INCLUDED
#define GLDHALTOUCH_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

//====================================================================== DEFINES
typedef struct
{
/*______________________________________________________________________________
 Desc:  Ottieni stato e coordinate del tocco.
 Arg: - <x>[out] in condizione di tocco, questa variabile viene scritta con le
            coordinate del tocco.
        <y>[out] in condizione di tocco, questa variabile viene scritta con le
            coordinate del tocco.
 Ret: - Restituisce true quando il touch è toccato.
______________________________________________________________________________*/
    bool (*GetTouch) (int16_t *x, int16_t *y);
} gldhaltouch_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GLDHALTOUCH_H_INCLUDED */

