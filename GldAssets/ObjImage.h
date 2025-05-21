#ifndef OBJIMAGE_H_INCLUDED
#define OBJIMAGE_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"

//====================================================================== DEFINES
#define OBJIMAGE_DEF(obj, parent) \
    GLDASSETS_DEF (objimage_t, thisImage, "ObjImage", obj, parent)

typedef enum
{
    OBJIMAGE_LOCATION_MEM, /* sorgente immagine in mamoria */
    OBJIMAGE_LOCATION_FS,  /* sorgente immagine da file system */
} objimage_Location_e;

typedef enum
{
    OBJIMAGE_TYPE_BMP, // l'immagine è in formato bitmap
    OBJIMAGE_TYPE_RAW, // l'immagine è in formato raw = raw pixel
} objimage_Type_e;

typedef struct
{
    gldobj_t Obj; /* radice dell'oggetto (comune a tutte le tipologie di oggetto) */

    /* enum che definisce se l'immagine è in memoria (flash o ram), oppure se si
    trova su un filesystem */
    objimage_Location_e Location : 2;
    /* il formato dell'immagine, bitmap o raw. */
    objimage_Type_e Type : 2;
    /* true se l'immagine può essere cacheata in memoria.
    Quando un'immagine viene creata, di deafult è cacheabile.
    Probabilmente vuoi resettare questo flag in immagini molto grandi, che vengono
    visualizzate one-shot, come le immagini dello screensaver.
    */
    bool Cacheable : 1;
    /* sorgente dell'immagine.
    per immagine mappata in memoria (OBJIMAGE_LOCATION_MEM) <Source> contiene
    l'indirizzo dell'immagine.
    per immagine da filesystem (OBJIMAGE_LOCATION_FS) <Source> contiene la path
    del file */
    const char *Source;
    glddraw_BmpFilter_t Filter;     /* filtro immagine */
} objimage_t;

//============================================================= GLOBAL VARIABLES


//============================================================ GLOBAL PROTOTYPES
gldobj_t *objimage_Create (uint32_t memSize);
void objimage_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd);

void objimage_WipeCache (const char *fname);
void objimage_SetFilter (gldobj_t *obj, const glddraw_BmpFilter_t *filter);
void objimage_SetImage (gldobj_t *obj, objimage_Location_e loc, const void *source);
void objimage_SetImageFs (gldobj_t *obj, const void *source);
void objimage_SetImageMem (gldobj_t *obj, const void *source);
void objimage_SetFuncMalloc (void *(*func) (size_t size));
void objimage_SetFuncFree (void (*func) (void *ptr));

#endif /* OBJIMAGE_H_INCLUDED */
