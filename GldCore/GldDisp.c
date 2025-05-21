//===================================================================== INCLUDES
#include "GldCore/GldDisp.h"
#include "GldCfg.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
/* il driver display che usa la libreria */
gldhaldisp_t *glddisp_Disp = NULL;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Inizializza il modulo display impostando il driver display che dovrà
    usare la libreria.
 Arg: - <disp> il driver display che dovrà usare la libreria.
 Ret: - None.
______________________________________________________________________________*/
void glddisp_Init (gldhaldisp_t * disp)
{
    glddisp_Disp = disp;
}

/*______________________________________________________________________________
 Desc:  Imposta una sottoarea interna al display, dove è concesso disegnare.
    Dopo che questa sottoarea è stat impostata, tutte le funzioni di disegno
    che scrivono sul display, lo fanno solo all'interno di questa area.
    Nessun pixel viene modificato al di fuori della clip area.
 Arg: - <disp> il display su cui impostare la clip area.
        <area> le coordinate della clip area.
 Ret: - None.
______________________________________________________________________________*/
void glddisp_SetClipArea (gldhaldisp_t *disp, const gldcoord_Area_t *area)
{
    if (disp != NULL && area != NULL)
    {
        gldcoord_Area_t clip_area;
        /* cerco l'intersezione fra il clip richiesto e la tela del display.
           'Clip' deve essere un sottoinsieme di 'Canvas' */
        gldcoord_AreaCopy (&clip_area, &disp->Canvas);
        gldcoord_AreaIntersection (&clip_area, &clip_area, area);
        gldcoord_AreaCopy (&disp->Clip, &clip_area);
    }
}

//============================================================ PRIVATE FUNCTIONS
