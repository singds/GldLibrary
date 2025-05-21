#ifndef GLD_H_INCLUDED
#define GLD_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include <stdlib.h>

//====================================================================== DEFINES
typedef struct
{
    int32_t AllocMemCurr; /* quantit� corrente di memoria allocata */
    int32_t AllocMemMax;  /* massima quantit� di memoria allocata simultaneamente */
} gld_Statistics_t;

//============================================================= GLOBAL VARIABLES
extern gld_Statistics_t gld_Statistics;

//============================================================ GLOBAL PROTOTYPES
void *gld_Malloc (size_t size);
void *gld_Realloc (void *ptr, size_t size);
void *gld_Calloc (size_t nmemb, size_t size);
void gld_Free (void *pnt);

#endif /* GLD_H_INCLUDED */
