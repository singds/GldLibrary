#ifndef GLDIMG_H_INCLUDED
#define GLDIMG_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "imgCvt.h"

//====================================================================== DEFINES

//_____________________________________________________________BITMAP IMAGE TYPE
#define GLDIMG_BMP_OFF_FILEH          (0)
#define GLDIMG_BMP_SIZE_FILEH         (14)
#define GLDIMG_BMP_OFF_IMAGEH         (14)
#define GLDIMG_BMP_SIZE_IMAGEH        (40)

#define GLDIMG_BMP_OFF_WIDTH          (18)
#define GLDIMG_BMP_OFF_HEIGHT         (22)

#define GLDIMG_BMP_GET_COLOR(clr_p)   ((clr_p)[0] | ((clr_p)[1] << 8) | ((clr_p)[2] << 16))
#define GLDIMG_BMP_PALETTE_OFF        (54)

typedef struct
{
    char BfType[2];
    /* la stringa ASCII "BM" */
    uint32_t BfSize;
    /* dimensione del file (non affidabile!) */
    uint32_t BfReserved;
    /* 0 */
    uint32_t BfOffBits;
    /* offset del primo byte della mappa dei pixel a partire dall'inizio del file */
} gldimg_BmpHeaderFile_t; /* 14 byte */

typedef struct
{
    uint32_t BiSize;
    /* dimensione in byte del blocco d'informazioni insieme alle strutture per
        il modello di colore aggiunte nelle versioni 4 e 5:
            versione 3 = 40 (hex 28)
            versione 4 = 108 (hex 6C)
            versione 5 = 124 (hex 7C) */
    int32_t BiWidth;
    /* larghezza dell'immagine in pixel */
    int32_t BiHeight;
    /* corrisponde in valore assoluto all'altezza dell'immagine in pixel:
        quando il valore è positivo
            l'immagine è bottom-up (la mappa dei pixel incomincia dalla riga di
            pixel più in basso e finisce con quella più in alto). Questa è la
            variante più comune.
        quando il valore è negativo
            l'immagine è top-down (la mappa dei pixel incomincia dalla riga di
            pixel più in alto e finisce con quella più in basso). */
    uint16_t BiPlanes;
    /* sempre 1 */
    uint16_t BiBitCount;
    /* profondità di colore dell'immagine in bit per pixel, dev'essere uno dei
        seguenti valori: 1, 4, 8, 16, 24 o 32. In caso di 1, 4 o 8 bit per pixel
        i colori sono indicizzati. I valori 16 e 32 sono poco comuni. Nella
        versione 5 del formato si può usare il valore 0 quando viene incapsulata
        un'immagine JPEG o PNG. */
    uint32_t BiCompression;
    /* uno dei seguenti valori:
        0 (BI_RGB)
            La mappa dei pixel non è compressa.
        1 (BI_RLE8)
            La mappa dei pixel è compressa con l'algoritmo RLE per 8 bit per pixel.
            Valido solo per biBitCount = 8 e biHeight > 0.
        2 (BI_RLE4)
            La mappa dei pixel è compressa con l'algoritmo RLE per 4 bit per pixel.
            Valido solo per biBitCount = 4 e biHeight > 0.
        3 (BI_BITFIELDS)
            La mappa dei pixel non è compressa ed è codificata secondo maschere di
            colore personalizzate. Valido solo per biBitCount = 16 o 32; poco comune.
            Nella versione 5 del formato sono ammessi inoltre i seguenti valori:
        4 (BI_JPEG)
            La bitmap incapsula un'immagine in formato JPEG.
        5 (BI_PNG)
            La bitmap incapsula un'immagine in formato PNG. */
    uint32_t BiSizeImage;
    /* Indica la dimensione in byte del buffer mappa dei pixel. Questo valore
        può essere lasciato a zero quando biCompression è impostato a BI_RGB. */
    uint32_t BiXPelsPerMeter;
    /* risoluzione orizzontale del dispositivo di output in pixel per metro;
        0 se la risoluzione non è specificata. */
    uint32_t BiYPelsPerMeter;
    /* risoluzione verticale del dispositivo di output in pixel per metro;
        0 se la risoluzione non è specificata. */
    uint32_t BiClrUsed;
    /* quando biBitCount = 1
            0
        quando biBitCount = 4 o 8
            numero di corrispondenze effettivamente utilizzate nella tavolozza dei
            colori; 0 indica il numero massimo (16 o 256).
        altrimenti
            numero di corrispondenze nella tavolozza dei colori (0 = nessuna tavolozza).
            Per profondità maggiori di 8 bit per pixel la tavolozza non è normalmente
            necessaria, ma quando c'è può essere usata dal sistema o da alcuni programmi
            per ottimizzare la rappresentazione dell'immagine. */
    uint32_t BiClrImportant;
    /* quando biBitCount = 1, 4 o 8
            numero di colori utilizzati nell'immagine; 0 indica tutti i colori della tavolozza.
        altrimenti
            se la tavolozza esiste e contiene tutti i colori utilizzati nell'immagine
                numero di colori
            altrimenti
                0 */
} gldimg_BmpHeaderImage_t; /* 40 byte */


//________________________________________________________________RAW IMAGE TYPE
#define GLDIMG_RAW_SIZE_HEADER        (64)

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void gldimg_BmpGetHeaderFile (gldimg_BmpHeaderFile_t *fh_p, const char *bmp_p);
void gldimg_BmpGetHeaderImage (gldimg_BmpHeaderImage_t *ih_p, const char *bmp_p);

void gldimg_RawGetHeader (imgcvt_Header_t *h_p, const char *raw_p);

#endif /* GLDIMG_H_INCLUDED */
