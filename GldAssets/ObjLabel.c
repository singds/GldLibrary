//===================================================================== INCLUDES
#include "GldAssets/ObjLabel.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjLabel"

//=========================================================== PRIVATE PROTOTYPES
static int32_t LimitTextWidth (char *text, uint16_t size, gldcoord_t max_width, const gldfont_t *font);
static gldcoord_t GetLineMaxPxl (objlabel_t *label);

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea una label.
    La label quando viene creata ha dimensione 100x100.
    Appena la label viene aggiunta come figlio di un'altro oggetto, acquista
    la dimensione e posizione di suo padre.
    Di default il testo viene disegnato a partire dall'angolo top-left dell'oggetto
    label. La posizione del testo può essere modificata con <objlabel_SetPos> e
    modificando il campo Align della label.

    Una label di default non ha memoria per contenere la stringa di testo da
    visualizzare.
    - Utilizzare la funzione objlabel_SetStaticTxt per impostare un stringa di
      testo statica, che il chiamante deve garantire che sia sempre disponibile
      in memoria.
    - Altimenti usare objlabel_Alloca + objlabel_Printf.

 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldlabel_t).
 Ret: - None.
______________________________________________________________________________*/
gldobj_t *objlabel_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objlabel_t, label, gldobj_Create (memSize));
    obj->FuncDraw = objlabel_Draw;
    obj->FuncEvent = objlabel_Event;
    
    label->Align = GLDCOORD_ALIGN_LT;
    label->AutoNewline = true;
    label->Font = NULL;
    label->Color = GLDCOLOR_BLACK;
    label->ColorBg = GLDCOLOR_NONE;
    label->Pos.X = 0;
    label->Pos.Y = 0;
    label->TxtStatic = NULL;
    label->TxtDynamic = NULL;
    label->TxtDynamicSize = 0;
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Disegna la label.
 Arg: - <disp> il display su cui disegnare.
        <obj> l'oggetto label da disegnare.
        <cmd> la fase di disegno da portare a termine.
 Ret: - None.
______________________________________________________________________________*/
void objlabel_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    /* perform the object drawing */
    if (cmd == GLDOBJ_DRAW_MAIN) {
        
        const char *srcTxt;
        
        if (label->TxtIsStatic)
            srcTxt = label->TxtStatic;
        else
            srcTxt = label->TxtDynamic;
        
        if (srcTxt)
        {
            char *textLimited;
            int textLimitedSz;
            uint16_t textSz;
            gldcoord_t x_pos, y_pos;
            
            textSz = strlen (srcTxt) + 1;
            textLimitedSz = 2 * textSz;
            textLimited = GLDCFG_MALLOC (textLimitedSz); /* copia della label con dimensione x limitata */
            GLDCFG_ASSERT (textLimited);

            x_pos = obj->Area.X1 + label->Pos.X;
            y_pos = obj->Area.Y1 + label->Pos.Y;

            if (label->AutoNewline == true)
            { /* creo una versione del testo con dimensione massima limitata alla
                grandezza grafica della label: inserendo caporiga dove necessario */
                gldcoord_t max_x_pxl;

                memcpy (textLimited, srcTxt, textSz);

                /* calcolo quanti pixel pi� occupare al massimo una linea */
                max_x_pxl = GetLineMaxPxl (label);
                LimitTextWidth (textLimited, textLimitedSz, max_x_pxl, label->Font);

                srcTxt = textLimited;
            }

            glddraw_Printf (disp, x_pos, y_pos, label->Align, label->Font, label->Color, label->ColorBg, "%s", srcTxt);
            GLDCFG_FREE (textLimited);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Funzione di gestione eventi per la label.
    Questa funzione, per esempio, ridimensiona l'oggetto label quando il padre
    della label viene cambiato o la sua dimensione viene modificata.
 Arg: - <obj> l'oggetto label su cui è avvento l'evento.
        <evt> enum che identifica l'evento
        <ext> eventuali dati aggiuntivi associati all'evento.
 Ret: - None.
______________________________________________________________________________*/
void objlabel_Event (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);

    switch (evt) {
        case GLDOBJ_EVENT_PARENT_COORD_CHANGED: {
            gldcoord_Area_t *oldArea = ext;
            if (gldobj_GetWidth (obj->Parent) != gldcoord_AreaWidth (oldArea) ||
                gldobj_GetHeight (obj->Parent) != gldcoord_AreaHeight (oldArea))
            {
                gldobj_SetRelPos (obj, obj->Parent, 0, 0);
                gldobj_SetSizeAs (obj, obj->Parent);
            }
            break;
        }
        case GLDOBJ_EVENT_PARENT_CHANGED: {
            if (obj->Parent) {
                gldobj_SetRelPos (obj, obj->Parent, 0, 0);
                gldobj_SetSizeAs (obj, obj->Parent);
            }
            break;
        }
        case GLDOBJ_EVENT_DESTROY: {
            /* libero la memoria allocata per il testo dinamico */
            if (label->TxtDynamic)
                GLDCFG_FREE (label->TxtDynamic);
            break;
        }
        default: break;
    }
}

/*______________________________________________________________________________ 
 Desc:  Alloca, sull'oggetto label, una certa quantità di memoria per contenere
    un printf della stringa da visualizzare.
 Arg: - <obj> la label su cui allocare.
        <size> byte di memoria da allocare per la stringa.
 Ret: - la memoria allocata.
______________________________________________________________________________*/
void *objlabel_Alloca (gldobj_t *obj, uint16_t size)
{
    void *newPtr = NULL;
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    if (size == 0) {
        /* libero la memoria allocata per il testo */
        if (label->TxtDynamic)
            GLDCFG_FREE (label->TxtDynamic);
        label->TxtDynamic = NULL;
        label->TxtDynamicSize = 0;
    }
    else {
        /* se label->TxtDynamic è NULL, realloc esegue semplicemente un malloc */
        newPtr = GLDCFG_REALLOC (label->TxtDynamic, size);
        
        if (newPtr) { /* realloc riuscito */
            
            if (label->TxtDynamic == NULL) // prima allocazione
                *((char *)newPtr) = 0; // terminatore
            label->TxtDynamic = newPtr;
            label->TxtDynamicSize = size;
        } else { /* realloc fallito */
            
            newPtr = label->TxtDynamic;
        }
    }
    return newPtr;
}

/*______________________________________________________________________________ 
 Desc:  Modifica il testo della label. La funzione � in stile printf e accetta
        lo stesso tipo di formattazione. La rappresentazione della label viene
        aggiornata solamente se il nuovo testo � diverso da quello precedente.
        
        Bisogna chamare <objlabel_Alloca> prima di questa funzione !
 Arg: - <obj> la label su cui stampare.
        <format> la formattazione della stringa da stampare.
        <...> altri parametri
 Ret: - la quantità di caratteri scritti (terminatore 0 non contato).
______________________________________________________________________________*/
int32_t objlabel_Printf (gldobj_t *obj, const char *format, ...)
{
    int32_t ret = 0;
    va_list va;
    
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    va_start (va, format);
    ret = objlabel_VPrintf (obj, format, va);
    va_end (va);
    return ret;
}

/*______________________________________________________________________________
 Desc:  Modifica il testo della label stampandoci dentro.
        Concettualmente identica a vprintf.
 Arg: - <obj> la label su cui stampare.
        <format> la formattazione della stringa da stampare.
        <va> lista di altri argomenti.
 Ret: - la quantità di caratteri scritti (terminatore 0 non contato).
______________________________________________________________________________*/
int32_t objlabel_VPrintf (gldobj_t *obj, const char *format, va_list va)
{
    int32_t ret = 0;
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    if (label->TxtDynamic == NULL)
        /* non hai allocato memoria per questa label, vedi objlabel_Alloca */
        return 0;

    if (format != NULL)
    {
        bool needInvalid = false;
        char* newTxt;
        
        /* alloco una stringa temporanea per contenere il printf.
        la creo di dimensione pari al numero di byte del testo dinamico che sono
        allocati per questa label */
        newTxt = GLDCFG_MALLOC(label->TxtDynamicSize);
        GLDCFG_ASSERT (newTxt);

        ret = vsnprintf (newTxt, label->TxtDynamicSize, format, va);
        newTxt[label->TxtDynamicSize - 1] = '\0'; /* assicuro il terminatore */

        /* verifico se il nuovo testo è diverso da quello già presente.
        in caso lo salvo e rinfresco la visualizzazione */
        if (strcmp (label->TxtDynamic, newTxt) != 0)
        {
            uint16_t copySz; /* numero di byte da copiare */
            
            needInvalid = true;
            copySz = strlen (newTxt) + 1;
            memcpy (label->TxtDynamic, newTxt, copySz);
        }
        /* se stavo visualizzando un testo statico rinfrasco la grafica */
        if (label->TxtIsStatic == true)
            needInvalid = true;
        
        label->TxtIsStatic = false;
        if (needInvalid)
            gldobj_Invalidate (&label->Obj);
        GLDCFG_FREE (newTxt);
    }
    
    return ret;
}

/*______________________________________________________________________________ 
 Desc:  Imposta un testo statico come testo visualizzato dalla label.
    <txt> non viene copiato all'interno della stringa, ma soltanto salvato come
    riferimento. Quindi il chiamante deve garantire che <txt> sia sempre in
    memoria.
 Arg: - <obj> la label su cui impostare il testo.
        <txt> il testo da impostare.
 Ret: - None.
______________________________________________________________________________*/
void objlabel_SetStaticTxt (gldobj_t *obj, const char *txt)
{
    bool needInvalid = false;
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    /* se è cambiato l'indirizzo di memoria del testo da visualizzare, rinfresco
    la grafica. */
    if (txt != label->TxtStatic) {
        needInvalid = true;
        label->TxtStatic = txt;
    }
        
    /* se stavo visualizzando un testo dinamico rinfrasco la grafica */
    if (label->TxtIsStatic == false)
        needInvalid = true;
    
    label->TxtIsStatic = true;
    if (needInvalid)
        gldobj_Invalidate (&label->Obj);
}

/*______________________________________________________________________________ 
 Desc:  Imposta il colore del testo della label.
 Arg: - <obj> la label.
        <color> il colore del testo.
 Ret: - None.
______________________________________________________________________________*/
void objlabel_SetColor (gldobj_t *obj, gldcolor_t color)
{
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);

    if (label->Color != color) {
        /* modifico il colore del testo e invalido la rappresentazione della label */
        label->Color = color;
        gldobj_Invalidate (&label->Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta la posizione del punto di riferimento del testo, in coordinate
        relative all'area della label. Le coordinate di questo punto sono usate
        nell'allineamento del testo.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void objlabel_SetPos (gldobj_t *obj, gldcoord_t x, gldcoord_t y)
{
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);

    if (x != label->Pos.X || y != label->Pos.Y)
    {
        label->Pos.X = x;
        label->Pos.Y = y;
        gldobj_Invalidate (obj);
    }
}

/*______________________________________________________________________________
 Desc:  Ottieni la stringa di testo che viene visualizzata nella label.
 Arg: - <obj> l'oggetto label.
 Ret: - la stringa visualizzata nella label.
______________________________________________________________________________*/
const char *objlabel_GetTxt (gldobj_t *obj)
{
    const char *ret;
    GLDASSETS_GET_TYPED (objlabel_t, label, L_OBJ_NAME, obj);
    
    if (label->TxtIsStatic)
        ret = label->TxtStatic;
    else
        ret = label->TxtDynamic;
    return ret;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Modifica la stringa 'text' in modo tale che ogni linea non superi la
        dimensione x (width) massima specificata. Per limitare la linghezza,
        l'algoritmo inserisce la sequenza di caporiga dove necessario.
 Arg: - <text> Puntatora al testo da limitare, ATTENZIONE che il contenuto e
               l'occupazione in byte della stringa vengono modificati dall'algoritmo.
        <size> Massimo spazio in byte disponibile per il testo.
        <max_width> Dimensine massima di una linea.
        <font_i> Informazioni sul font utilizzato.
 Ret: - None.
______________________________________________________________________________*/
static int32_t LimitTextWidth (char *text, uint16_t size, gldcoord_t max_width, const gldfont_t *font)
{
    char *curr_line, *next_line;
    int16_t line_bytes;

    curr_line = text;
    while ((next_line = gldtxt_GetLine (curr_line, &line_bytes)) != NULL)
    {
        int16_t line_width;              /* dimensione x della linea (pxl) */
        char *ltr_curr;                  // puntatore alla lettera che sto analizzando
        char *ltr_next;                  // puntatore alla prossima lettera
        char *last_space;                // puntatore all'ultimo spazio che ho identificato
        char *ltr_end;                   // puntatore al primo byte non appartenente alla stringa
        gldtxt_Letter_t ltr;             // unicode lettera che sto analizzando
        gldtxt_Letter_t previus_ltr = 0; // unicode lettera appena precedente

        /* get all the character until 'size' is reached or string terminator found.
           sum all the letter width */
        line_width = 0;
        last_space = NULL; /* posizione dell'ultimo spazio */

        ltr_end = curr_line + line_bytes;
        ltr_curr = curr_line;
        while ((ltr_curr < ltr_end) && (ltr_next = gldtxt_GetLetter (&ltr, ltr_curr)) != NULL)
        {
            int16_t ltr_width;                                 /* dimensione in pxl della lettera corrente */
            const int16_t eol_bytes = strlen (GLDCFG_TXT_EOL); /* assegno la dimensione in byte della sequenza di end-of-line */

            if (strlen (text) + eol_bytes + 1 > size)
            { /* Non c'� pi� spazio per aggiungere ulteriori caporiga */
                /* Should never reach this point */
                break;
            }

            if (ltr == ' ')
                last_space = ltr_curr;

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
            if (previus_ltr)
                line_width += gldfont_GetKerning (font, previus_ltr, ltr);
            previus_ltr = ltr;
#endif

            ltr_width = gldfont_GetLetterWidth (font, ltr);
            if (line_width + ltr_width > max_width)
            { /* Con l'aggiunta di questa lettera la linea diventa troppo lunga */
                const char *src_end = text + strlen (text) + 1;

                if (last_space != NULL)
                { /* inserisco un caporiga al posto dell'ultimo spazio identificato */
                    memmove (last_space + eol_bytes, last_space + 1, (src_end - (last_space + 1)));
                    memcpy (last_space, GLDCFG_TXT_EOL, eol_bytes);

                    /* Ho sostituito nel mezzo della stringa uno spazio con una
                       sequanza di EOL. Il puntatore alla linea successiva si
                       sposta di conseguenza. */
                    next_line += eol_bytes - 1;
                    /* metto il 'cursore' subito dopo l'EOL che ho inserito */
                    ltr_next = last_space + eol_bytes;
                    last_space = NULL;
                } else
                { /* inserisco un caporiga nel mezzo di questa parola,
                     appena prima della lettera corrente */
                    memmove (ltr_curr + eol_bytes, ltr_curr, (src_end - ltr_curr));
                    memcpy (ltr_curr, GLDCFG_TXT_EOL, eol_bytes);

                    /* Ho aggiunto nel mezzo della stringa una sequanza di EOL.
                       Il puntatore alla linea successiva si sposta di conseguenza. */
                    next_line += eol_bytes;
                    /* metto il 'cursore' subito dopo l'EOL che ho inserito */
                    ltr_next = ltr_curr + eol_bytes;
                }
                previus_ltr = 0;
                line_width = 0;
            } else
                line_width += ltr_width;

            /* move to next letter */
            ltr_curr = ltr_next;
        }

        /* move to next line */
        curr_line = next_line;
    }

    return 0;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il numero massimo di pixel che pu� occupare una linea di testo.
        Il caporiga automatico manda a capo il testo per garantire che ogni linea
        abbia una dimensione in pixel minore di questa.
 Arg: - <label> l'oggetto label.
 Ret: - il numero massimo di pixel che può occupare una linea di testo.
______________________________________________________________________________*/
static gldcoord_t GetLineMaxPxl (objlabel_t *label)
{
    gldcoord_t max_x_pxl = 0;
    switch (GLDCOORD_GET_HALIGN (label->Align))
    {
    default:
    case GLDCOORD_ALIGN_LEFT:
        max_x_pxl = gldcoord_AreaWidth (&label->Obj.Area) - (label->Pos.X);
        break;

    case GLDCOORD_ALIGN_RIGHT:
        max_x_pxl = label->Pos.X;
        break;

    case GLDCOORD_ALIGN_HMID:
        max_x_pxl = 2 * _MIN_ (label->Pos.X, (gldcoord_AreaWidth (&label->Obj.Area) - (label->Pos.X)));
        break;
    }
    return max_x_pxl;
}
