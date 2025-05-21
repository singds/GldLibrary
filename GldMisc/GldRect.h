#ifndef GLDRECT_H_INCLUDED
#define GLDRECT_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldCfg.h"
#include "GldMisc/GldCoord.h"

//====================================================================== DEFINES

typedef struct
{
    void *MemStart; // memory quad lowest address
    gldcoord_Area_t Area;
    uint8_t Bpp; // byte per pixel
    gldcoord_Ori_e Ori; // orientamento dei pixel in memoria
    uint16_t Width; // memory quad line width
    uint16_t Height; // memory quad num lines
    uint16_t Offset; // memory quad offset between consecutive lines
} gldrect_t;

typedef struct
{
    /* la superficie sorgente e quella di destinazione devono avere stessi bpp.
    possono però avere orientamenti differenti. Le prestazioni di gran lunga
    migliori si ottengono quando le superfici hanno lo stesso orientamento.
    */
    gldrect_t SrcCanvas; /* superficie di memoria sorgente */
    gldrect_t DstCanvas; /* superficie di memoria di destinazione. */
    
    
    /* coordinata dell'angolo top-left dell'area di destinazione (interna al
    rettangolo di memoria di destinazione) */
    gldcoord_Point_t AreaSrcPoint;
    /* coordinata dell'angolo top-left dell'area sorgente (interna al
    rettangolo di memoria di sorgente) */
    gldcoord_Point_t AreaDstPoint;
    /* dimensione x dell'area da copiare */
    gldcoord_t AreaXSize;
    /* dimensione y dell'area da copiare */
    gldcoord_t AreaYSize;
} gldrect_CopyMemArea_t;

#define GLDRECT_NEXT_PXL_ORI_0(rect, pxlPtr)           ((void *)((uint32_t)pxlPtr + (rect)->Bpp))
#define GLDRECT_NEXT_PXL_ORI_90(rect, pxlPtr)          ((void *)((uint32_t)pxlPtr - ((rect)->Width + (rect)->Offset) * (rect)->Bpp))
#define GLDRECT_NEXT_PXL_ORI_180(rect, pxlPtr)         ((void *)((uint32_t)pxlPtr - (rect)->Bpp))
#define GLDRECT_NEXT_PXL_ORI_270(rect, pxlPtr)         ((void *)((uint32_t)pxlPtr + ((rect)->Width + (rect)->Offset) * (rect)->Bpp))
    
#define GLDRECT_GET_PXL_ORI_0(rect, x, y) \
(void *) \
( \
    (uint32_t)(rect)->MemStart + \
    (((y) * ((rect)->Width + (rect)->Offset)) + (x)) * (rect)->Bpp \
)

#define GLDRECT_GET_PXL_ORI_90(rect, x, y) \
(void *) \
( \
    (uint32_t)(rect)->MemStart + \
    ((((rect)->Height - (x) - 1) * ((rect)->Width + (rect)->Offset)) + (y)) * (rect)->Bpp \
)

#define GLDRECT_GET_PXL_ORI_180(rect, x, y) \
(void *) \
( \
    (uint32_t)(rect)->MemStart + \
    ((((rect)->Height - (y) - 1) * ((rect)->Width + (rect)->Offset)) + (rect)->Width - (x) - 1) * (rect)->Bpp \
)

#define GLDRECT_GET_PXL_ORI_270(rect, x, y) \
(void *) \
( \
    (uint32_t)(rect)->MemStart + \
    (((x) * ((rect)->Width + (rect)->Offset)) + (rect)->Width - (y) - 1) * (rect)->Bpp \
)

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void *gldrect_GetFrameMemoryPxl (gldrect_t *rect, gldcoord_t x, gldcoord_t y);
void gldrect_GetFrameMemoryRect (gldrect_t *inRect, gldrect_t *outRect);
void gldrect_CopyMemArea (gldrect_CopyMemArea_t *prm);
void gldrect_SetWidthHeightAsFull (gldrect_t *rect);

#endif /* GLDCOLOR_H_INCLUDED */
