#ifndef GLDDRAWSTD_H_INCLUDED
#define GLDDRAWSTD_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include <stdlib.h>
#include "GldCore/GldDraw.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void glddrawstd_Bmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, glddraw_BmpData_t *d);
void glddrawstd_BmpFs (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, glddraw_BmpFsData_t *d);
void glddrawstd_PxlMap (gldhaldisp_t *disp, gldrect_t *mapRect, gldcolor_PxlFormat_t format);

#endif /* GLDDRAWSTD_H_INCLUDED */
