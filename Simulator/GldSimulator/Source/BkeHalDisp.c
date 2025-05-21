//===================================================================== INCLUDES
#include "BkeHalDisp.h"

#include <string.h>
#include "GldSimulator.h"
#include "GldCfg.h"

void gldsimulator_InvalidateArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size);

//====================================================================== DEFINES
#define L_FRAME_BUFFER_ADDR                     ((uintptr_t)(&VideoRam))

#if (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__0)
	#define L_FRAME_BUFFER_PXL_ADDR(x, y)        (L_FRAME_BUFFER_ADDR + (((y) * BKEHALDISP_FRAME_BUFFER_X_RES) + (x)) * BKEHALDISP_BYTE_PER_PXL)
#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__90)
	#define L_FRAME_BUFFER_PXL_ADDR(x, y)        (L_FRAME_BUFFER_ADDR + (((BKEHALDISP_FRAME_BUFFER_X_RES - (x) - 1) * BKEHALDISP_FRAME_BUFFER_Y_RES) + (y)) * BKEHALDISP_BYTE_PER_PXL)
#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__180)
	#define L_FRAME_BUFFER_PXL_ADDR(x, y)        (L_FRAME_BUFFER_ADDR + (((BKEHALDISP_FRAME_BUFFER_Y_RES - (y) - 1) * BKEHALDISP_FRAME_BUFFER_X_RES) + BKEHALDISP_FRAME_BUFFER_X_RES - (x) - 1) * BKEHALDISP_BYTE_PER_PXL)
#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__270)
	#define L_FRAME_BUFFER_PXL_ADDR(x, y)        (L_FRAME_BUFFER_ADDR + (((x) * BKEHALDISP_FRAME_BUFFER_Y_RES) + BKEHALDISP_FRAME_BUFFER_Y_RES - (y) - 1) * BKEHALDISP_BYTE_PER_PXL)
#else
	#error "Wrong orientation !"
#endif

//=========================================================== PRIVATE PROTOTYPES
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr);
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr);
static void FlushArea (void *mem_p, gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size);

static void *GetMemoryRect (gldcoord_Area_t *area_p, uint16_t *width, uint16_t *height, uint16_t *offset);

//============================================================= STATIC VARIABLES
static uint8_t VideoRam[BKEHALDISP_FRAME_BUFFER_X_RES * BKEHALDISP_FRAME_BUFFER_Y_RES * BKEHALDISP_BYTE_PER_PXL];

//============================================================= GLOBAL VARIABLES
gldhaldisp_t bkehaldisp_Display;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
int bkehaldisp_Init (void)
{
	bkehaldisp_Display.Canvas.X1 = 0;
	bkehaldisp_Display.Canvas.Y1 = 0;
	bkehaldisp_Display.Canvas.X2 = BKEHALDISP_FRAME_BUFFER_X_RES;
	bkehaldisp_Display.Canvas.Y2 = BKEHALDISP_FRAME_BUFFER_Y_RES;
	
	memcpy (&bkehaldisp_Display.Clip, &bkehaldisp_Display.Canvas, sizeof (gldcoord_Area_t));
	
	bkehaldisp_Display.SetPxl = SetPxl;
	bkehaldisp_Display.FillArea = FillArea;
	bkehaldisp_Display.FlushArea = FlushArea;
	
	return (0);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
const void *bkehaldisp_GetFramebuffer (void)
{
	return VideoRam;
}


//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr)
{
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr)
{
	return 0;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void FlushArea (void *mem_p, gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size)
{
	gldcoord_Area_t dst_area;
	uint8_t *src_pxl, *dst_pxl;
	uint16_t dst_mem_w, dst_mem_h, dst_mem_off;
	
	dst_area.X1 = x0;
	dst_area.Y1 = y0;
	dst_area.X2 = x0 + x_size;
	dst_area.Y2 = y0 + y_size;


	src_pxl = mem_p;
	dst_pxl = GetMemoryRect (&dst_area, &dst_mem_w, &dst_mem_h, &dst_mem_off);

	/* eseguo la copiatura a mano dell'area sorgente in quella di destinazione */
	while (dst_mem_h)
	{
		for (uint32_t w = 0; w < dst_mem_w; w++)
		{
			uint16_t srcColor = *(uint16_t *)src_pxl;
			uint8_t red, green, blue;

			red =   (srcColor & 0xf800) >> 8;
			green = (srcColor & 0x07e0) >> 3;
			blue =  (srcColor & 0x001f) << 3;

			dst_pxl[0] = red;
			dst_pxl[1] = green;
			dst_pxl[2] = blue;

			dst_pxl += BKEHALDISP_BYTE_PER_PXL;
			src_pxl += GLDCFG_MEMFB_BYTE_PER_PXL;
		}
		dst_pxl += dst_mem_off * BKEHALDISP_BYTE_PER_PXL;
		dst_mem_h--;
	}
	gldsimulator_InvalidateArea (x0, y0, x_size, y_size);
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
	
	x0 = area_p->X1;
	y0 = area_p->Y1;
	x_size = gldcoord_AreaWidth (area_p);
	y_size = gldcoord_AreaHeight (area_p);
	
	#if ((BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__0) || (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__180))
	*width = x_size;
	*height = y_size;
	*offset = BKEHALDISP_FRAME_BUFFER_X_RES - x_size;
	#else
	*width = y_size;
	*height = x_size;
	*offset = BKEHALDISP_FRAME_BUFFER_Y_RES - y_size;
	#endif
	
	#if (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__0)
	ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0);
	#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__180)
	ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0 + (y_size - 1));
	#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__90)
	ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0);
	#elif (BKEHALDISP_FRAME_BUFFER_ORI == BKEHALDISP_FRAME_BUFFER_ORI__270)
	ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0 + (y_size - 1));
	#endif
	return ptrRectMem;
}
