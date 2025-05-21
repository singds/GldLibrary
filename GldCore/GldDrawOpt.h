#ifndef GLDDRAWOPT_H_INCLUDED
#define GLDDRAWOPT_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include <stdlib.h>
#include "GldCore/GldDraw.h"

//====================================================================== DEFINES

typedef struct
{
    void *BuffPtr;
    uint32_t X1;
    uint32_t Y1;
    uint32_t Xsize;
    uint32_t Ysize;
    uint8_t Bpp;
} glddrawopt_MemRect_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
int32_t glddrawopt_Bmp (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y, glddraw_BmpData_t *d);
int32_t glddrawopt_BmpFs (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y, glddraw_BmpFsData_t *d);
int32_t glddrawopt_PxlMap (glddrawopt_MemRect_t *mem, gldrect_t *mapRect, gldcolor_PxlFormat_t format);

#endif /* GLDDRAWOPT_H_INCLUDED */
