#ifndef GLDTXT_H_INCLUDED
#define GLDTXT_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldCfg.h"

//====================================================================== DEFINES
typedef uint32_t gldtxt_Letter_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
char *gldtxt_GetLetter (gldtxt_Letter_t *ltr, const char *txt);
char *gldtxt_GetLine (const char *txt, int16_t *size);
int16_t gldtxt_GetLineCnt (const char *txt);

#endif /* GLDTXT_H_INCLUDED */

