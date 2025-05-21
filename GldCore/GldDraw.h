#ifndef GLDDRAW_H_INCLUDED
#define GLDDRAW_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldHal/GldHalDisp.h"
#include "GldMisc/GldColor.h"
#include "GldMisc/GldCoord.h"
#include "GldMisc/GldRect.h"
#include "GldRes/GldFont.h"
#include "GldRes/GldTxt.h"
#include "GldRes/GldImg.h"

//====================================================================== DEFINES

typedef struct
{
    enum
    {
        GLDDRAW_BMPFILTER_NONE,
        GLDDRAW_BMPFILTER_GRAYSCALE,
        GLDDRAW_BMPFILTER_BLEND,
    } Type;
    gldcolor_t BlendClr;
    gldcolor_Opa_t BlendOpa;
} glddraw_BmpFilter_t;

typedef struct
{
    const gldimg_BmpHeaderImage_t *BmpIh;
    gldcoord_Area_t *AreaView;
    const char *MapPtr;
    const char *Paletta;
    int LineOff;
    int LeftByteOff;
    int LeftBitOff;
} glddraw_BmpData_t;

typedef struct
{
    const gldimg_BmpHeaderImage_t *BmpIh;
    gldcoord_Area_t *AreaView;
    FILE *File;
    int MapOff;
    int LineSz;
    const char *Paletta;
    int LineOff;
    int LeftByteOff;
    int LeftBitOff;
} glddraw_BmpFsData_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void glddraw_Pxl (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcolor_t clr);
void glddraw_PxlOpa (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcolor_t clr, gldcolor_Opa_t opacity);
void glddraw_FillArea (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcolor_t clr);
void glddraw_FillRound (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcoord_t radius, gldcolor_t clr);
void glddraw_EmptyRound (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcoord_t size, gldcoord_t radius, gldcolor_t clr);
void glddraw_Line (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcolor_t clr);
void glddraw_Circle (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius, gldcolor_t clr);
void glddraw_FillCircle (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius, gldcolor_t clr);
void glddraw_Arc (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius, gldcoord_t start, gldcoord_t end, gldcolor_t clr);
void glddraw_ThickArc (gldhaldisp_t *disp, gldcoord_t xc, gldcoord_t yc, gldcoord_t radiusStart, gldcoord_t radiusEnd, gldcoord_t start, gldcoord_t end, gldcolor_t clr);
int16_t glddraw_Printf (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align, gldfont_t *font, gldcolor_t f_clr, gldcolor_t b_clr, const char *format, ...);
void glddraw_Bmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align, const char *bmp_p, const glddraw_BmpFilter_t *filter);
void glddraw_FsBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align, const char *f_name, const glddraw_BmpFilter_t *filter);
void glddraw_Triangle (gldhaldisp_t *disp, const gldcoord_Point_t *points, gldcolor_t color);
void glddraw_PxlMap (gldhaldisp_t *disp, const uint8_t *pxlMap, gldcoord_t x, gldcoord_t y, uint32_t xSz, uint32_t ySz, gldcoord_Ori_e ori, gldcolor_PxlFormat_t format);

#endif /* GLDDRAW_H_INCLUDED */
