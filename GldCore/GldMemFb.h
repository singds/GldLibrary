#ifndef GLDMEMFB_H_INCLUDED
#define GLDMEMFB_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldHal/GldHalDisp.h"
#include "GldDrawOpt.h"
#include "GldCfg.h"

//====================================================================== DEFINES

//============================================================= GLOBAL VARIABLES
extern gldhaldisp_t gldmemfb_Disp;

//============================================================ GLOBAL PROTOTYPES

/*______________________________________________________________________________
 Desc:  Fast waiy to copy a pixel.
 Arg: - <srcPxl>[in] pointer to source pixel.
        <dstPxl>[in] pointer to destination pixel.
 Ret: - None.
______________________________________________________________________________*/
static inline void gldmemfb_CopyPxl (void *srcPxl, void *dstPxl)
{
#if (GLDCFG_MEMFB_BYTE_PER_PXL == 1)
    *(uint8_t *)dstPxl = *(uint8_t *)srcPxl;
#elif (GLDCFG_MEMFB_BYTE_PER_PXL == 2)
    *(uint16_t *)dstPxl = *(uint16_t *)srcPxl;
#elif (GLDCFG_MEMFB_BYTE_PER_PXL == 3)
    (uint8_t *)dstPxl[0] = (uint16_t *)srcPxl[0];
    (uint8_t *)dstPxl[1] = (uint16_t *)srcPxl[1];
    (uint8_t *)dstPxl[2] = (uint16_t *)srcPxl[2];
#elif (GLDCFG_MEMFB_BYTE_PER_PXL == 4)
    *(uint32_t *)dstPxl = *(uint32_t *)srcPxl;
#endif
}

/* function binded with display driver */
void *gldmemfb_GetMemFb (gldcoord_Area_t *canvas_p, const gldcoord_Area_t *area_p);
void gldmemfb_SetMemFb (gldhaldisp_t *disp, uint8_t fbIdx);
void gldmemfb_GetMemRect (glddrawopt_MemRect_t *memRect);

#endif /* GLDMEMFB_H_INCLUDED */

