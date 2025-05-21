//===================================================================== INCLUDES
#include "GldCore/GldMemFb.h"

#include <string.h>
#include "GldCore/GldBoost.h"

//====================================================================== DEFINES
#define L_FRAME_BUFFER_ADDR  ((uint32_t)MemoryFb)
#define L_FRAME_BUFFER_X_RES (gldmemfb_MemFbWidth)
#define L_FRAME_BUFFER_Y_RES (gldmemfb_MemFbHeight)

#if (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((y)*L_FRAME_BUFFER_X_RES) + (x)) * GLDCFG_MEMFB_BYTE_PER_PXL)
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__90)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((L_FRAME_BUFFER_X_RES - (x)-1) * L_FRAME_BUFFER_Y_RES) + (y)) * GLDCFG_MEMFB_BYTE_PER_PXL)
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__180)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((L_FRAME_BUFFER_Y_RES - (y)-1) * L_FRAME_BUFFER_X_RES) + L_FRAME_BUFFER_X_RES - (x)-1) * GLDCFG_MEMFB_BYTE_PER_PXL)
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__270)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((x)*L_FRAME_BUFFER_Y_RES) + L_FRAME_BUFFER_Y_RES - (y)-1) * GLDCFG_MEMFB_BYTE_PER_PXL)
#else
#error "Wrong orientation !"
#endif

//=========================================================== PRIVATE PROTOTYPES
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr);
static gldcolor_t GetPxl (gldcoord_t x, gldcoord_t y);
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr);

static void *GetMemoryRect (gldcoord_Area_t *area_p, uint16_t *width, uint16_t *height, uint16_t *offset);
static void SetDisplayCanvas (gldcoord_Area_t *area);

//============================================================= STATIC VARIABLES
#if (GLDCFG_MEMFB_ENABLED)
#if (GLDCFG_MEMFB_ADDRESS == 0)
/* static allocated pre-rendering buffer memory */
static uint8_t MemoryFb[GLDCFG_MEMFB_PXL_SIZE * GLDCFG_MEMFB_BYTE_PER_PXL];
#else
static uint8_t *MemoryFb = (uint8_t *)GLDCFG_MEMFB_ADDRESS;
#endif
#else
static uint8_t *MemoryFb = NULL;
#endif

//============================================================= GLOBAL VARIABLES
gldhaldisp_t gldmemfb_Disp = {
    .SetPxl = SetPxl,
    .GetPxl = GetPxl,
    .FillArea = FillArea,
};
gldcoord_t gldmemfb_MemFbWidth, gldmemfb_MemFbHeight;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Ottieni un memory buffer per rappresentare l'area 'area_p'. Il driver
        'gldmemfb_Disp' viene inoltre configurato per renderizzare la porzione
        di area restituita.
 Arg: - <canvas_p>[out] Viene scritta con le coordinate della piu' grande
            sotto-area di 'area_p' che si riesce a rappresentare con il memory
            buffer ottenuto.
        <aprea_p>[in] Puntatore all'area che si vorrebbe rappresentare con il
            memory buffer.
 Ret: - Restituisce il puntatore al buffer di memoria.
______________________________________________________________________________*/
void *gldmemfb_GetMemFb (gldcoord_Area_t *canvas_p, const gldcoord_Area_t *area_p)
{
    uint32_t lines; // available lines
    
    canvas_p->X1 = area_p->X1;
    canvas_p->X2 = area_p->X2;
    canvas_p->Y1 = area_p->Y1;
    lines = GLDCFG_MEMFB_PXL_SIZE / gldcoord_AreaWidth (area_p);
    canvas_p->Y2 = _MIN_ (area_p->Y1 + lines, GLDCOORD_MAX);

    gldcoord_AreaIntersection (canvas_p, canvas_p, area_p);
    SetDisplayCanvas (canvas_p);
    return MemoryFb;
}

#if (GLDCFG_DOUBLE_BUFFER == 1 && GLDCFG_MEMFB_ENABLED == 0)
/*______________________________________________________________________________
 Desc:  Configura il driver 'gldmemfb_Disp' per renderizzare direttamente su un
        dei due framebuffer.
 Arg: - <disp>[in] display driver che fornisce i framebuffer.
        <fbIdx>[in] indice del framebuffer su cui vuoi renderizzare.
 Ret: - None.
______________________________________________________________________________*/
void gldmemfb_SetMemFb (gldhaldisp_t *disp, uint8_t fbIdx)
{
    gldcoord_Area_t canvas;
    
    canvas.X1 = disp->Canvas.X1;
    canvas.X2 = disp->Canvas.X2;
    canvas.Y1 = disp->Canvas.Y1;
    canvas.Y2 = disp->Canvas.Y2;
    SetDisplayCanvas (&canvas);
    if (disp->LineOffset) {
        /* attenzione ! il rendering diretto sul frambuffer buffer attualmente
        non supporta la presenza del line offset */
        GLDCFG_PRINTF ("err: configurazione non supportata !");
        GLDCFG_ASSERT (0);
    }
    MemoryFb = disp->GetFramePtr (fbIdx);
}
#endif

/*______________________________________________________________________________
 Desc:  Ottieni le informazioni caratteristiche del 'rettangolo' di memoria che
        rappresenta il framebuffer parziale.
 Arg: - <memRect> struttura scritta con le informazioni sul rettangolo di memoria.
 Ret: - None.
______________________________________________________________________________*/
void gldmemfb_GetMemRect (glddrawopt_MemRect_t *memRect)
{
    memRect->Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
    memRect->BuffPtr = MemoryFb;
    memRect->X1 = gldmemfb_Disp.Canvas.X1;
    memRect->Y1 = gldmemfb_Disp.Canvas.Y1;
    memRect->Xsize = gldcoord_AreaWidth (&gldmemfb_Disp.Canvas);
    memRect->Ysize = gldcoord_AreaHeight (&gldmemfb_Disp.Canvas);
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Imposta il colore di un pixel del memory frame buffer.
 Arg: - <x>[in] Coordinata x del pixel.
        <y>[in] Coordinata y del pixel.
        <clr>[in] Colore del pixel in formato standard ARGB8888.
 Ret: - None.
______________________________________________________________________________*/
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr)
{
    x -= gldmemfb_Disp.Canvas.X1;
    y -= gldmemfb_Disp.Canvas.Y1;
    
    gldcfg_WritePxl ((void *)L_FRAME_BUFFER_PXL_ADDR (x, y), clr);
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il colore presente nel memory frame buffer alle coordinate
        indicate.
 Arg: - <x>[in] Coordinata x del pixel.
        <y>[in] Coordinata y del pixel.
 Ret: - Colore del pixel in formato standard ARGB8888.
______________________________________________________________________________*/
static gldcolor_t GetPxl (gldcoord_t x, gldcoord_t y)
{
    x -= gldmemfb_Disp.Canvas.X1;
    y -= gldmemfb_Disp.Canvas.Y1;

    return gldcfg_ReadPxl ((void *)L_FRAME_BUFFER_PXL_ADDR (x, y));
}

/*______________________________________________________________________________ 
 Desc:  Riempi un'area rettangolare del memory frame buffer con un colore.
 Arg: - <x0>[in] Coordinata x vertice top-left del rettangolo.
        <y0>[in] Coordinata y vertice top-left del rettangolo.
        <x_size>[in] Dimensione x del rettangolo [pxl].
        <y_size>[in] Dimensione y del rettangolo [pxl].
        <clr>[in] Colore da utilizzare per riempire in formato ARGB8888.
 Ret: - None.
______________________________________________________________________________*/
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr)
{
    gldcoord_Area_t area;
    uint8_t *dstPxl;
    uint8_t *srcLine; // prima linea riempita di colore
    uint16_t width; // larghezza area (in pxl)
    uint16_t height; // altezza area (in pxl)
    uint16_t offset; // pixel da saltare fra la dine di una linea e la successiva
    uint32_t sampleClr; // campione di colore

    area.X1 = x0;
    area.Y1 = y0;
    area.X2 = x0 + x_size;
    area.Y2 = y0 + y_size;

    dstPxl = GetMemoryRect (&area, &width, &height, &offset);
    
    // creo un campione di colore
    gldcfg_WritePxl (&sampleClr, clr);

    if (gldboost_Boost &&
        gldboost_Boost->FillArea (dstPxl, width, height,
                                  offset, &sampleClr, GLDCFG_MEMFB_BYTE_PER_PXL) == 0)
    {
        ; // riempimento completato con accelerazione hardware
    }
    
    /* rempio la prima linea */
    srcLine = dstPxl;
    for (int k = 0; k < width; k++)
    {
        gldmemfb_CopyPxl (&sampleClr, dstPxl);
        dstPxl += GLDCFG_MEMFB_BYTE_PER_PXL;
    }
    dstPxl += offset * GLDCFG_MEMFB_BYTE_PER_PXL;

    /* copio la linea appena colorata nelle successive:
       la funzione memcpy sftrutta al meglio le istruzioni del processore
       rendendo pi� veloce questa operazione */
    while (--height)
    {
        memcpy (dstPxl, srcLine, (width * GLDCFG_MEMFB_BYTE_PER_PXL));
        dstPxl += (width + offset) * GLDCFG_MEMFB_BYTE_PER_PXL;
    }
    return 0;
}


/*______________________________________________________________________________ 
 Desc:  Ottieni i parametri caratteristici del rettangolo di memoria che
        rappresenta l'area 'area_p' nel frame buffer.
 Arg: - <area_p> [in]  coordinate dell'area interna al frame buffer.
        <width>  [out] larghezza in pixel.
        <height> [out] altezza in pixel.
        <offset> [out] pixel interposti tra una linea e la successiva.
 Ret: - Restituisce il puntatore all'indirizzo di memoria pi� basso del rettangolo
        di pixel.
______________________________________________________________________________*/
static void *GetMemoryRect (gldcoord_Area_t *area_p, uint16_t *width, uint16_t *height, uint16_t *offset)
{
    /* puntatore indirizzo di memoria pi� basse del rettangolo di pixel */
    void *ptrRectMem = NULL;
    gldcoord_t x0, y0, x_size, y_size;

    x0 = area_p->X1 - gldmemfb_Disp.Canvas.X1;
    y0 = area_p->Y1 - gldmemfb_Disp.Canvas.Y1;
    x_size = gldcoord_AreaWidth (area_p);
    y_size = gldcoord_AreaHeight (area_p);

#if ((GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0) || (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__180))
    *width = x_size;
    *height = y_size;
    *offset = L_FRAME_BUFFER_X_RES - x_size;
#else
    *width = y_size;
    *height = x_size;
    *offset = L_FRAME_BUFFER_Y_RES - y_size;
#endif

#if (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0);
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__180)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0 + (y_size - 1));
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__90)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0);
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__270)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0 + (y_size - 1));
#endif

    return ptrRectMem;
}

/*______________________________________________________________________________
 Desc:  Imposta le coordinate dell'area del framebuffer display.
 Arg: - <area> coordinate del framebuffer display.
 Ret: - None.
______________________________________________________________________________*/
static void SetDisplayCanvas (gldcoord_Area_t *area)
{
    /* imposto la tela del display virtuale come l'area restituita:
       Ora il display 'gldmemfb_Disp' mappa l'area 'canvas_p'. */
    gldcoord_AreaCopy (&gldmemfb_Disp.Canvas, area);

    /* le informazioni che scrivo in queste variabili.
       le potrei ricavare anche da 'gldmemfb_Disp.Canvas'.
       risulta pero' piu' veloce averle gia' disponibili.
    */
    gldmemfb_MemFbWidth = gldcoord_AreaWidth (&gldmemfb_Disp.Canvas);
    gldmemfb_MemFbHeight = gldcoord_AreaHeight (&gldmemfb_Disp.Canvas);
}
