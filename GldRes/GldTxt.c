//===================================================================== INCLUDES
#include "GldRes/GldTxt.h"

#include "GldMisc/GldCoord.h"
#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Ottieni la codifica della prima lettera della stringa 'txt'.
        Assegna la lettera a 'ltr' e restituisce un puntatore alla lettera
        successiva.
 Arg: - <ltr> pnt dove acquisire la prima lettera.
      - <txt> stringa di testo.
 Ret: - Restituisce la posizione della lettera successiva a quella appena
        acquisita. Restituisce NULL non � disponibile una lettera.
______________________________________________________________________________*/
char *gldtxt_GetLetter (gldtxt_Letter_t *ltr, const char *txt)
{
    char *nextLtr = NULL;
    gldtxt_Letter_t letter;

#if (GLDCFG_TXT_ENCODING == GLDCFG_TXT_ENCODING__ANSI)
    /* if letter available */
    if (txt != NULL && txt[0] != '\0')
    {   /* assign the first available letter */
        letter = txt[0];
        nextLtr = (char *)txt + 1;
        
        if (ltr)
            *ltr = letter;
    }
#elif (GLDCFG_TXT_ENCODING == GLDCFG_TXT_ENCODING__UTF8)
    /* if byte available */
    if (txt != NULL && txt[0] != '\0')
    {
        if ((txt[0] & 0x80) == 0x00)
        {   /* one byte letter */
            letter = txt[0] & 0x7F;
            nextLtr = (char *)txt + 1;
        } else if ((txt[0] & 0xE0) == 0xC0 && (txt[1] & 0xC0) == 0x80)
        {   /* two byte letter */
            letter = (txt[1] & 0x3F) | (txt[0] & 0x1F) << 6;
            nextLtr = (char *)txt + 2;
        } else if ((txt[0] & 0xF0) == 0xE0 && (txt[1] & 0xC0) == 0x80 && (txt[2] & 0xC0) == 0x80)
        {   /* three byte letter */
            letter = (txt[2] & 0x3F) | (txt[1] & 0x3F) << 6 | (txt[0] & 0x0F) << 12;
            nextLtr = (char *)txt + 3;
        } else if ((txt[0] & 0xF8) == 0xF0 && (txt[1] & 0xC0) == 0x80 && (txt[2] & 0xC0) == 0x80 && (txt[3] & 0xC0) == 0x80)
        {   /* four byte letter */
            letter = (txt[3] & 0x3F) | (txt[2] & 0x3F) << 6 | (txt[1] & 0x3F) << 12 | (txt[0] & 0x07) << 18;
            nextLtr = (char *)txt + 4;
        } else
        {
            /* this byte format is incompatible. rappresent as a space and move
            to the next */
            letter = ' ';
            nextLtr = (char *)txt + 1;
        }
        
        if (ltr)
            *ltr = letter;
    }
#endif

    return nextLtr;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la prima linea del testo fornito.
 Arg: - Nessun argomento.
 Ret: - Restituisce la posizione della linea successiva a quella appena
        acquisita. Restituisce NULL se non � disponibile una linea.
______________________________________________________________________________*/
char *gldtxt_GetLine (const char *txt, int16_t *size)
{
    char *next_line, *line_end;

    if (*txt != NULL)
    { /* get the first available line end address */
        line_end = strstr (txt, GLDCFG_TXT_EOL);
        if (line_end == NULL)
        {
            line_end = (char *)txt + strlen (txt);
            next_line = line_end;
        } else
        { /* jump the end of line sequence */
            next_line = line_end + strlen (GLDCFG_TXT_EOL);
        }

        /* set the first line available size */
        *size = line_end - txt;
    } else
    {
        next_line = NULL;
    }

    return next_line;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il numero di linee di cui � composto il testo specificato.
        La sequenza di line ending � 'GLDCFG_TXT_EOL'.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
int16_t gldtxt_GetLineCnt (const char *txt)
{
    int16_t lines, line_size;
    const char *next_line;

    /* get all the lines and keep the count */
    lines = 0;
    while ((next_line = gldtxt_GetLine (txt, &line_size)) != NULL)
    {
        lines++;
        txt = next_line;
    }

    return lines;
}

//============================================================ PRIVATE FUNCTIONS
