#ifndef GLDMISC_H_INCLUDED
#define GLDMISC_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include <stdlib.h>

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void *gldmisc_Get32Be (void *v32_p, const void *addr);
void *gldmisc_Get32Le (void *v32_p, const void *addr);
void *gldmisc_Get16Be (void *v16_p, const void *addr);
void *gldmisc_Get16Le (void *v16_p, const void *addr);
void *gldmisc_Get8 (void *v8_p, const void *addr);
void *gldmisc_GetString (void *char_p, const void *addr, size_t size);

#endif /* GLDMISC_H_INCLUDED */

