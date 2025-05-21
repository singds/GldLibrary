//===================================================================== INCLUDES
#include "GldRes/GldImg.h"

#include "GldMisc/GldMisc.h"
#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Ottieni il "file header" del file bitmap.
 Arg: - <fh_p>[out] puntatore a struttura da compilare.
        <bmp_p> sorgente bmp.
 Ret: - None.
______________________________________________________________________________*/
void gldimg_BmpGetHeaderFile (gldimg_BmpHeaderFile_t *fh_p, const char *bmp_p)
{
    /* Move the pointer to the start of the data structure */
    bmp_p += GLDIMG_BMP_OFF_FILEH;

    /* Get the bitmap file header parameters */
    memcpy (fh_p->BfType, bmp_p, sizeof (fh_p->BfType));
    bmp_p += 2;
    bmp_p = gldmisc_Get32Le (&fh_p->BfSize, bmp_p);
    bmp_p = gldmisc_Get32Le (&fh_p->BfReserved, bmp_p);
    bmp_p = gldmisc_Get32Le (&fh_p->BfOffBits, bmp_p);
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l' "image header" del file bitmap.
 Arg: - <ih_p>[out] puntatore a struttura da compilare.
        <bmp_p> sorgente bmp.
 Ret: - None.
______________________________________________________________________________*/
void gldimg_BmpGetHeaderImage (gldimg_BmpHeaderImage_t *ih_p, const char *bmp_p)
{
    /* Move the pointer to the start of the data structure */
    bmp_p += GLDIMG_BMP_OFF_IMAGEH;

    /* Get the bitmap image header parameters */
    bmp_p = gldmisc_Get32Le (&ih_p->BiSize, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiWidth, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiHeight, bmp_p);
    bmp_p = gldmisc_Get16Le (&ih_p->BiPlanes, bmp_p);
    bmp_p = gldmisc_Get16Le (&ih_p->BiBitCount, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiCompression, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiSizeImage, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiXPelsPerMeter, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiYPelsPerMeter, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiClrUsed, bmp_p);
    bmp_p = gldmisc_Get32Le (&ih_p->BiClrImportant, bmp_p);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldimg_RawGetHeader (imgcvt_Header_t *h_p, const char *raw_p)
{
    raw_p = gldmisc_GetString (&h_p->magic, raw_p, 3);
    raw_p = gldmisc_GetString (&h_p->version, raw_p, 3);
    raw_p = gldmisc_Get8 (&h_p->orientation, raw_p);
    raw_p = gldmisc_Get8 (&h_p->color_format, raw_p);
    raw_p = gldmisc_Get32Be (&h_p->width, raw_p);
    raw_p = gldmisc_Get32Be (&h_p->height, raw_p);
    raw_p = gldmisc_Get32Be (&h_p->pxl_offset, raw_p);
}

//============================================================ PRIVATE FUNCTIONS
