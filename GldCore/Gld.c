//===================================================================== INCLUDES
#include "Gld.h"

#include "GldCfg.h"

LIBRARY_INFO (0, 0, "Gld"); // Info versione Major, Minor, Nome modulo
/* v-0.0
    Prima versione ufficialmente rilasciata.
*/

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
gld_Statistics_t gld_Statistics;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Fornisce informazioni su questa libreria, come nome o versione.
 Arg: - Nessun argomento.
 Ret: - Puntatore a struttura constante che raccoglie informazioni specifiche
        per questa libreria.
______________________________________________________________________________*/
const library_Info_t *gld_GetInfo (void)
{
    return &LibInfo;
}

/*______________________________________________________________________________ 
 Desc:  Come il malloc tradizionale, ma tiene traccia della memoria allocata.
 Arg: - <size> dimensione in byte da allocare.
 Ret: - indirizzo della memoria allocata.
______________________________________________________________________________*/
void *gld_Malloc (size_t size)
{
    void *memory = malloc (size);
    if (memory != NULL)
    {
        uint32_t memUsed = *((uint32_t *)memory - 1);
        gld_Statistics.AllocMemCurr += memUsed;
        /* tengo traccia del massimo valore raggiunto */
        if (gld_Statistics.AllocMemCurr > gld_Statistics.AllocMemMax)
            gld_Statistics.AllocMemMax = gld_Statistics.AllocMemCurr;
        GLDCFG_PRINTF ("ok: malloc [addr=0x%08X byte=%d]\n", (uint32_t)memory, memUsed);
    } else
    {
        GLDCFG_PRINTF ("err: fail malloc [%d byte]\n", size);
    }

    return memory;
}

/*______________________________________________________________________________ 
 Desc:  Come il calloc tradizionale, ma tiene traccia della memoria allocata.
 Arg: - <nmemb> numbero di elementi da allocare.
        <size> dimensione in byte di ogni elemento.
 Ret: - indirizzo della memoria allocata.
______________________________________________________________________________*/
void *gld_Calloc (size_t nmemb, size_t size)
{
    void *memory = calloc (nmemb, size);
    if (memory != NULL)
    {
        uint32_t memUsed = *((uint32_t *)memory - 1);
        gld_Statistics.AllocMemCurr += memUsed;
        /* tengo traccia del massimo valore raggiunto */
        if (gld_Statistics.AllocMemCurr > gld_Statistics.AllocMemMax)
            gld_Statistics.AllocMemMax = gld_Statistics.AllocMemCurr;
        
        GLDCFG_PRINTF ("ok: calloc [addr=0x%08X byte=%d]\n", (uint32_t)memory, memUsed);
    } else
    {
        GLDCFG_PRINTF ("err: fail calloc [%d]x[%d byte]\n", nmemb, size);
    }

    return memory;
}

/*______________________________________________________________________________ 
 Desc:  Come il realloc tradizionale, ma tiene traccia della memoria allocata.
 Arg: - <ptr> indirizzo della memoria già allocata.
        <size> nuova dimensione in byte del blocco di memoria.
 Ret: - indirizzo del nuovo blocco di memeoria allocata.
______________________________________________________________________________*/
void *gld_Realloc (void *ptr, size_t size)
{
    void *newPtr;
    uint32_t freeSz = 0; /* dimensone della memoria liberata */
    
    /* con ptr a NULL realloc esegue semplicemente un malloc */
    if (ptr)
        freeSz = *((uint32_t *)ptr - 1);
    
    newPtr = realloc (ptr, size);
    if (newPtr)
    {   /* realloc eseguito con successo */
        uint32_t allocSz; /* dimensione della memoria effettivamente allocata */
        
        allocSz = *((uint32_t *)newPtr - 1);
        gld_Statistics.AllocMemCurr -= freeSz;
        gld_Statistics.AllocMemCurr += allocSz;
        
        /* tengo traccia del massimo valore raggiunto */
        if (gld_Statistics.AllocMemCurr > gld_Statistics.AllocMemMax)
            gld_Statistics.AllocMemMax = gld_Statistics.AllocMemCurr;
        
        GLDCFG_PRINTF ("ok: realloc -free [addr=0x%08X byte=%d] -alloc [addr=0x%08X byte=%d]\n",
            (uint32_t)ptr, freeSz, (uint32_t)newPtr, allocSz);
    } else
    {
        GLDCFG_PRINTF ("err: fail realloc [addr=0x%08X byte=%d]", (uint32_t)ptr, size);
    }
    return newPtr;
}

/*______________________________________________________________________________ 
 Desc:  Come il free tradizionale, ma tiene traccia della memoria allocata.
 Arg: - <memory> indirizzo della memoria da liberare.
 Ret: - None.
______________________________________________________________________________*/
void gld_Free (void *memory)
{
    if (memory != NULL)
    {
        uint32_t memUsed = *((uint32_t *)memory - 1);
        gld_Statistics.AllocMemCurr -= memUsed;
        GLDCFG_PRINTF ("ok: free [addr=0x%08X byte=%d]\n", (uint32_t)memory, memUsed);
        free (memory);
    }
}


//============================================================ PRIVATE FUNCTIONS
