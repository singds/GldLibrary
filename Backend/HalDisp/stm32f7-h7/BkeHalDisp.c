//===================================================================== INCLUDES
#include "BkeHalDisp.h"

#include "GldCfg.h"
#include <string.h>

//====================================================================== DEFINES
#define L_FRAME_BUFFER_ORI__0   0 /* tested */
#define L_FRAME_BUFFER_ORI__90  1 /* tested */
#define L_FRAME_BUFFER_ORI__180 2 /* tested */
#define L_FRAME_BUFFER_ORI__270 3 /* tested */

//*** <<< Use Configuration Wizard in Context Menu >>> ***
//  <h> Display STM32F7-H7

//   <o> Orientamento
//      <0=> 0      [L_FRAME_BUFFER_ORI__0]
//      <1=> 90     [L_FRAME_BUFFER_ORI__90]
//      <2=> 180    [L_FRAME_BUFFER_ORI__180]
//      <3=> 270    [L_FRAME_BUFFER_ORI__270]
//   <i> Orientamento del framebuffer
#define L_FRAME_BUFFER_ORI 1
//   <o> Framebuffer x resolution [pxl] <0-3840>
//   <i> Width in pixel del frame buffer
#define L_FRAME_BUFFER_X_RES 768
//   <o> Framebuffer y resolution [pxl] <0-3840>
//   <i> Height in pixel del frame buffer
#define L_FRAME_BUFFER_Y_RES 1366
//   <o> Framebuffer line pad [pxl] <0-3840>
//   <i> Pixel at the and of each frame buffer line
//   <i> Can be used to align the frame buffer line (default 0)
#define L_FRAME_BUFFER_LINE_PAD 10
/* il supporto ai dummy pixel su ciascuna linea è stato aggiunto per poter garantire
l'allineamento a 64byte di ciascuna linea del framebuffer su stm32h7.
L'allineamento delle linee a 64byte garantisce prestazioni migliori in termini
di memory bandwidth.
    vedi (4.5.2) Optimizing the LTDC framebuffer fetching from external memories (SDRAM or SRAM)
    in https://www.st.com/resource/en/application_note/dm00287603-lcdtft-display-controller-ltdc-on-stm32-mcus-stmicroelectronics.pdf
*/

//   <e> Dma2d
//   <i> Abilita l'utilizzo del dma2d per accelerare le operazioni copiatura o
//   <i> fill di aree di memoria.
#define L_ENABLE_DMA2D 1
//    <o> Min to dma2d fill [pxl] <0-3840>
//    <i> Minima dimensione dell'area da riempire per cui sia conveniente sfruttare
//    <i> l'accelerazione del dma2d
#define L_DMA2D_FILL_MIN_AREA (50)
//    <o> Min to dma2d copy [pxl] <0-3840>
//    <i> Minima dimensione dell'area da copiare per cui sia conveniente sfruttare
//    <i> l'accelerazione del dma2d
#define L_DMA2D_FLUSH_MIN_AREA (50)
//   </e>
//   <e> Force single frame
//   <i> Forza l'utilizzo di un singolo frame, utile in debug.
#define L_FORCE_SINGLE_FRAME   0
//   </e>
//  </h>
//*** <<< end of configuration section >>>    ***

#define L_FRAME_BUFFER_ADDR WorkingFbAddress

/* frame buffer byte per pixel */
#define L_BYTE_PER_PXL 2

#if (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__0)
#define L_FRAME_BUFFER_LINE_WIDTH     (L_FRAME_BUFFER_X_RES + L_FRAME_BUFFER_LINE_PAD)
#define L_FRAME_BUFFER_TOTAL_SIZE     (L_FRAME_BUFFER_LINE_WIDTH * L_FRAME_BUFFER_Y_RES * L_BYTE_PER_PXL)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((y)*L_FRAME_BUFFER_LINE_WIDTH) + (x)) * L_BYTE_PER_PXL)
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__90)
#define L_FRAME_BUFFER_LINE_WIDTH     (L_FRAME_BUFFER_Y_RES + L_FRAME_BUFFER_LINE_PAD)
#define L_FRAME_BUFFER_TOTAL_SIZE     (L_FRAME_BUFFER_LINE_WIDTH * L_FRAME_BUFFER_X_RES * L_BYTE_PER_PXL)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((L_FRAME_BUFFER_X_RES - (x)-1) * L_FRAME_BUFFER_LINE_WIDTH) + (y)) * L_BYTE_PER_PXL)
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__180)
#define L_FRAME_BUFFER_LINE_WIDTH     (L_FRAME_BUFFER_X_RES + L_FRAME_BUFFER_LINE_PAD)
#define L_FRAME_BUFFER_TOTAL_SIZE     (L_FRAME_BUFFER_LINE_WIDTH * L_FRAME_BUFFER_Y_RES * L_BYTE_PER_PXL)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((L_FRAME_BUFFER_Y_RES - (y)-1) * L_FRAME_BUFFER_LINE_WIDTH) + L_FRAME_BUFFER_X_RES - (x)-1) * L_BYTE_PER_PXL)
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__270)
#define L_FRAME_BUFFER_LINE_WIDTH     (L_FRAME_BUFFER_Y_RES + L_FRAME_BUFFER_LINE_PAD)
#define L_FRAME_BUFFER_TOTAL_SIZE     (L_FRAME_BUFFER_LINE_WIDTH * L_FRAME_BUFFER_X_RES * L_BYTE_PER_PXL)
#define L_FRAME_BUFFER_PXL_ADDR(x, y) (L_FRAME_BUFFER_ADDR + (((x)*L_FRAME_BUFFER_LINE_WIDTH) + L_FRAME_BUFFER_Y_RES - (y)-1) * L_BYTE_PER_PXL)
#else
#error "Wrong orientation !"
#endif

#define L_SWAP_INTERRUPT 1

extern DMA2D_HandleTypeDef hdma2d;

//=========================================================== PRIVATE PROTOTYPES
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr);
static gldcolor_t GetPxl (gldcoord_t x, gldcoord_t y);
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr);
static void FlushArea (void *mem_p, gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size);
void *GetFramePtr (uint8_t index);
void SetVisibleFrame (uint8_t index);

static void *GetMemoryRect (gldcoord_Area_t *area_p, uint16_t *width, uint16_t *height, uint16_t *offset);

//============================================================= STATIC VARIABLES
static bkehaldisp_Init_t *Init;
static uintptr_t WorkingFbAddress;
static uint16_t LcdIntActiveLine, LcdIntPorchLine;
struct
{
    uint32_t Address;
    volatile bool Waiting;
} Swap;

//============================================================= GLOBAL VARIABLES
bkehaldisp_Init_t bkehaldisp_Init;
gldhaldisp_t bkehaldisp_Display;
uint8_t bkehaldisp_VisibleFrame; // indice del frame che � attualmente visibile
uint8_t bkehaldisp_WorkingFrame; // indice del frame in lavoro: su cui viene renderizzata la pagina

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
int32_t bkehaldisp_InitModule (void)
{
    int32_t ret = _STATUS_OK_;
        
    Init = &bkehaldisp_Init;
    LTDC->IER = 0;
#if (L_SWAP_INTERRUPT == 1)
    LcdIntActiveLine = (LTDC->BPCR & 0x7FF) - 1;
    LcdIntPorchLine = (LTDC->AWCR & 0x7FF) - 1;
    LTDC->LIPCR = LcdIntActiveLine;
    LTDC->IER |= LTDC_IER_LIE;
    NVIC_EnableIRQ (LTDC_IRQn);
#endif
    LTDC->IER |= LTDC_IER_FUIE | LTDC_IER_TERRIE;
    NVIC_EnableIRQ (LTDC_ER_IRQn);

    bkehaldisp_Display.Canvas.X1 = 0;
    bkehaldisp_Display.Canvas.Y1 = 0;
    bkehaldisp_Display.Canvas.X2 = L_FRAME_BUFFER_X_RES;
    bkehaldisp_Display.Canvas.Y2 = L_FRAME_BUFFER_Y_RES;
    bkehaldisp_Display.Ori = (gldcoord_Ori_e)L_FRAME_BUFFER_ORI;
    bkehaldisp_Display.LineOffset = L_FRAME_BUFFER_LINE_PAD;

    memcpy (&bkehaldisp_Display.Clip, &bkehaldisp_Display.Canvas, sizeof (gldcoord_Area_t));

    bkehaldisp_Display.SetPxl = SetPxl;
    bkehaldisp_Display.GetPxl = GetPxl;
    bkehaldisp_Display.FillArea = FillArea;
    bkehaldisp_Display.FlushArea = FlushArea;
    bkehaldisp_Display.GetFramePtr = GetFramePtr;
    bkehaldisp_Display.SetVisibleFrame = SetVisibleFrame;
    bkehaldisp_SetWorkingFrame (0);
    bkehaldisp_SetVisibleFrame (0);

    return ret;
}

/*______________________________________________________________________________ 
 Desc:  Setta il framebuffer su cui le funzioni del driver vengono eseguite.
 Arg: - <num>[in] numero del framebuffer.
 Ret: - None.
______________________________________________________________________________*/
void bkehaldisp_SetWorkingFrame (uint8_t num)
{
#if (L_FORCE_SINGLE_FRAME == 1)
    num = 0; // con questa linea vedi sempre lo stesso frame
#endif
    num = _MIN_ (1, num);
    if (Init->FuncEvent)
        Init->FuncEvent (&bkehaldisp_Display, BKEHALDISP_EVENT_SET_WORKING_FRAME, (void *)(uintptr_t)num);
    WorkingFbAddress = Init->FbAddress[num];
    bkehaldisp_WorkingFrame = num;
}

/*______________________________________________________________________________ 
 Desc:  Visualizza uno specifico framebuffer.
 Arg: - <num>[in] numero del framebuffer da visualizzare.
 Ret: - None.
______________________________________________________________________________*/
void bkehaldisp_SetVisibleFrame (uint8_t num)
{
#if (L_FORCE_SINGLE_FRAME == 1)
    num = 0; // con questa linea vedi sempre lo stesso frame
#endif
    num = _MIN_ (1, num);
#if (L_SWAP_INTERRUPT == 1)
    bkehaldisp_CleanFrame (num); /* finalizzo scritture del processore */
    
    if (Init->FuncEvent)
        Init->FuncEvent (&bkehaldisp_Display, BKEHALDISP_EVENT_SET_VISIBLE_FRAME, (void *)(uintptr_t)num);
    Swap.Address = Init->FbAddress[num];
    Swap.Waiting = true;
    /* aspetto fino a che il cambio frame non e' avvenuto */
    while (Swap.Waiting)
        ;
#else
    LTDC_LAYER (&hltdc, 0)->CFBAR = Init->FbAddress[num];
    /* reload shadow register:
    -IMR: immediatly
    -VBR: during the vertical blanking period (at the beginning of the first
    line after the active display area).
    This bit is set by software and cleared only by hardware after reload (it cannot be cleared
    through register write once it is set).
    */
    hltdc.Instance->SRCR = LTDC_SRCR_VBR; // LTDC_SRCR_VBR / LTDC_SRCR_IMR
#endif

    bkehaldisp_VisibleFrame = num;
}

/*______________________________________________________________________________ 
 Desc:  Clean di tutta la cache del framebuffer. I dati in cache vengono scritti
        effettivamente in ram.
 Arg: - <num>[in] numero del framebuffer da flushare.
 Ret: - None.
______________________________________________________________________________*/
void bkehaldisp_CleanFrame (uint8_t num)
{   /* Finalizzo le scritture in RAM */
#if (L_FORCE_SINGLE_FRAME == 1)
    num = 0;
#endif
    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
    {   // If Data Cache is enabled
        SCB_CleanDCache_by_Addr ((void *)Init->FbAddress[num], L_FRAME_BUFFER_TOTAL_SIZE);
    }
}

/*______________________________________________________________________________ 
 Desc:  Invalida la cache del framebuffer. Il framebuffer in cache viene scartato.
        Le successive letture dal framebuffer vengono portate a termine leggendo
        direttamente in memoria.
 Arg: - <num>[in] numero del frambuffer da invalidare.
 Ret: - None.
______________________________________________________________________________*/
void bkehaldisp_InvalidateFrame (uint8_t num)
{   /* Finalizzo le scritture in RAM */
#if (L_FORCE_SINGLE_FRAME == 1)
    num = 0;
#endif
    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
    {   // If Data Cache is enabled
        SCB_InvalidateDCache_by_Addr ((void *)Init->FbAddress[num], L_FRAME_BUFFER_TOTAL_SIZE);
    }
}

#if (L_SWAP_INTERRUPT == 1)
void LTDC_IRQHandler (void)
{
    /* ltdc interrupt
    only "Line" e "Register reload" trigger this interrupt */
    if (LTDC->ISR & 1)
    {
        LTDC->ICR = 1;
        if (LTDC->LIPCR == LcdIntActiveLine)
        {
            //entering active area
            LTDC->LIPCR = LcdIntPorchLine;
            // Swap frame buffers immediately instead of waiting for the task to be scheduled in.
            // Note: task will also swap when it wakes up, but that operation is guarded and will not have
            // any effect if already swapped.
            if (Swap.Waiting)
            {
                LTDC_Layer1->CFBAR = (uint32_t)Swap.Address;
                /* Reload immediate */
                LTDC->SRCR = (uint32_t)LTDC_SRCR_IMR;
                Swap.Waiting = false;
            }
        } else
        {
            //exiting active area
            LTDC->LIPCR = LcdIntActiveLine;
        }
    }
}
#endif

void LTDC_ER_IRQHandler (void)
{   /* ltdc error interrupt
    only "FIFO underrun" e "Transfer error" trigger this interrupt */
    uint32_t isr, icr = 0;
    
    isr = LTDC->ISR;
    if (isr & LTDC_ISR_FUIF) {
        /* fifo underrun error
        l'LTDC ha bisogno di dati che non sono pronti nel fifo. Probabile sovraccarico
        del bus di memoria. */
        icr |= LTDC_ICR_CFUIF;
    }
    if (isr & LTDC_ISR_TERRIF) {
        icr |= LTDC_ICR_CTERRIF;
    }
    LTDC->ICR = icr;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void SetPxl (gldcoord_t x, gldcoord_t y, gldcolor_t clr)
{
    uint16_t *ptrUsa, color;

    ptrUsa = (uint16_t *)L_FRAME_BUFFER_PXL_ADDR (x, y);

    color = ((GLDCOLOR_GET_R (clr) & 0xf8) << 8);
    color |= ((GLDCOLOR_GET_G (clr) & 0xfC) << 3);
    color |= ((GLDCOLOR_GET_B (clr) & 0xf8) >> 3);

    *ptrUsa = color;
}

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static gldcolor_t GetPxl (gldcoord_t x, gldcoord_t y)
{
    uint16_t *ptrUsa, color;
    gldcolor_t retClr;

    ptrUsa = (uint16_t *)L_FRAME_BUFFER_PXL_ADDR (x, y);
    
    color = *ptrUsa;
    retClr = GLDCOLOR_OR_R (((color & 0xf800) >> 8)) |
             GLDCOLOR_OR_G ((color & 0x07e0) >> 3) |
             GLDCOLOR_OR_B (((color & 0x001f) << 3));
    return retClr;
}

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t FillArea (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr)
{
    uint16_t *dstPxl;
    uint16_t width, height, offset;
    gldcoord_Area_t fillArea;

    fillArea.X1 = x0;
    fillArea.Y1 = y0;
    fillArea.X2 = x0 + x_size;
    fillArea.Y2 = y0 + y_size;

    dstPxl = GetMemoryRect (&fillArea, &width, &height, &offset);

#if L_ENABLE_DMA2D != 0
    if ((x_size * y_size) > L_DMA2D_FILL_MIN_AREA)
    {
        /*________________________________________________________ DMA2D FILL */
        /*________________________________________________________ DMA2D FILL */
        Init->Dma2d->Instance = DMA2D;
        Init->Dma2d->Init.Mode = DMA2D_R2M;
        Init->Dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
        Init->Dma2d->Init.OutputOffset = offset;
        if (HAL_DMA2D_Init (Init->Dma2d) != HAL_OK)
        { /* error */
            for (;;)
                ;
        }

        /* Il colore passato a 'pdata' deve avere il formato ARGB8888 */
        HAL_DMA2D_Start (Init->Dma2d, clr, (uint32_t)dstPxl, width, height);
        HAL_DMA2D_PollForTransfer (Init->Dma2d, 1000);
    } else
#endif
    {
        /*_______________________________________________________ MEMCPY FILL */
        /*_______________________________________________________ MEMCPY FILL */
        uint16_t color, *srcLine;

        color = ((GLDCOLOR_GET_R (clr) & 0xf8) << 8);
        color |= ((GLDCOLOR_GET_G (clr) & 0xfC) << 3);
        color |= ((GLDCOLOR_GET_B (clr) & 0xf8) >> 3);

        srcLine = dstPxl;

        /* coloro la prima linea */
        for (int k = 0; k < width; k++)
        {
            *dstPxl = color;
            dstPxl++;
        }
        dstPxl += offset;

        /* copio la linea appena colorata nelle successive:
        la funzione memcpy sftrutta al meglio le istruzioni del processore
        rendendo pi� veloce questa operazione */
        while (--height)
        {
            memcpy (dstPxl, srcLine, (width * L_BYTE_PER_PXL));
            dstPxl += width + offset;
        }
    }

    return 0;
}

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void FlushArea (void *mem_p, gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size)
{
#if (GLDCFG_MEMFB_ORI != L_FRAME_BUFFER_ORI && GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0)
    
#elif (GLDCFG_MEMFB_ORI != L_FRAME_BUFFER_ORI)
#error "l'orentamento del display e del buffer di pre-rendering deve coincidere"
#else
    gldcoord_Area_t dst_area;
    uint8_t *dst_mem, *src_mem;
    uint16_t dst_mem_w, dst_mem_h, dst_mem_off;
    
    dst_area.X1 = x0;
    dst_area.Y1 = y0;
    dst_area.X2 = x0 + x_size;
    dst_area.Y2 = y0 + y_size;

    dst_mem = GetMemoryRect (&dst_area, &dst_mem_w, &dst_mem_h, &dst_mem_off);

#if L_ENABLE_DMA2D != 0
    if ((x_size * y_size) > L_DMA2D_FLUSH_MIN_AREA)
    {
        /*_______________________________________________________ DMA2D FLUSH */
        /*_______________________________________________________ DMA2D FLUSH */
        Init->Dma2d->Instance = DMA2D;
        Init->Dma2d->Init.Mode = DMA2D_M2M;
        Init->Dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
        Init->Dma2d->Init.OutputOffset = dst_mem_off;
        if (HAL_DMA2D_Init (Init->Dma2d) != HAL_OK)
        { /* error */
            for (;;)
                ;
        }

        /* configuro i parametri del layer sorgente (foreground layer, index = 1) */
        Init->Dma2d->LayerCfg[1].InputOffset = 0;
        Init->Dma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
        Init->Dma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        Init->Dma2d->LayerCfg[1].InputAlpha = 0;
        Init->Dma2d->LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
        if (HAL_DMA2D_ConfigLayer (Init->Dma2d, 1) != HAL_OK)
        { /* error */
            for (;;)
                ;
        }

        HAL_DMA2D_Start (Init->Dma2d, (uint32_t)mem_p, (uint32_t)dst_mem, dst_mem_w, dst_mem_h);
        HAL_DMA2D_PollForTransfer (Init->Dma2d, 1000);
    } else
#endif
    {
        /*______________________________________________________ MEMCPY FLUSH */
        /*______________________________________________________ MEMCPY FLUSH */
        src_mem = mem_p;

        /* eseguo la copiatura a mano dell'area sorgente in quella di destinazione */
        while (dst_mem_h)
        {
            memcpy (dst_mem, src_mem, dst_mem_w * L_BYTE_PER_PXL);
            /* muovo i puntatori sorgente e destinazione */
            src_mem += dst_mem_w * L_BYTE_PER_PXL;
            dst_mem += (dst_mem_w + dst_mem_off) * L_BYTE_PER_PXL;

            dst_mem_h--;
        }
    }
#endif
}

/*______________________________________________________________________________
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void *GetFramePtr (uint8_t index)
{
#if (L_FORCE_SINGLE_FRAME == 1)
    index = 0;
#endif
    return (void *)Init->FbAddress[index];
}

/*______________________________________________________________________________
 Desc:  Vedi <GldHalDisp.h> per descrizione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void SetVisibleFrame (uint8_t index)
{
    bkehaldisp_SetVisibleFrame (index);
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

#if ((L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__0) || (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__180))
    *width = x_size;
    *height = y_size;
    *offset = L_FRAME_BUFFER_LINE_WIDTH - x_size;
#else
    *width = y_size;
    *height = x_size;
    *offset = L_FRAME_BUFFER_LINE_WIDTH - y_size;
#endif

#if (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__0)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0);
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__180)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0 + (y_size - 1));
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__90)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0 + (x_size - 1), y0);
#elif (L_FRAME_BUFFER_ORI == L_FRAME_BUFFER_ORI__270)
    ptrRectMem = (void *)L_FRAME_BUFFER_PXL_ADDR (x0, y0 + (y_size - 1));
#endif
    return ptrRectMem;
}
