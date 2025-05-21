//===================================================================== INCLUDES
#include "GldAssets/ObjImage.h"

#include "GldRes/GldImg.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjImage"

typedef struct
{
    gldllist_Node_t Node;
    char *MemPtr; // indirizzo del file in memoria
    const char *FilePath; // path del file cachato
} CacheNode_t;

//=========================================================== PRIVATE PROTOTYPES
static bool GetImageFromCache (const char *source, objimage_Location_e location, bool cachable, const char **ptr);
static int8_t GetGldClrFromRaw (uint8_t imgcvtClr);

static char *CacheGet (const char *path);
static void CacheAdd (const char *path, char *memPtr);
static void CacheRemove (void);

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};
static gldllist_t CacheList; // first element is the lru (last recently used)
static void *(*ImgHeapMalloc) (size_t size);
static void (*ImgHeapFree) (void *ptr);

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea un'oggetto immagine.
    Di default un'oggetto immagine è un'oggetto invisibile di grandezza 100x100.
    La funzione <objimage_SetImage> imposta l'immagine visualizzata da questo
    oggetto e ne definisce anche la dimensione.
 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldimage_t).
 Ret: - None.
______________________________________________________________________________*/
gldobj_t *objimage_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objimage_t, image, gldobj_Create (memSize));
    obj->FuncDraw = objimage_Draw;
    
    if (!ImgHeapMalloc)
        ImgHeapMalloc = gld_Malloc;
    if (!ImgHeapFree)
        ImgHeapFree = gld_Free;
    
    image->Filter.Type = GLDDRAW_BMPFILTER_NONE;
    image->Source = NULL;
    image->Cacheable = true;
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Disegna l'immagine.
 Arg: - <disp> il display driver.
        <obj> l'oggetto immagine da disegnare.
        <cmd> la fase di disegno da portare a termine.
 Ret: - None.
______________________________________________________________________________*/
void objimage_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objimage_t, image, L_OBJ_NAME, obj);
    
    /* perform the object drawing */
    if (cmd == GLDOBJ_DRAW_MAIN) {
        const char *source;
        bool inMemory;
        gldcoord_t xPos, yPos;

        xPos = obj->Area.X1;
        yPos = obj->Area.Y1;
        
        inMemory = GetImageFromCache (image->Source, image->Location, image->Cacheable, &source);
        if (source == NULL)
            return ;
        
        switch (image->Type)
        {
        case OBJIMAGE_TYPE_BMP: { //________________________________BITMAP IMAGE
            if (inMemory)
                glddraw_Bmp (disp, xPos, yPos, GLDCOORD_ALIGN_LT, source, &image->Filter);
            else
                glddraw_FsBmp (disp, xPos, yPos, GLDCOORD_ALIGN_LT, source, &image->Filter);
            break;
        }
        
        case OBJIMAGE_TYPE_RAW: { //___________________________________RAW IMAGE
            if (inMemory) {
                imgcvt_Header_t header;
                gldcoord_Ori_e ori;
                int8_t format;
                
                gldimg_RawGetHeader (&header, source);
                ori = (gldcoord_Ori_e)header.orientation;
                format = GetGldClrFromRaw (header.color_format);
                if (format == -1) {
                    /* formato colore non supportato */
                } else {
                    
                    glddraw_PxlMap (disp, (void *)&source[header.pxl_offset], xPos, yPos,
                        header.width, header.height, ori, (gldcolor_PxlFormat_t)format);
                }
            } else {
#if (0)
                read header
                parse header
                create image memory full rect
                draw_area = intersect (display_canvas, image_area)
                while (draw_area)
                    subarea = get_subarea (draw_area, buffer_size)
                    read subarea from file in buffer
                    draw subarea
                    draw_area -= subarea
#endif
            }
            break;
        }
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Applica un filtro all'immagine.
    Un filtro può per esempio impostare l'immagine in scala di grigi, oppure
    sovrapporre all'immagine un certo color, tipo colorare l'immagine tutta di
    rosso.
 Arg: - <obj> l'immagine su cui applicare il filtro.
        <filter> la definizione del filtro.
            Il contenuto di <filter> viene copiato all'interno dell'oggetto immagine,
            Non serve che la variabile <filter> passata dal chiamante resti allocata.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetFilter (gldobj_t *obj, const glddraw_BmpFilter_t *filter)
{
    GLDASSETS_GET_TYPED (objimage_t, image, L_OBJ_NAME, obj);
    GLDCFG_ASSERT (filter);
    
    /* invalido l'area dell'immagine solo se il nuovo stile � diverso da quello
    attuale */
    if (memcmp (filter, &image->Filter, sizeof (glddraw_BmpFilter_t)) != 0)
    {
        memcpy (&image->Filter, filter, sizeof (glddraw_BmpFilter_t));
        gldobj_Invalidate (&image->Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Modifica l'immagine visualizzata.
    Questa chiamata imposta anche la dimensione dell'oggetto immagine come
    l'immagine che viene impostata: se imposti un'immagine 100x50, l'oggetto
    immagine prende dimensione 100x50.
 Arg: - <loc> se l'immagine si trova in memoria o in filesystem.
        <source> sorgente immagine.
            se l'immagine è in memoria, questo è un puntatore all'immagine.
            se l'immagine è in filesystem, questa è la stringa della path dell'immagine
            all'interno del filesystem.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetImage (gldobj_t *obj, objimage_Location_e loc, const void *source)
{
    GLDASSETS_GET_TYPED (objimage_t, image, "ObjImage", obj);
    
    /* devo effettivamente invalidare l'area di questo oggetto ? */
    bool changed = false;
    gldcoord_t height = 0, width = 0;
    FILE *f = NULL;

    if (loc != image->Location)
        changed = true;
    else if (loc == OBJIMAGE_LOCATION_MEM) {
        if (image->Source != source)
            changed = true;
    }
    else if (loc == OBJIMAGE_LOCATION_FS) {
        if (image->Source == NULL || source == NULL) {
            /* se uno dei due è nullo controllo solamente i puntatori,
            altrimenti comparo le stringhe delle path */
            if (source != image->Source)
                changed = true;
        }
        else if (strcmp (image->Source, source) != 0)
            changed = true;
    }
    
    if (changed)
    {
        char magic[4] = { 0 };
        
        image->Source = NULL;
        /* invalido l'area dell'attuale immagine, quella nuova potrebbe essere
        più piccola */
        gldobj_Invalidate (obj);
        
        if (source)
        {
            bool inMemory;
            const char *memP;
            
            inMemory = GetImageFromCache (source, loc, true, &memP);
            
            /* recupero il megic number (magic bytes / signature) del file */
            if (inMemory)
                memcpy (magic, memP, sizeof (magic));
            else
            {
                f = fopen (source, "rb");
                if (!f) {
                    /* impossibile aprire il file */
                    goto exit;
                }
                if (fread (&magic, 1, sizeof (magic), f) != sizeof (magic)) {
                    /* impossibile leggere i primi byte del file. */
                    goto exit;
                }
            }
            
            /* discrimino il tipo di immagine sulla base della firma.
            se riconosco il formato:
                - decodifico l'header per ottenere larghezza / altezza.
                - se tutto va bene salvo tipo e sorgente.
            un'immagine senza sorgente non viene semplicemente visualizzata
            */
            if (strncmp (magic, "RAW", 3) == 0) // is a raw image
            {
                char tmp[GLDIMG_RAW_SIZE_HEADER];
                imgcvt_Header_t header;
                
                if (inMemory)
                {
                    memcpy (tmp, memP, sizeof (tmp));
                } else
                {
                    size_t rdSz = sizeof (tmp);
                    
                    fseek (f, 0, SEEK_SET);
                    if (fread (tmp, 1, rdSz, f) != rdSz)
                        goto exit;
                }
                
                gldimg_RawGetHeader (&header, tmp);
                width = header.width;
                height = header.height;
                image->Type = OBJIMAGE_TYPE_RAW;
                image->Source = source;
                image->Cacheable = 1;
            }
            else if (strncmp (magic, "BM", 2) == 0) // is a bitmap image
            {
                char tmp[GLDIMG_BMP_SIZE_FILEH + GLDIMG_BMP_SIZE_IMAGEH];
                gldimg_BmpHeaderImage_t header;
                
                if (inMemory)
                {
                    memcpy (tmp, memP, sizeof (tmp));
                } else
                {
                    size_t rdSz = sizeof (tmp);
                    
                    fseek (f, 0, SEEK_SET);
                    if (fread (tmp, 1, rdSz, f) != rdSz)
                        goto exit;
                }
                
                gldimg_BmpGetHeaderImage (&header, tmp);
                width = header.BiWidth;
                height = _ABS_ (header.BiHeight);
                image->Type = OBJIMAGE_TYPE_BMP;
                image->Source = source;
                image->Cacheable = 0;
            }
        }
        
        /* dimensiono l'oggetto esattamente come l'immagine */
        image->Obj.Area.X2 = image->Obj.Area.X1 + width;
        image->Obj.Area.Y2 = image->Obj.Area.Y1 + height;
        
        image->Location = loc;
        
        gldobj_Invalidate (obj);
    }

exit:
    if (f)
        fclose (f);
}

/*______________________________________________________________________________
 Desc:  Imposta un'immagine da filesystem come immagine per questo oggetto.
 Arg: - <obj> l'oggetto immagine.
        <source> la path che identifica l'immagine all'interno del FS.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetImageFs (gldobj_t *obj, const void *source)
{
    objimage_SetImage (obj, OBJIMAGE_LOCATION_FS, source);
}

/*______________________________________________________________________________
 Desc:  Imposta un'immagine che si trova in memoria (flash o ram), come immagine
    per questo oggetto.
 Arg: - <obj> l'oggetto immagine.
        <source> puntatore all'immagine in memoria.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetImageMem (gldobj_t *obj, const void *source)
{
    objimage_SetImage (obj, OBJIMAGE_LOCATION_MEM, source);
}

/*______________________________________________________________________________ 
 Desc:  Cancella la cache di una particolare immagine.
    Se passi NULL cancella la cache di tutte le immagini: tutte le immagini verranno
    rilette prima dal filesystem.
 Arg: - <fname> elimina la cache di questo file.
 Ret: - None.
______________________________________________________________________________*/
void objimage_WipeCache (const char *fname)
{
    CacheNode_t *cacheEntry;
    gldllist_Node_t *node;
    
    if (fname) {
        // cancello l'immagine corrispondente
        for (node = gldllist_GetFirst (&CacheList);
             node;
             node = gldllist_GetNext (node))
        {
            cacheEntry = gldllist_Get (node);
            if (strcmp (fname, cacheEntry->FilePath) == 0) {
                ImgHeapFree (cacheEntry->MemPtr);
                gldllist_Remove (node);
                GLDCFG_FREE (cacheEntry);
            }
        }
    } else {
        // cancello tutti i record della cache immagini
        while ((node = gldllist_GetFirst (&CacheList)) != NULL)
        {
            cacheEntry = gldllist_Get (node);
            ImgHeapFree (cacheEntry->MemPtr);
            gldllist_Remove (node);
            GLDCFG_FREE (cacheEntry);
        }
    }
}

/*______________________________________________________________________________
 Desc:  Imposta la funzione da usare per l'allocazione di memoria nella cache
    immagini.
 Arg: - <func> puntatore a funzione che si comporta come il malloc standard.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetFuncMalloc (void *(*func) (size_t size))
{
    ImgHeapMalloc = func;
}

/*______________________________________________________________________________
 Desc:  Imposta la funzione da usare per liberare memoria nella cache
    immagini.
 Arg: - <func> puntatore a funzione che si comporta come il free standard.
 Ret: - None.
______________________________________________________________________________*/
void objimage_SetFuncFree (void (*func) (void *ptr))
{
    ImgHeapFree = func;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________
 Desc:  Scansiona la cache immagini cercando l'immagine specificata.
    Se l'immagine è in cache, restituisce true e scrive in <ptr> l'indirizzo
    di memoria in cui si trova l'immagine.
 Arg: - <source>   (stinga) la path dell'immagine da cercare.
        <location>
        <cachable> se l'immagine può essere cacheata o no.
        <ptr> puntatore che viene scritto con l'indirizzo di memoria dell'immagine
            in cache, se presente.
 Ret: - true se l'immagine è in cache e <ptr> è stato scritto con l'indirizzo
    di quell'immagine.
______________________________________________________________________________*/
static bool GetImageFromCache (const char *source, objimage_Location_e location, bool cachable, const char **ptr)
{
    bool inMemory = false;
    char *memPtr;
    
    /* nessuna sorgente immagine */
    if (source == NULL)
        *ptr = NULL;
    /* l'immagine è già in memoria, non serve interpellare la cache */
    else if (location == OBJIMAGE_LOCATION_MEM) {
        *ptr = source;
        inMemory = true;
    }
    /* il file dell'immagine è già in cache ? */
    else if ((memPtr = CacheGet (source)) != NULL) {
        *ptr = memPtr;
        inMemory = true;
    }
    /* cache abilitata per questa immagine ?
    se si -> cerco di caricare il file completo in memoria */
    else if (cachable) {
        FILE *f;
        
        f = fopen (source, "rb");
        if (f) {
            size_t fsize; /* dimensione del file */
            
            fseek (f, 0, SEEK_END);
            fsize = ftell (f);
            fseek (f, 0, SEEK_SET);
            
            do
            {
                /* alloco la stringa contente la path dell'immagine subito dopo
                l'immagine stessa */
                memPtr = ImgHeapMalloc (fsize + strlen (source) + 1);
                if (memPtr == NULL)
                {
                    if (CacheList.Size) {
                        /* libreo l'immagine utilizzata meno di recente
                        poi riprovo ad allocare */
                        CacheRemove ( );
                    } else {
                        /* ho liberato tutta la memoria che avevo a disposizione
                        l'immagine è troppo grande */
                        break;
                    }
                }
            } while (memPtr == NULL);
            
            /* se sono riuscito a caricare il file in memoria */
            if (memPtr)
            {
                if (fread (memPtr, 1, fsize, f) == fsize) {
                    char *cachePath;
                    
                    // copio la path del file in fondo alla memoria allocata
                    cachePath = memPtr + fsize;
                    strcpy (cachePath, source);
                    CacheAdd (cachePath, memPtr);
                    *ptr = memPtr;
                    inMemory = true;
                } else {
                    /* non sono riuscito a leggere correttamente tutto il file,
                    libero la memoria che avevo allocato */
                    ImgHeapFree (memPtr);
                }
            }
            fclose (f);
        }
    }
    
    if (inMemory == false)
        *ptr = source;
    return inMemory;
}

/*______________________________________________________________________________
 Desc:  Ottiene l'enum del colore definito dalla libreria, partendo dall'enum
    del colore definitio dal convertitore di immagini.
    Questa funzione praticamente mette in corrispondenza i due enum.
 Arg: - <ingcvtClr> l'enumerativo del formato colore definito dal convertitore
    di immagini.
 Ret: - L'enumerativo del formato colore corrispondente definito dalla libreria
    grafica.
______________________________________________________________________________*/
static int8_t GetGldClrFromRaw (uint8_t imgcvtClr)
{
    struct
    {
        uint8_t imgcvtClr;
        gldcolor_PxlFormat_t gldClr;
    } static const colorBindTable[] =
    {
        { IMGCVT_CLR_FORMAT_BGRA8888, GLDCOLOR_BGRA8888 },
        { IMGCVT_CLR_FORMAT_RGBA8888, GLDCOLOR_RGBA8888 },
        { IMGCVT_CLR_FORMAT_RGB565LE, GLDCOLOR_RGB565LE },
    };
    
    for (int i = 0; i < _NELEMENTS_ (colorBindTable); i++)
    {
        if (colorBindTable[i].imgcvtClr == imgcvtClr)
            return colorBindTable[i].gldClr;
    }
    return -1;
}

/*______________________________________________________________________________
 Desc:  Ispeziona la cache e verifica se l'immagine con la path specificata si
    trova in cache. Se l'immagine è in cache, viene restituito il puntatore
    all'immagine.
 Arg: - <path> (stringa) la path dell'immagine nel FS.
 Ret: - puntatore all'immgaine in memoria (se l'immagine è in cache),
    NULL altrimenti.
______________________________________________________________________________*/
static char *CacheGet (const char *path)
{
    gldllist_Node_t *node;
    char *memPtr = NULL;
    
    /* comincio a cercare dall'entry usata più di recente */
    node = gldllist_GetLast (&CacheList);
    while (node != NULL)
    {
        CacheNode_t *cacheEntry = gldllist_Get (node);
        if (strcmp (path, cacheEntry->FilePath) == 0)
        {   /* cache hit */
            gldllist_Remove (&cacheEntry->Node);
            gldllist_AddLast (&CacheList, &cacheEntry->Node);
            memPtr = cacheEntry->MemPtr;
            break;
        }
        node = gldllist_GetPrev (node);
    }
    return memPtr;
}

/*______________________________________________________________________________
 Desc:  Aggiunge un record alla cache.
 Arg: - <path> (stinga) la path dell'immagine da aggiungere.
            qesta stringa non viene copiata, quindi deve rimanere allocata.
        <memPtr> indirizzo dell'immagine in memoria.
 Ret: - None.
______________________________________________________________________________*/
static void CacheAdd (const char *path, char *memPtr)
{
    CacheNode_t *cacheEntry;
    
    cacheEntry = GLDCFG_MALLOC (sizeof (CacheNode_t) + strlen (path) + 1);
    GLDCFG_ASSERT (cacheEntry);
    gldllist_NodeInit (&cacheEntry->Node);
    gldllist_Set (&cacheEntry->Node, cacheEntry);
    cacheEntry->FilePath = path;
    cacheEntry->MemPtr = memPtr;
    gldllist_AddLast (&CacheList, &cacheEntry->Node);
}

/*______________________________________________________________________________
 Desc:  Rimuove un record dalla cache.
    Viene rimosso il record usato meno di recente, quello che si suppone sarà
    riusato servirà a breve con minore probabilità.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void CacheRemove (void)
{
    gldllist_Node_t *first;
    CacheNode_t *cacheEntry;
    
    first = gldllist_GetFirst (&CacheList);
    if (first) {
        cacheEntry = gldllist_Get (first);
        ImgHeapFree (cacheEntry->MemPtr);
        gldllist_Remove (first);
        GLDCFG_FREE (cacheEntry);
    }
}
