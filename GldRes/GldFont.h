#ifndef GLDFONT_H_INCLUDED
#define GLDFONT_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldCfg.h"
#include "GldTxt.h"

//====================================================================== DEFINES

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
/* old font support */
typedef struct
{
    uint8_t Bpp;        /* not really used */
    uint16_t FirstChar; /* first glyph table character */
    uint16_t LastChar;  /* last glyph table character */
    uint16_t Height;    /* font character height [pxl] */
    const char *GlyphTable;
    const char *GlyphMap;
    const char *Array; /* array c di questo font */
} gldfont_t;
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
/* new font support */
/* Il novo formato per i font supporta l'antialiasing.
   Perch� l'antialiasing funzioni deve essere attivo il prerendering 'GLDCFG_MEMFB_ENABLED'.
   Oppure in alternativa con rendering diretto su display il driver del display
   deve implementare la funzione 'GetPxl'. */
typedef struct
{
    uint8_t EmptyTopHeight; /* Righe dal top NON utilizzate */
    uint8_t DataHeight;     /* Espresso in righe */
    uint32_t OffsetData;    /* Offset di puntamento ai data del carattere */
    uint8_t Width;          /* Larghezza in pixel completa */
} gldfont_NFontInfo_t;

typedef struct gldfont_NFont_s
{
    uint16_t FirstChar; /* primo carattere di questo gruppo */
    uint16_t LastChar;  /* ultimo carattere di questo gruppo */
    uint8_t Height;     /* Altezza dei caratteri [pxl] */
    const gldfont_NFontInfo_t *PtrNFontInfo;
    const uint8_t *PtrFontData;
    uint8_t BitColor; /* Profondit� colore 1,2,4 */

    struct gldfont_NFont_s *NextFontGroup; /* prossimo gruppo di lettere di questo font */

} gldfont_NFont_t;

typedef gldfont_NFont_t gldfont_t;
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
#include "fontBuilderForC.h"

typedef FONTBUILDERFORC_TYPE_FONT gldfont_t;
#endif

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
gldfont_t *gldfont_Open (const char *font_array);
const char *gldfont_GetLetterDrawInfo (const gldfont_t *font, gldtxt_Letter_t ltr);
#endif
#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
gldfont_t *gldfont_Open (const gldfont_t *font_struct);
const gldfont_NFontInfo_t *gldfont_GetLetterDrawInfo (const gldfont_NFont_t *font, gldtxt_Letter_t ltr, const uint8_t **map_pp);
#endif
#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
gldfont_t *gldfont_Open (const FONTBUILDERFORC_TYPE_FONT *font_struct);
const FONTBUILDERFORC_TYPE_CHARACTER *gldfont_GetCharacter (const gldfont_t *font, gldtxt_Letter_t ltr);
int16_t gldfont_GetKerning (const gldfont_t *font, gldtxt_Letter_t left_ltr, gldtxt_Letter_t right_ltr);
#endif
int16_t gldfont_GetHeight (const gldfont_t *font);
int16_t gldfont_GetLetterWidth (const gldfont_t *font, gldtxt_Letter_t ltr);
int16_t gldfont_GetTxtWidth (const gldfont_t *font, const char *txt, uint32_t size);

#endif /* GLDFONT_H_INCLUDED */
