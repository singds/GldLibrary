//===================================================================== INCLUDES
#include "GldDrawStd.h"

#include "GldCfg.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS
/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void glddrawstd_Bmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, glddraw_BmpData_t *d)
{
    switch (d->BmpIh->BiBitCount)
    {
    case 24:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;

            /* move to the current line */
            lp = d->MapPtr + ((y_pos - y) * d->LineOff + d->LeftByteOff);

            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                uint32_t clr;

                /* ottieni il colore non indicizzato */
                clr = GLDIMG_BMP_GET_COLOR (lp);

                if (clr != GLDCOLOR_NONE)
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                lp += 3;
            }
        }
        break;

    case 8:
        for (int y_pos = d->AreaView->Y1; y_pos < d->AreaView->Y2; y_pos++)
        {
            const char *lp;

            /* move to the current line */
            lp = d->MapPtr + ((y_pos - y) * d->LineOff + d->LeftByteOff);

            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                const char *clr_p;
                uint32_t clr;

                /* get color from palette */
                clr_p = &d->Paletta[*lp << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                lp++;
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
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                bit_off -= d->BmpIh->BiBitCount;
                if (bit_off < 0)
                {
                    bit_off = 8 - d->BmpIh->BiBitCount;
                    lp++;
                }
            }
        }
        break;
    }
    }
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void glddrawstd_BmpFs (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, glddraw_BmpFsData_t *d)
{
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
                break; // error
            }
            lp = map_line + d->LeftByteOff;
            
            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                uint32_t clr;

                /* ottieni il colore non indicizzato */
                clr = GLDIMG_BMP_GET_COLOR (lp);

                if (clr != GLDCOLOR_NONE)
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                lp += 3;
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

            for (int x_pos = d->AreaView->X1; x_pos < d->AreaView->X2; x_pos++)
            {
                const char *clr_p;
                uint32_t clr;

                /* get color from palette */
                clr_p = &d->Paletta[*lp << 2];
                clr = GLDIMG_BMP_GET_COLOR (clr_p);

                if (clr != GLDCOLOR_NONE)
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                lp++;
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
                    disp->SetPxl (x_pos, y_pos, clr);

                /* move to next pixel */
                bit_off -= d->BmpIh->BiBitCount;
                if (bit_off < 0)
                {
                    bit_off = 8 - d->BmpIh->BiBitCount;
                    lp++;
                }
            }
        }
        break;
    }
    }
    GLDCFG_FREE (map_line);
}

/*______________________________________________________________________________
 Desc:  Versione non ottimizzata del ciclo finale della funzione <glddraw_PxlMap>
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void glddrawstd_PxlMap (gldhaldisp_t *disp, gldrect_t *mapRect, gldcolor_PxlFormat_t format)
{
    gldcoord_t xStart, xEnd;
    gldcoord_t yStart, yEnd;
    
    if (format == GLDCOLOR_RGBA8888)
    {
        xStart = mapRect->Area.X1;
        xEnd = xStart + gldcoord_AreaWidth (&mapRect->Area);
        yStart = mapRect->Area.Y1;
        yEnd = yStart + gldcoord_AreaHeight (&mapRect->Area);
        
        for (int y = yStart; y < yEnd; y++)
        {
            for (int x = xStart; x < xEnd; x++)
            {
                gldcolor_t fClr;
                gldcolor_Opa_t opa;
                const char *pxlPnt;
                
                pxlPnt = gldrect_GetFrameMemoryPxl (mapRect, x, y);
                opa = pxlPnt[3];
                if (opa != GLDCOLOR_OPA_0)
                {
                    fClr = (pxlPnt[0] << 16) | (pxlPnt[1] << 8) | (pxlPnt[2] << 0);
                
                    if (opa != GLDCOLOR_OPA_100)
                    {
                        fClr = gldcolor_Mix (fClr, disp->GetPxl (x, y), opa);
                    }
                    disp->SetPxl (x, y, fClr);
                }
                pxlPnt += 4;
            }
        }
    }
}

//============================================================ PRIVATE FUNCTIONS
