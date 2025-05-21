//===================================================================== INCLUDES
#include "GldDrawOpt.h"

#include "GldCfg.h"
#include "GldCore/GldBoost.h"

//====================================================================== DEFINES
#if (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0)
#define L_NEXT_PXL(mem, pxlPtr)           ((void *)((uint8_t *)pxlPtr + mem->Bpp))
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__90)
#define L_NEXT_PXL(mem, pxlPtr)           ((void *)((uint8_t *)pxlPtr - (mem->Ysize * mem->Bpp)))
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__180)
#define L_NEXT_PXL(mem, pxlPtr)           ((void *)((uint8_t *)pxlPtr - mem->Bpp))
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__270)
#define L_NEXT_PXL(mem, pxlPtr)           ((void *)((uint8_t *)pxlPtr + (mem->Ysize * mem->Bpp)))
#endif

//=========================================================== PRIVATE PROTOTYPES
void *GetPxlPnt (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y);

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS
/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t glddrawopt_Bmp (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y, glddraw_BmpData_t *d)
{
    uint8_t* outPxlPtr;

    switch (d->BmpIh->BiBitCount)
    {
    case 24:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;

            /* move to the current line */
            lp = d->MapPtr + ((y_pos - y) * d->LineOff + d->LeftByteOff);
            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                uint32_t clr;

                /* ottieni il colore non indicizzato */
                clr = GLDIMG_BMP_GET_COLOR (lp);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                lp += 3;
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;

    case 8:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;

            /* move to the current line */
            lp = d->MapPtr + ((y_pos - y) * d->LineOff + d->LeftByteOff);
            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                const char *clr_p;
                uint32_t clr;

                /* get color from palette */
                clr_p = &d->Paletta[*lp << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                lp++;
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;

    case 1:
    case 2:
    case 4:
    {
        /* get the sub-byte pixel mask */
        int pxl_mask = 0;
        for (int k = 0; k < d->BmpIh->BiBitCount; k++)
            pxl_mask = (pxl_mask << 1) | 1;

        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        { /* left most pixel in the byte are the most significant bits */
            int bit_off = (8 - d->BmpIh->BiBitCount) - d->LeftBitOff;
            const char *lp;

            /* move to the current line */
            lp = d->MapPtr + ((y_pos - y) * d->LineOff + d->LeftByteOff);
            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                int index;
                const char *clr_p;
                uint32_t clr;

                /* get the pixel palette index */
                index = (*lp & (pxl_mask << bit_off)) >> bit_off;

                /* get color from palette */
                clr_p = &d->Paletta[index << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                bit_off -= d->BmpIh->BiBitCount;
                if (bit_off < 0)
                {
                    bit_off = 8 - d->BmpIh->BiBitCount;
                    lp++;
                }
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;
    }
    }
    return 0;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t glddrawopt_BmpFs (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y, glddraw_BmpFsData_t *d)
{
    uint8_t* outPxlPtr;
    char *map_line;
    
    map_line = GLDCFG_MALLOC (d->LineSz); /* leggo qui una linea di pixel alla volta */
    GLDCFG_ASSERT (map_line);

    switch (d->BmpIh->BiBitCount)
    {
    case 24:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;
            
            /* move to the current line */
            fseek (d->File, d->MapOff + (y_pos - y) * d->LineOff, SEEK_SET);
            if (fread (map_line, 1, d->LineSz, d->File) != d->LineSz) {
                break;
            }
            lp = map_line + d->LeftByteOff;

            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                uint32_t clr;

                /* ottieni il colore non indicizzato */
                clr = GLDIMG_BMP_GET_COLOR (lp);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                lp += 3;
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;
    
    case 8:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;

            /* move to the current line */
            fseek (d->File, d->MapOff + (y_pos - y) * d->LineOff, SEEK_SET);
            if (fread (map_line, 1, d->LineSz, d->File) != d->LineSz) {
                break; // error
            }
            lp = map_line + d->LeftByteOff;

            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                const char *clr_p;
                uint32_t clr;

                /* get color from palette */
                clr_p = &d->Paletta[*lp << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                lp++;
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;

    case 1:
    case 2:
    case 4:
    {
        /* get the sub-byte pixel mask */
        int pxl_mask = 0;
        for (int k = 0; k < d->BmpIh->BiBitCount; k++)
            pxl_mask = (pxl_mask << 1) | 1;

        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        { /* left most pixel in the byte are the most significant bits */
            int bit_off = (8 - d->BmpIh->BiBitCount) - d->LeftBitOff;
            const char *lp;

            /* move to the current line */
            fseek (d->File, d->MapOff + (y_pos - y) * d->LineOff, SEEK_SET);
            if (fread (map_line, 1, d->LineSz, d->File) != d->LineSz) {
                break; // error
            }
            lp = map_line + d->LeftByteOff;

            outPxlPtr = GetPxlPnt (mem, d->AreaView->X1, y_pos);
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                int index;
                const char *clr_p;
                uint32_t clr;

                /* get the pixel palette index */
                index = (*lp & (pxl_mask << bit_off)) >> bit_off;

                /* get color from palette */
                clr_p = &d->Paletta[index << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                {   // scrivo direttamente il pixel
                    gldcfg_WritePxl (outPxlPtr, clr);
                }

                /* move to next pixel */
                bit_off -= d->BmpIh->BiBitCount;
                if (bit_off < 0)
                {
                    bit_off = 8 - d->BmpIh->BiBitCount;
                    lp++;
                }
                outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
            }
        }
        break;
    }
    }
    GLDCFG_FREE (map_line);
    return 0;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int32_t glddrawopt_PxlMap (glddrawopt_MemRect_t *mem, gldrect_t *mapRect, gldcolor_PxlFormat_t format)
{
    uint8_t *outPxlPtr;
    uint8_t *pxlPnt;
    
    if (mapRect->Ori == GLDCFG_MEMFB_ORI)
    {   /* uso accelerazione hardware se disponibile */
        gldrect_t fbRect;
        gldrect_t fbSubRect;
        
        fbRect.MemStart = mem->BuffPtr;
        fbRect.Ori = (gldcoord_Ori_e)GLDCFG_MEMFB_ORI;
        fbRect.Bpp = mem->Bpp;
        fbRect.Area.X1 = mem->X1;
        fbRect.Area.X2 = mem->X1 + mem->Xsize;
        fbRect.Area.Y1 = mem->Y1;
        fbRect.Area.Y2 = mem->Y1 + mem->Ysize;
        gldrect_SetWidthHeightAsFull (&fbRect);
        
        gldcoord_AreaCopy (&fbSubRect.Area, &mapRect->Area);
        gldrect_GetFrameMemoryRect (&fbRect, &fbSubRect);
        
        pxlPnt = mapRect->MemStart;
        outPxlPtr = fbSubRect.MemStart;
        
        if (gldboost_Boost &&
            gldboost_Boost->BlendArea (
                    outPxlPtr, fbSubRect.Offset, GLDCOLOR_NATIVE,
                    pxlPnt, mapRect->Offset, format,
                    mapRect->Width, mapRect->Height) == 0)
            return 0; // completato con successo
        
        /* hardware boost can't handle the request.
        fall back to software implementation */
    }
    
    if (mapRect->Ori == GLDCOORD_ORI_0)
    {
        if (format == GLDCOLOR_RGBA8888)
        {
            gldcoord_t xStart, xEnd;
            gldcoord_t yStart, yEnd;
            
            xStart = mapRect->Area.X1;
            xEnd = xStart + gldcoord_AreaWidth (&mapRect->Area);
            yStart = mapRect->Area.Y1;
            yEnd = yStart + gldcoord_AreaHeight (&mapRect->Area);
            
            pxlPnt = mapRect->MemStart;
            for (int y = yStart; y < yEnd; y++)
            {
                outPxlPtr = GetPxlPnt (mem, xStart, y);
                for (int x = xStart; x < xEnd; x++)
                {
                    gldcolor_t fClr; // foreground color
                    gldcolor_t opa; // opacity
                    
                    opa = pxlPnt[3];
                    if (opa != GLDCOLOR_OPA_0)
                    {
                        fClr = (pxlPnt[0] << 16) | (pxlPnt[1] << 8) | (pxlPnt[2] << 0);
                        
                        if (opa != GLDCOLOR_OPA_100)
                        {
                            gldcolor_t bClr; // background color (rgb888)
                            
                            bClr = gldcfg_ReadPxl (outPxlPtr);
                            fClr = gldcolor_Mix (fClr, bClr, opa);
                        }
                        gldcfg_WritePxl (outPxlPtr, fClr);
                    }
                    
                    pxlPnt += 4;
                    outPxlPtr = L_NEXT_PXL (mem, outPxlPtr);
                }
                pxlPnt += mapRect->Offset << 2;
            }
            return 0;
        }
    }
    return 1; // pixel format non supportato
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void *GetPxlPnt (glddrawopt_MemRect_t *mem, gldcoord_t x, gldcoord_t y)
{
    x -= mem->X1;
    y -= mem->Y1;
#if (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__0)
    return (void *)((uint32_t)mem->BuffPtr + (y * mem->Xsize + x) * mem->Bpp);
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__90)
    return (void *)((uint32_t)mem->BuffPtr + ((mem->Xsize - x - 1) * mem->Ysize + y) * mem->Bpp);
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__180)
    return (void *)((uint32_t)mem->BuffPtr + (((mem->Ysize - y - 1) * mem->Xsize) + mem->Xsize - x - 1) * mem->Bpp);
#elif (GLDCFG_MEMFB_ORI == GLDCFG_MEMFB_ORI__270)
    return (void *)((uint32_t)mem->BuffPtr + ((x * mem->Ysize) + mem->Ysize - y - 1) * mem->Bpp);
#endif
}
