//===================================================================== INCLUDES
#include "BkeHalBoost.h"

#include "GldCfg.h"

//====================================================================== DEFINES

//*** <<< Use Configuration Wizard in Context Menu >>> ***

// <e> Software blend implementation.
// <i> Utilizza un'implementazione software per il blending di due aree.
// <i> Questa opzione è utile solo per debuggare.
#define L_ENABLE_SW_BLEND 0
// </e>

// <e> Hardware acceleration with Dma2d
// <i> Abilita l'utilizzo del dma2d per accelerare le operazioni copiatura o
// <i> fill di aree di memoria.
#define L_ENABLE_HW_ACCELERATION 1
//  <o> Min pixel count for DMA2D blend [pxl] <0-3840>
//  <i> Minima dimensione dell'area da blendare per cui sia conveniente sfruttare
//  <i> l'accelerazione del DMA2D. Altrimenti conviene fare via software.
#define L_MIN_PXL_PER_BLENDAREA (0)
//  <o> Min pixel count for DMA2D copy [pxl] <0-3840>
//  <i> Minima dimensione dell'area da copiare per cui sia conveniente sfruttare
//  <i> l'accelerazione del DMA2D. Altrimenti conviene fare via software.
#define L_MIN_PXL_PER_COPYAREA (1000)
//  <o> Min pixel count for DMA2D fill [pxl] <0-3840>
//  <i> Minima dimensione dell'area da riempire per cui sia conveniente sfruttare
//  <i> l'accelerazione del DMA2D. Altrimenti conviene fare via software.
#define L_MIN_PXL_PER_FILLAREA (1000)
// </e>

//*** <<< end of configuration section >>>    ***

/*
this define is only there for debug purposes.
you should never disable this if you are not fully aware what you are doing.
*/
#define L_DO_CACHE_MAINTENANCE     1

//=========================================================== PRIVATE PROTOTYPES
static int32_t BlendArea (void *dstPtr, uint32_t dstOff, gldcolor_PxlFormat_t dstFormat,
                          void *srcPtr, uint32_t srcOff, gldcolor_PxlFormat_t srcFormat,
                          uint32_t width, uint32_t height);
static int32_t CopyArea (void *dstPtr, uint32_t dstOff,
                         void *srcPtr, uint32_t srcOff,
                         uint32_t width, uint32_t height);
static int32_t FillArea (void *dstPtr, uint32_t width, uint32_t height,
                         uint32_t off, void *srcMem, uint8_t srcMemSize);

//============================================================= STATIC VARIABLES
static bkehalboost_Init_t *Init;

//============================================================= GLOBAL VARIABLES
bkehalboost_Init_t bkehalboost_Init;
gldhalboost_t bkehalboost_Boost;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t bkehalboost_InitModule (void)
{
    int32_t ret = _STATUS_OK_;
    
    Init = &bkehalboost_Init;
    bkehalboost_Boost.BlendArea = BlendArea;
    bkehalboost_Boost.CopyArea = CopyArea;
    bkehalboost_Boost.FillArea = FillArea;
    
    return ret;
}

//============================================================ PRIVATE FUNCTIONS

#if (L_ENABLE_SW_BLEND == 0)
/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalBoost.h> per la descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t BlendArea (void *dstPtr, uint32_t dstOff, gldcolor_PxlFormat_t dstFormat,
                          void *srcPtr, uint32_t srcOff, gldcolor_PxlFormat_t srcFormat,
                          uint32_t width, uint32_t height)
{
    if ((width * height > L_MIN_PXL_PER_BLENDAREA) &&
        (dstFormat == GLDCOLOR_NATIVE) &&
        (srcFormat == GLDCOLOR_BGRA8888))
    {
#if (L_ENABLE_HW_ACCELERATION == 1)
        bool ok = true;
        
#if (L_DO_CACHE_MAINTENANCE == 1)
        if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // clean destination
            SCB_CleanDCache_by_Addr (dstPtr, (width + dstOff) * height * 2);
        if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // clean source
            SCB_CleanDCache_by_Addr (srcPtr, (width + srcOff) * height * 4);
#endif
        
        if (ok)
        {
            Init->Dma2d->Instance = DMA2D;
            Init->Dma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->Init.BytesSwap = DMA2D_BYTES_REGULAR;
            Init->Dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
            Init->Dma2d->Init.LineOffsetMode = DMA2D_LOM_PIXELS;
            Init->Dma2d->Init.Mode = DMA2D_M2M_BLEND;
            Init->Dma2d->Init.OutputOffset = dstOff;
            Init->Dma2d->Init.RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_Init (Init->Dma2d) != HAL_OK)
                ok = false;
        }
        
        if (ok)
        {
            /* configuro i parametri del layer sorgente (background layer, index = 0) */
            Init->Dma2d->LayerCfg[0].InputOffset = dstOff;
            Init->Dma2d->LayerCfg[0].InputColorMode = DMA2D_INPUT_RGB565;
            Init->Dma2d->LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
            Init->Dma2d->LayerCfg[0].InputAlpha = 0x00;
            Init->Dma2d->LayerCfg[0].ChromaSubSampling = DMA2D_NO_CSS;
            Init->Dma2d->LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_ConfigLayer (Init->Dma2d, 0) != HAL_OK)
                ok = false;
        }

        if (ok)
        {
            /* configuro i parametri del layer sorgente (foreground layer, index = 1) */
            Init->Dma2d->LayerCfg[1].InputOffset = srcOff;
            Init->Dma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
            Init->Dma2d->LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
            Init->Dma2d->LayerCfg[1].InputAlpha = 0x00;
            Init->Dma2d->LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
            Init->Dma2d->LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_ConfigLayer (Init->Dma2d, 1) != HAL_OK)
                ok = false;
        }

        if (ok &&
            HAL_DMA2D_BlendingStart_IT (Init->Dma2d, (uint32_t)srcPtr, (uint32_t)dstPtr, (uint32_t)dstPtr, width, height) != HAL_OK)
            ok = false;
        
        if (ok)
        {   // wait dma2d end
            /* per una gestione più raffinata potresti sospendere il thread della
            grafica durante l'utilizzo del dma, e risvegliarlo quando ha terminato.
            per ora tempo ne abbiamo, quindi aspettiamo che finisca.
            */
            HAL_DMA2D_StateTypeDef state;
            do
            {
                state = HAL_DMA2D_GetState (Init->Dma2d);
            } while (state != HAL_DMA2D_STATE_READY &&
                     state != HAL_DMA2D_STATE_ERROR);
            if (state == HAL_DMA2D_STATE_READY)
            {
#if (L_DO_CACHE_MAINTENANCE == 1)
                if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // invalidate destination
                    SCB_InvalidateDCache_by_Addr (dstPtr, (width + dstOff) * height * 2);
#endif
                return 0;
            } else {
                GLDCFG_ASSERT (0);
            }
        }
#endif
    }
    return -1;
}

#else

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalBoost.h> per la descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t BlendArea (void *dstPtr, uint32_t dstOff, gldcolor_PxlFormat_t dstFormat,
                          void *srcPtr, uint32_t srcOff, gldcolor_PxlFormat_t srcFormat,
                          uint32_t width, uint32_t height)
{
    if ((width * height > L_MIN_PXL_PER_BLENDAREA) &&
        (dstFormat == GLDCOLOR_NATIVE) &&
        (srcFormat == GLDCOLOR_BGRA8888))
    {
        uint16_t *outPxlPtr;
        uint8_t *pxlPnt;
        
        pxlPnt = srcPtr;
        outPxlPtr = dstPtr;
        
        for (int_fast32_t h = height; h > 0; h--)
        {
            for (int_fast32_t w = width; w > 0; w--)
            {
                gldcolor_t fClr; // foreground color
                gldcolor_t opa; // opacity
                
                fClr = *(uint32_t *)pxlPnt & 0xffffff;
                opa = (*(uint32_t *)pxlPnt >> 24) & 0xff;
                
                if (opa != GLDCOLOR_OPA_100)
                {
                    gldcolor_t bClr; // background color
                    
                    bClr = GLDCOLOR_OR_R (((*outPxlPtr) >> 8) & 0xf8);
                    bClr |= GLDCOLOR_OR_G (((*outPxlPtr) >> 3) & 0xfC);
                    bClr |= GLDCOLOR_OR_B (((*outPxlPtr) << 3) & 0xf8);
                    
                    fClr = gldcolor_Mix (fClr, bClr, opa);
                }
                
                uint16_t clr565;
                clr565  = ((GLDCOLOR_GET_R (fClr) & 0xf8) << 8);
                clr565 |= ((GLDCOLOR_GET_G (fClr) & 0xfC) << 3);
                clr565 |= ((GLDCOLOR_GET_B (fClr) & 0xf8) >> 3);
                *((uint16_t *)(outPxlPtr)) = clr565;
                
                pxlPnt += 4;
                outPxlPtr += 1;
            }
            pxlPnt += srcOff * 4;
            outPxlPtr += dstOff;
        }
        return 0;
    }
    return -1;
}
#endif

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalBoost.h> per la descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t CopyArea (void *dstPtr, uint32_t dstOff,
                         void *srcPtr, uint32_t srcOff,
                         uint32_t width, uint32_t height)
{
    if ((width * height > L_MIN_PXL_PER_COPYAREA) &&
        (width % 2) == 0 &&
        (dstOff % 2) == 0 &&
        (srcOff % 2) == 0)
    {
#if (L_ENABLE_HW_ACCELERATION == 1)
        bool ok = true;

#if (L_DO_CACHE_MAINTENANCE == 1)
        if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // clean destination
            SCB_CleanDCache_by_Addr (dstPtr, (width + dstOff) * height);
        if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // clean source
            SCB_CleanDCache_by_Addr (srcPtr, (width + srcOff) * height);
#endif
        
        if (ok)
        {
            Init->Dma2d->Instance = DMA2D;
            Init->Dma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->Init.BytesSwap = DMA2D_BYTES_REGULAR;
            Init->Dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
            Init->Dma2d->Init.LineOffsetMode = DMA2D_LOM_PIXELS;
            Init->Dma2d->Init.Mode = DMA2D_M2M;
            Init->Dma2d->Init.OutputOffset = dstOff >> 1;
            Init->Dma2d->Init.RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_Init (Init->Dma2d) != HAL_OK)
                ok = false;
        }
        
        if (ok)
        {
            /* configuro i parametri del layer sorgente (foreground layer, index = 1) */
            Init->Dma2d->LayerCfg[1].InputOffset = srcOff >> 1;
            Init->Dma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
            Init->Dma2d->LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
            Init->Dma2d->LayerCfg[1].InputAlpha = 0x00;
            Init->Dma2d->LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
            Init->Dma2d->LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_ConfigLayer (Init->Dma2d, 1) != HAL_OK)
                ok = false;
        }
        
        if (ok &&
            HAL_DMA2D_Start_IT (Init->Dma2d, (uint32_t)srcPtr, (uint32_t)dstPtr, width >> 1, height) != HAL_OK)
            ok = false;
        
        if (ok)
        {   // wait dma2d end
            HAL_DMA2D_StateTypeDef state;
            do
            {
                state = HAL_DMA2D_GetState (Init->Dma2d);
            } while (state != HAL_DMA2D_STATE_READY &&
                     state != HAL_DMA2D_STATE_ERROR);
            if (state == HAL_DMA2D_STATE_READY)
            {
#if (L_DO_CACHE_MAINTENANCE == 1)
                if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // invalidate destination
                    SCB_InvalidateDCache_by_Addr (dstPtr, (width + dstOff) * height);
#endif
                return 0;
            } else {
                GLDCFG_ASSERT (0);
            }
        }
#endif
    }
    return -1;
}

/*______________________________________________________________________________ 
 Desc:  Vedi <GldHalBoost.h> per la descrizione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int32_t FillArea (void *dstPtr, uint32_t width, uint32_t height,
                         uint32_t off, void *srcMem, uint8_t srcMemSize)
{
    if ((width * height > L_MIN_PXL_PER_FILLAREA) &&
        (srcMemSize == 2) &&
        (width * height) > 100)
    {
#if (L_ENABLE_HW_ACCELERATION == 1)
        bool ok = true;
        
#if (L_DO_CACHE_MAINTENANCE == 1)
        if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // clean destination
            SCB_CleanDCache_by_Addr (dstPtr, (width + off) * height * srcMemSize);
#endif
        
        if (ok)
        {
            Init->Dma2d->Instance = DMA2D;
            Init->Dma2d->Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
            Init->Dma2d->Init.BytesSwap = DMA2D_BYTES_REGULAR;
            Init->Dma2d->Init.ColorMode = DMA2D_OUTPUT_RGB565;
            Init->Dma2d->Init.LineOffsetMode = DMA2D_LOM_PIXELS;
            Init->Dma2d->Init.Mode = DMA2D_R2M;
            Init->Dma2d->Init.OutputOffset = off;
            Init->Dma2d->Init.RedBlueSwap = DMA2D_RB_REGULAR;
            if (HAL_DMA2D_Init (Init->Dma2d) != HAL_OK)
                ok = false;
        }

        /* Il colore passato a 'pdata' deve avere il formato ARGB8888 */
        if (ok &&
            HAL_DMA2D_Start_IT (Init->Dma2d, *(uint16_t *)srcMem, (uint32_t)dstPtr, width, height) != HAL_OK)
            ok = false;
            
        if (ok)
        {   // wait dma2d end
            HAL_DMA2D_StateTypeDef state;
            do
            {
                state = HAL_DMA2D_GetState (Init->Dma2d);
            } while (state != HAL_DMA2D_STATE_READY &&
                     state != HAL_DMA2D_STATE_ERROR);
            if (state == HAL_DMA2D_STATE_READY)
            {
#if (L_DO_CACHE_MAINTENANCE == 1)
                if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U) // invalidate destination
                    SCB_InvalidateDCache_by_Addr (dstPtr, (width + off) * height * srcMemSize);
#endif
                return 0;
            } else {
                GLDCFG_ASSERT (0);
            }
        }
#endif
    }
    return -1;
}
