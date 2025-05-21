#ifndef GLDHALDISP_H_INCLUDED
#define GLDHALDISP_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldMisc/GldColor.h"
#include "GldMisc/GldCoord.h"

//====================================================================== DEFINES
typedef struct
{
    gldcoord_Area_t Canvas;  /* Display area canvas coords */
    gldcoord_Area_t Clip;    /* Must always be a 'Canvas' subset */
    
    /* Questi due parametri sono utilizzati soltanto in modalità double buffer.
    Assieme a 'Canvas' caratterizzano completamente i framebuffer (rettangoli di
    memoria) i cui indirizzi iniziali si ottengono con 'GetFramePtr'. */
    gldcoord_Ori_e Ori;      /* Display memory buffer orientation. Framebuffer memeory orientation */
    gldcoord_t LineOffset;   /* Pixel offset between consecutive frame buffer lines */
    
/*______________________________________________________________________________
 Desc:  Setta il colore di un pixel.
 Arg: - <x>[in] coordinata del pixel.
        <y>[in] coordinata del pixel.
        <clr>[in] colore del pixel. Questo è sempre un uint32_t ARGB8888, dove
            il campo B è il byte meno significativo:
            B = (clr >> 0) & 0xff;
            G = (clr >> 8) & 0xff;
            ...
 Ret: - None.
______________________________________________________________________________*/
    void (*SetPxl) (gldcoord_t x, gldcoord_t y, gldcolor_t clr);

/*______________________________________________________________________________
 Desc:  Ottieni il colore di un pixel.
 Arg: - <x>[in] coordinata del pixel.
        <y>[in] coordinata del pixel.
 Ret: - Colore del pixel. Questo deve sempre essere un uint32_t ARGB8888, con il
    campo B nel byte meno significativo:
    B = (clr >> 0) & 0xff;
    G = (clr >> 8) & 0xff;
    ...
______________________________________________________________________________*/
    gldcolor_t (*GetPxl) (gldcoord_t x, gldcoord_t y);

/*______________________________________________________________________________
 Desc:  Riempi una porzione completa del display con un singolo colore.
 Arg: - <x>[in] coordinata angolo top-left.
        <y>[in] coordinata angolo top-left.
        <x_size>[in] dimensione x dell'area in pixel.
        <y_size>[in] dimensione y dell'area in pixel.
        <clr>[in] colore dell'area.
 Ret: - La funzione deve restituire -1 se per qualsiasi motivo non riesce a
    portare a termine l'operazione. In questo caso la libreria procederà
    effettuando la stessa operazione via software. 
______________________________________________________________________________*/
    int32_t (*FillArea) (gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size, gldcolor_t clr);

/* REQUIRED BY PRE-RENDERING__________________________________________________*/
/*______________________________________________________________________________
 Desc:  Funzione indispensabile per abilitare il pre-rendering.
    Copia i pixel dal buffer temporaneo, nel display effettivo.
 Arg: - <mem_p>[in] indirizzo di memoria dell'area. i pixel sono tutti impaccati:
            non c'è offset fra una linea di pixel e la successiva.
            I pixel di <mem_p> sono disposti secondo l'orientamento <GLDCFG_MEMFB_ORI>.
        <x0>[in] coordinata angolo top-left.
        <y0>[in] coordinata angolo top-left.
        <x_size>[in] dimensione x dell'area in pixel.
        <y_size>[in] dimensione y dell'area in pixel.
 Ret: - Ci si aspetta che questa funzione vada sempre a buon fine. Non c'è altra
    alternativa :-)
______________________________________________________________________________*/
    void (*FlushArea) (void *mem_p, gldcoord_t x0, gldcoord_t y0, gldcoord_t x_size, gldcoord_t y_size);

/* REQUIRED BY DOUBLE-BUFFER__________________________________________________*/
/*______________________________________________________________________________
 Desc:  Ottieni l'indirizzo di memoria di un framebuffer.
 Arg: - <index>[in] indice del framebuffer.
 Ret: - Indirizzo di memoria del framebuffer. La libreria scrive all'interno del
    framebuffer con un formato pixel nativo. Inoltre i pixel del framebuffer
    devono essere impaccati: non è previso offset fra una linea di pixel e la
    successiva.
______________________________________________________________________________*/
    void *(*GetFramePtr) (uint8_t index);

/*______________________________________________________________________________
 Desc:  Rendi visibile il framebuffer [index = 0,1].
 Arg: - <index>[in] indice del framebuffer da rendere visibile.
 Ret: - La funzione deve ritornare soltanto quando il framebuffer è
    effettivamente visibile. L'altro buffer deve essere nascosto quando questa
    funzione ritorna perche comincio a renderizzarci sopra subito.
______________________________________________________________________________*/
    void (*SetVisibleFrame) (uint8_t index);
} gldhaldisp_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GLDHALDISP_H_INCLUDED */
