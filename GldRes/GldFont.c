//===================================================================== INCLUDES
#include "GldRes/GldFont.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
/*______________________________________________________________________________ 
 Desc:  Apre un font e ne restituisce l'handler.
 Arg: - <font_array>[in] Array che contiene il font.
 Ret: - Handler del font aperto.
______________________________________________________________________________*/
gldfont_t *gldfont_Open (const char *font_array)
{
    gldfont_t *font;

    if ((font = GLDCFG_CALLOC (1, sizeof (gldfont_t))) != NULL)
    {
        /* retrieve font info from the binary font */
        font->Bpp = 0; /* not used */
        font->FirstChar = font_array[2] | (font_array[3] << 8);
        font->LastChar = font_array[4] | (font_array[5] << 8);
        font->Height = font_array[6] | (font_array[7] << 8);

        font->GlyphTable = font_array + 8;
        font->GlyphMap = font_array;
    }

    return font;
}
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
/*______________________________________________________________________________ 
 Desc:  Apre un font e ne restituisce l'handler.
 Arg: - <font_struct>[in] Nuova struttura che definisce un font.
 Ret: - Handler del font aperto.
______________________________________________________________________________*/
gldfont_t *gldfont_Open (const gldfont_NFont_t *font_struct)
{ /* no operation required */
    return (gldfont_t *)font_struct;
}
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
/*______________________________________________________________________________ 
 Desc:  Apre un font e ne restituisce l'handler.
 Arg: - <font_struct>[in] Nuova struttura che definisce un font.
 Ret: - Handler del font aperto.
______________________________________________________________________________*/
gldfont_t *gldfont_Open (const FONTBUILDERFORC_TYPE_FONT *font_struct)
{ /* no operation required */
    return (gldfont_t *)font_struct;
}
#endif

/*______________________________________________________________________________ 
 Desc:  Ottieni la larghezza in pixel di uno specifico carattere.
 Arg: - <font>[in] Font del carattere.
        <ltr>[in] Carattere di cui vuoi la larghezza.
 Ret: - Larghezza del carattere [pxl].
______________________________________________________________________________*/
int16_t gldfont_GetLetterWidth (const gldfont_t *font, gldtxt_Letter_t ltr)
{
    int16_t width = 0;

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
    if (ltr >= font->FirstChar && ltr <= font->LastChar)
    { /* glyph available */
        uint16_t index = (ltr - font->FirstChar) << 2;
        width = font->GlyphTable[index];
    }
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
    while (font != NULL)
    {
        if (ltr >= font->FirstChar && ltr <= font->LastChar)
        { /* glyph available */
            uint16_t index = ltr - font->FirstChar;
            width = font->PtrNFontInfo[index].Width;
            break;
        } else
        { /* cerco nel gruppo su lettere successivo se � presente */
            font = font->NextFontGroup;
        }
    }
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
    /* ??? cosa restituisco qua ? c'� il kerning da tenere in considerazione */
    /* per il momento facciamo cos� */
    const FONTBUILDERFORC_TYPE_CHARACTER *character;

    character = gldfont_GetCharacter (font, ltr);
    if (character)
    {
        width = character->pxl_advance;
    }
#endif

    return width;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'occupazione x dei primi 'size' byte della stringa [pxl].
 Arg: - <font>[in] Font del testo.
        <txt>[in] Stringa di testo.
        <size>[in] Numero di byte da analizzare.
 Ret: - Larghezza totale del testo [pxl].
______________________________________________________________________________*/
int16_t gldfont_GetTxtWidth (const gldfont_t *font, const char *txt, uint32_t size)
{
    int16_t width = 0;

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY) || (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
    const char *txt_end;
    gldtxt_Letter_t ltr;

    /* get all the character until 'size' is reached or string terminator found.
       sum all the letter width */
    txt_end = txt + size;
    txt = gldtxt_GetLetter (&ltr, txt);
    while (txt != NULL && txt <= txt_end)
    {
        width += gldfont_GetLetterWidth (font, ltr);
        txt = gldtxt_GetLetter (&ltr, txt);
    }
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
    const char *txt_end;
    gldtxt_Letter_t ltr;             // letter that i'm considering
    gldtxt_Letter_t previus_ltr = 0; // the letter just before

    /* get all the character until 'size' is reached or string terminator found.
       compute the total cursor x movement. */
    txt_end = txt + size;
    txt = gldtxt_GetLetter (&ltr, txt);
    while (txt != NULL && txt <= txt_end)
    {
        if (previus_ltr)
        { /* a kerning adjustment may be needed */
            width += gldfont_GetKerning (font, previus_ltr, ltr);
        }
        width += gldfont_GetLetterWidth (font, ltr);
        previus_ltr = ltr;
        /* acquisisco la prossima lettera */
        txt = gldtxt_GetLetter (&ltr, txt);
    }
#endif

    return width;
}

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
/*______________________________________________________________________________ 
 Desc:  Ottieni la pixelmap di uno specifico carattere.
 Arg: - <font> il font.
        <ltr> il carattere di cui ottenere il glifo.
 Ret: - Il glifo / pixelmap del carattere.
______________________________________________________________________________*/
const char *gldfont_GetLetterDrawInfo (const gldfont_t *font, gldtxt_Letter_t ltr)
{
    const char *map = NULL;

    if (ltr >= font->FirstChar && ltr <= font->LastChar)
    { /* glyph available */
        uint32_t offset;

        uint16_t index = ((ltr - font->FirstChar) << 2) + 1;
        offset = font->GlyphTable[index] | (font->GlyphTable[index + 1] << 8) | (font->GlyphTable[index + 2] << 16);
        map = font->GlyphMap + offset;
    }

    return map;
}
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
/*______________________________________________________________________________ 
 Desc:  Ottieni la pixelmap di uno specifico carattere.
 Arg: - <font>   il font.
        <ltr>    il carattere di cui ottenere il glifo.
        <map_pp> puntatore in cui viene scritto l'indirizzo di memoria dove si
            trova il glifo della lettera richiesta.
 Ret: - Struttura di informazioni sulla lettera richiesta.
______________________________________________________________________________*/
const gldfont_NFontInfo_t *gldfont_GetLetterDrawInfo (const gldfont_NFont_t *font, gldtxt_Letter_t ltr, const uint8_t **map_pp)
{
    const gldfont_NFontInfo_t *nFontInfo = NULL;

    while (font != NULL)
    {
        if (ltr >= font->FirstChar && ltr <= font->LastChar)
        { /* glyph available */
            uint16_t index = ltr - font->FirstChar;

            nFontInfo = &font->PtrNFontInfo[index];
            *map_pp = &font->PtrFontData[nFontInfo->OffsetData];
            break;
        } else
        { /* cerco nel gruppo su lettere successivo se � presente */
            font = font->NextFontGroup;
        }
    }

    return nFontInfo;
}
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
const FONTBUILDERFORC_TYPE_CHARACTER *gldfont_GetCharacter (const gldfont_t *font, gldtxt_Letter_t ltr)
{
    const FONTBUILDERFORC_TYPE_CHARACTER *character = NULL;

    for (uint8_t i = 0; i < font->num_ranges; i++)
    {
        const FONTBUILDERFORC_TYPE_RANGE *range;

        range = &font->ranges[i];
        if (ltr >= range->first && ltr <= (range->first + range->num_characters))
        { /* glyph available */
            uint16_t index;

            index = ltr - range->first;
            character = &range->characters[index];
            break;
        }
    }
    return character;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
int16_t gldfont_GetKerning (const gldfont_t *font, gldtxt_Letter_t left_ltr, gldtxt_Letter_t right_ltr)
{
    int32_t pxl_advance = 0;

    if ((font->kerning != NULL) && (font->num_kerning != 0))
    { /* kerning info available for this font */
        const FONTBUILDERFORC_TYPE_CHARACTER *character;

        character = gldfont_GetCharacter (font, left_ltr);
        if (character != NULL)
        { /* glyph available */
            /* check for kerning information about 'right_ltr' */
            uint16_t kerning_index;

            kerning_index = character->kerning_index;
            while (kerning_index < font->num_kerning && font->kerning[kerning_index].left_ch == left_ltr)
            {
                if (font->kerning[kerning_index].right_ch == right_ltr)
                { /* position adjustment needed betwen 'left_ltr' and 'right_ltr' */
                    pxl_advance = font->kerning[kerning_index].pxl_adjust;
                    break;
                }
                kerning_index++;
            }
        }
    }
    return pxl_advance;
}
#endif

//============================================================ PRIVATE FUNCTIONS
