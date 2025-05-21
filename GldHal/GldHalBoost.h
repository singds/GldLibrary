#ifndef GLDHALBOOST_H_INCLUDED
#define GLDHALBOOST_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldMisc/GldColor.h"
#include "GldMisc/GldCoord.h"

//====================================================================== DEFINES
typedef struct
{
/* Hadrware accelerated functions */

/*______________________________________________________________________________
 Desc:  Fonde l'area sorgente con l'area di destinazione, e scrive il risultato
    nell'area di destinazione.
 Arg: - <dstPtr>[in] indirizzo area di destinazione.
        <dstOff>[in] offset in pixel tra una linea e la siccessiva, nell'area di
            destinazione.
        <dstFormat>[in] formato dei pixel dell'area di destinazione.
            Quando la destinazione un framebuffer, il formato è GLDCOLOR_NATIVE.
        <srcPtr>[in] indirizzo area sorgente.
        <srcOff>[in] offset in pixel tra una linea e la siccessiva, nell'area
            sorgente.
        <width>[in] larghezza in pixel dell'area sorgente e destinazione.
        <height>[in] numero di linee dell'area sorgente e destinazione.
 Ret: - La funzione deve restituire -1 se per qualsiasi motivo non riesce a
    portare a termine l'operazione. In questo caso la libreria procederà
    effettuando la stessa operazione via software.
______________________________________________________________________________*/
    int32_t (*BlendArea) (void *dstPtr, uint32_t dstOff, gldcolor_PxlFormat_t dstFormat,
                          void *srcPtr, uint32_t srcOff, gldcolor_PxlFormat_t srcFormat,
                          uint32_t width, uint32_t height);

/*______________________________________________________________________________
 Desc:  Copia l'area di memoria sorgente nell'area di destinazione.
 Arg: - <dstPtr>[in] indirizzo area di destinazione.
        <dstOff>[in] offset in byte tra una linea e la siccessiva, nell'area di
            destinazione.
        <srcPtr>[in] indirizzo area sorgente.
        <srcOff>[in] offset in byte tra una linea e la siccessiva, nell'area
            sorgente.
        <width>[in] larghezza in byte dell'area sorgente e destinazione.
        <height>[in] numero di linee dell'area sorgente e destinazione.
 Ret: - La funzione deve restituire -1 se per qualsiasi motivo non riesce a
    portare a termine l'operazione. In questo caso la libreria procederà
    effettuando la stessa operazione via software.
______________________________________________________________________________*/
    int32_t (*CopyArea) (void *dstPtr, uint32_t dstOff,
                         void *srcPtr, uint32_t srcOff,
                         uint32_t width, uint32_t height);

/*______________________________________________________________________________
 Desc:  Riempie l'area di destinazione ripetendo uno specifico chunck di
        dimensione <srcMemSize> byte. Solitamente <srcMem> punta alla
        rappresentazione in memoria di un pixel nativo, e <srcMemSize> è la
        dimensione in byte di questo pixel.
 Arg: - <dstPtr>[in] indirizzo area di destinazione.
        <width>[in] numero di chunck che costituiscono una linea dell'area di
            destinazione.
        <height>[in] numero di linee dell'area di destinazione.
        <srcOff>[in] offset in byte tra una linea e la siccessiva, nell'area
            sorgente.
        <srcMem>[in] Indirizzo di memoria del campione di chunck da ripetere.
        <srcMemSize>[in] Dimensione in byte del chunck.
 Ret: - La funzione deve restituire -1 se per qualsiasi motivo non riesce a
    portare a termine l'operazione. In questo caso la libreria procederà
    effettuando la stessa operazione via software.
______________________________________________________________________________*/
    int32_t (*FillArea) (void *dstPtr, uint32_t width, uint32_t height,
                         uint32_t off, void *srcMem, uint8_t srcMemSize);
} gldhalboost_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GLDHALDISP_H_INCLUDED */
