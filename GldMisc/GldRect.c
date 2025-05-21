//===================================================================== INCLUDES
#include "GldMisc/GldRect.h"

#include <math.h>
#include <string.h>
#include "GldCore/GldBoost.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Ottieni l'indirizzo di memoria di un particolare pixel all'interno di un
        rettangolo di memoria, tenendo in considerazione l'orientamento.
 Arg: - <rect>[in] rettangolo di memoria, completamente caratterizzato, cio�
            tutti i suoi campi devono essere correttamente settati.
        <x>[in] coordinata x del pixel (coordinate assolute).
        <y>[in] coordinata y del pixel (coordinate assolute).
 Ret: - Indirizzo di memoria del pixel.
______________________________________________________________________________*/
void *gldrect_GetFrameMemoryPxl (gldrect_t *rect, gldcoord_t x, gldcoord_t y)
{
    void *ptrPxl = NULL;
    
    x -= rect->Area.X1;
    y -= rect->Area.Y1;
    
    switch (rect->Ori)
    {
        case GLDCOORD_ORI_0:
            ptrPxl = GLDRECT_GET_PXL_ORI_0 (rect, x, y);
            break;
        case GLDCOORD_ORI_90:
            ptrPxl = GLDRECT_GET_PXL_ORI_90 (rect, x, y);
            break;
        case GLDCOORD_ORI_180:
            ptrPxl = GLDRECT_GET_PXL_ORI_180 (rect, x, y);
            break;
        case GLDCOORD_ORI_270:
            ptrPxl = GLDRECT_GET_PXL_ORI_270 (rect, x, y);
            break;
    }
    return ptrPxl;
}

/*______________________________________________________________________________
 Desc:  Ottieni le caratteristiche di un sottorettangolo di memoria interno ad
        un'altro rettangolo di memoira.
 Arg: - <inRect>[in] rettangolo di cui vuoi ottenere il sottorettangolo.
          tutti i suoi campi devono essere correttamente configurati.
        <outRect>[in][out] rettangolo di memoria risultante.
          Tu imposti in questa variabile la sottoarea che vuoi ottenere.
          Quindi imposti outRect->Area, senza modificare gli altri campi.
          L'area che imposti deve essere completamente contenuta nell'area di
          inRect.
          La fuzione completa outRect impotando tutti gli altri suoi campi.
          Questi sono i campi di outRect che la funzione modifica:
         - [MemStart] Lowest pixel memory address.
         - [Bpp] Same as inRect.
         - [Ori] Same as inRect.
         - [Width] Numero di pixel contigui in memoria, per ciascuna linea.
            Questo valore � in generale diverso dalla width dell'area di outRect
            a causa dell'orientamento.
         - [Height] Numero totale di linee.
            Questo valore � in generale diverso dalla height dell'area di outRect
            a causa dell'orientamento.
         - [Offset] Offset in pixel fra una linea e la successiva.
 Ret: - None.
______________________________________________________________________________*/
void gldrect_GetFrameMemoryRect (gldrect_t *inRect, gldrect_t *outRect)
{
    gldcoord_Area_t common;
    gldcoord_t x1, y1, x_size, y_size;
    
    // getArea deve essere completamente contenuta in fbArea
    gldcoord_AreaIntersection (&common, &inRect->Area, &outRect->Area);
    GLDCFG_ASSERT (gldcoord_AreaCmp (&common, &outRect->Area) == 0);

    x1 = outRect->Area.X1;
    y1 = outRect->Area.Y1;
    x_size = gldcoord_AreaWidth (&outRect->Area);
    y_size = gldcoord_AreaHeight (&outRect->Area);
    
    outRect->Bpp = inRect->Bpp;
    outRect->Ori = inRect->Ori;
    switch (inRect->Ori)
    {
        case GLDCOORD_ORI_0:
        case GLDCOORD_ORI_180:
            outRect->Width = x_size;
            outRect->Height = y_size;
            outRect->Offset = gldcoord_AreaWidth (&inRect->Area) + inRect->Offset - x_size;
            break;
        
        case GLDCOORD_ORI_90:
        case GLDCOORD_ORI_270:
            outRect->Width = y_size;
            outRect->Height = x_size;
            outRect->Offset = gldcoord_AreaHeight (&inRect->Area) + inRect->Offset - y_size;
            break;
    }
    
    switch (inRect->Ori)
    {
        case GLDCOORD_ORI_0:
            outRect->MemStart = gldrect_GetFrameMemoryPxl (inRect, x1, y1);
            break;
        case GLDCOORD_ORI_90:
            outRect->MemStart = gldrect_GetFrameMemoryPxl (inRect, x1 + (x_size - 1), y1);
            break;
        case GLDCOORD_ORI_180:
            outRect->MemStart = gldrect_GetFrameMemoryPxl (inRect, x1 + (x_size - 1), y1 + (y_size - 1));
            break;
        case GLDCOORD_ORI_270:
            outRect->MemStart = gldrect_GetFrameMemoryPxl (inRect, x1, y1 + (y_size - 1));
            break;
    }
}

/*______________________________________________________________________________
 Desc:  Source and destination area must have same pixel format.
        Source and destination must fall in one of the following conditions
          * src[orientation] = dst[orientation]    // fast
          * src[orientation] = 0                   // medium
          * other combinations                     // slow
        Copia il rettangolo di memoira sorgente nel rettangolo di memoria di
        destinazione.
 Arg: - <prm>[in] struttura di tutti i parametri necessari.
            Vedi 'gldrect_CopyMemArea_t' per una descrizione dettagliata.
 Ret: - None.
______________________________________________________________________________*/
void gldrect_CopyMemArea (gldrect_CopyMemArea_t *prm)
{
    gldrect_t srcRect;
    gldrect_t dstRect;
    
    GLDCFG_ASSERT (prm->SrcCanvas.Bpp == prm->DstCanvas.Bpp);
    
    /* calcolo le coordinate dell'area sorgente. le coordinate si riferiscono
    alla superficie sorgente */
    srcRect.Area.X1 = prm->AreaSrcPoint.X;
    srcRect.Area.X2 = prm->AreaSrcPoint.X + prm->AreaXSize;
    srcRect.Area.Y1 = prm->AreaSrcPoint.Y;
    srcRect.Area.Y2 = prm->AreaSrcPoint.Y + prm->AreaYSize;
    
    /* calcolo le coordinate dell'area di destinazione. le coordinate si
    riferiscono alla superficie di destinazione */
    dstRect.Area.X1 = prm->AreaDstPoint.X;
    dstRect.Area.X2 = prm->AreaDstPoint.X + prm->AreaXSize;
    dstRect.Area.Y1 = prm->AreaDstPoint.Y;
    dstRect.Area.Y2 = prm->AreaDstPoint.Y + prm->AreaYSize;

    /* ottengo i parametri caratteristici delle sottosuperfici sorgente e
    destinazione. Le sottosuperfici che corrispondono all'area sorgente e
    a quella di destinazione */
    gldrect_GetFrameMemoryRect (&prm->SrcCanvas, &srcRect);
    gldrect_GetFrameMemoryRect (&prm->DstCanvas, &dstRect);
    
    char *srcPtr, *dstPtr;
    int_fast8_t bpp;
    
    bpp = srcRect.Bpp;
    srcPtr = srcRect.MemStart;
    dstPtr = dstRect.MemStart;
    if (srcRect.Ori == dstRect.Ori)
    {
        uint16_t width, height;
        uint16_t srcOffset, dstOffset;
        
        /*
        srcRect.Width and dstRect.Width should be the same
        srcRect.Height and dstRect.Height should be the same
        So i use indifferently one of them.
        */
        width = srcRect.Width;
        height = srcRect.Height;
        bpp = srcRect.Bpp;
        srcOffset = srcRect.Offset;
        dstOffset = dstRect.Offset;
        
        /* tento accelerazione hardware se disponibile */
        if (gldboost_Boost &&
            gldboost_Boost->CopyArea (dstPtr, dstOffset * bpp,
                                      srcPtr, srcOffset * bpp,
                                      width * bpp, height) == 0)
        {
            ; /* copia effettuata con accelerazione hardware */
        }
        else
        {
            /* copio rettangolo di memoria sorgente nel rettangolo di memoria di
            destinazione. */
            while (height)
            {
                memcpy (dstPtr, srcPtr, width * bpp);
                dstPtr += (width + dstOffset) * bpp;
                srcPtr += (width + srcOffset) * bpp;
                height--;
            }
        }
    }
    else if (srcRect.Ori == GLDCOORD_ORI_0)
    {
        switch (dstRect.Ori)
        {
            case GLDCOORD_ORI_90:
            {
                for (int_fast32_t y = 0; y < srcRect.Height; y++)
                {
                    dstPtr = GLDRECT_GET_PXL_ORI_90 (&dstRect, 0, y);
                    for (int_fast32_t x = srcRect.Width; x; x--)
                    {
                        for (int_fast8_t b = 0; b < bpp; b++)
                            dstPtr[b] = srcPtr[b];
                        srcPtr += bpp;
                        dstPtr = GLDRECT_NEXT_PXL_ORI_90 (&dstRect, dstPtr);
                    }
                    srcPtr += srcRect.Offset * bpp;
                }
                break;
            }
            
            case GLDCOORD_ORI_180:
            {
                for (int_fast32_t y = 0; y < srcRect.Height; y++)
                {
                    dstPtr = GLDRECT_GET_PXL_ORI_180 (&dstRect, 0, y);
                    for (int_fast32_t x = srcRect.Width; x; x--)
                    {
                        for (int_fast8_t b = 0; b < bpp; b++)
                            dstPtr[b] = srcPtr[b];
                        srcPtr += bpp;
                        dstPtr = GLDRECT_NEXT_PXL_ORI_180 (&dstRect, dstPtr);
                    }
                    srcPtr += srcRect.Offset * bpp;
                }
                break;
            }
            
            case GLDCOORD_ORI_270:
            {
                for (int_fast32_t y = 0; y < srcRect.Height; y++)
                {
                    dstPtr = GLDRECT_GET_PXL_ORI_270 (&dstRect, 0, y);
                    for (int_fast32_t x = srcRect.Width; x; x--)
                    {
                        for (int_fast8_t b = 0; b < bpp; b++)
                            dstPtr[b] = srcPtr[b];
                        srcPtr += bpp;
                        dstPtr = GLDRECT_NEXT_PXL_ORI_270 (&dstRect, dstPtr);
                    }
                    srcPtr += srcRect.Offset * bpp;
                }
                break;
            }
            
            default:
                break;
        }
    }
    else
    {   /* codice non ottimizzato, non dovresti mai cadere qua */
        for (int y = 0; y < srcRect.Height; y++)
        {
            for (int x = 0; x < srcRect.Width; x++)
            {
                srcPtr = gldrect_GetFrameMemoryPxl (&srcRect, srcRect.Area.X1 + x, srcRect.Area.Y1 + y);
                dstPtr = gldrect_GetFrameMemoryPxl (&dstRect, dstRect.Area.X1 + x, dstRect.Area.Y1 + y);
                for (int_fast8_t b = 0; b < bpp; b++)
                    dstPtr[b] = srcPtr[b];
            }
        }
    }
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldrect_SetWidthHeightAsFull (gldrect_t *rect)
{
    /* the area is a full contiguous block of pixel memory.
    so there is no offset. */
    rect->Offset = 0;
    switch (rect->Ori)
    {
        case GLDCOORD_ORI_0:
        case GLDCOORD_ORI_180:
            rect->Width = gldcoord_AreaWidth (&rect->Area);
            rect->Height = gldcoord_AreaHeight (&rect->Area);
            break;
        
        case GLDCOORD_ORI_90:
        case GLDCOORD_ORI_270:
            rect->Width = gldcoord_AreaHeight (&rect->Area);
            rect->Height = gldcoord_AreaWidth (&rect->Area);
            break;
    }
}

//============================================================ PRIVATE FUNCTIONS
