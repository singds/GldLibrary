//===================================================================== INCLUDES
#include "GldAssets/ObjProgressBar.h"

//====================================================================== DEFINES
#define L_OBJ_NAME            "ObjProgressbar"

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES
static gldobj_Class_t ObjClass = {
    .Name = L_OBJ_NAME,
};

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Crea un'oggetto progress bar.
    Di default la progress bar si riempie da sinistra a destra con un colore
    azzurro, ha un bordo nero di 4 pxl e gli angoli arrotondati con raggio 6 pxl.
    La prograss bar ha dimensione 100x20 pxl di default.
 Arg: - <memSize> dimensione di memoria da allocare per l'oggetto.
        se 0 alloca sizeof(gldimage_t).
 Ret: - Un nuovo oggetto progress bar.
______________________________________________________________________________*/
gldobj_t *objprogressbar_Create (uint32_t memSize)
{
    GLDASSETS_ALLOCATE (objprogressbar_t, progressBar, objbutton_Create (memSize));
    obj->FuncDraw = objprogressbar_Draw;
    obj->Area.X1 = 0;
    obj->Area.Y1 = 0;
    obj->Area.X2 = 100;
    obj->Area.Y2 = 20;

    progressBar->Percentuale = 0;
    progressBar->Orientation = OBJPROGRESSBAR_ORI_LR;
    progressBar->BodyRadius = 6;
    progressBar->BorderSize = 4;
    progressBar->BodyColor = 0x3c90f3;
    progressBar->BorderColor = GLDCOLOR_BLACK;
    return obj;
}

/*______________________________________________________________________________ 
 Desc:  Disegna la progress bar.
 Arg: - <disp> il display driver.
        <obj>  l'oggetto label da disegnare.
        <cmd>  la fase di disegno da portare a termine.
 Ret: - None.
______________________________________________________________________________*/
void objprogressbar_Draw (gldhaldisp_t *disp, gldobj_t *obj, gldobj_Draw_e cmd)
{
    GLDASSETS_GET_TYPED (objprogressbar_t, progressBar, L_OBJ_NAME, obj);
    
    /* perform the object drawing */
    if (cmd == GLDOBJ_DRAW_MAIN) {
        gldcoord_Area_t *area = &progressBar->Button.Obj.Area;
        gldcoord_Area_t saveArea, barArea;
        gldcoord_t radius, barPxl, x1, x2, y1, y2;
        gldcolor_t barClr;

        if (progressBar->BorderColor != GLDCOLOR_NONE && progressBar->BorderSize > 0)
            glddraw_EmptyRound (disp, area->X1, area->Y1, area->X2, area->Y2, progressBar->BorderSize, progressBar->BodyRadius, progressBar->BorderColor);

        /* queste sono le coordinate della barra completa */
        x1 = area->X1 + progressBar->BorderSize;
        y1 = area->Y1 + progressBar->BorderSize;
        x2 = area->X2 - progressBar->BorderSize;
        y2 = area->Y2 - progressBar->BorderSize;

        /* creo l'area con cui andr� a mascherare la barra completa per dare
        l'effetto del riempimento */
        barArea.X1 = x1;
        barArea.Y1 = y1;
        barArea.X2 = x2;
        barArea.Y2 = y2;

        radius = 0;
        if (progressBar->BodyRadius > progressBar->BorderSize)
            radius = progressBar->BodyRadius - progressBar->BorderSize;

        /* definisco il colore della barra */
        barClr = progressBar->BodyColor;

        if (progressBar->Orientation <= 1)
        { /* orientamento orizzontale */
            barPxl = (progressBar->Percentuale * (barArea.X2 - barArea.X1)) / 100;

            if (progressBar->Orientation == OBJPROGRESSBAR_ORI_LR)
                barArea.X2 = barArea.X1 + barPxl;
            if (progressBar->Orientation == OBJPROGRESSBAR_ORI_RL)
                barArea.X1 = barArea.X2 - barPxl;
        } else
        { /* orientamento verticale */
            barPxl = (progressBar->Percentuale * (barArea.Y2 - barArea.Y1)) / 100;

            if (progressBar->Orientation == OBJPROGRESSBAR_ORI_TB)
                barArea.Y2 = barArea.Y1 + barPxl;
            if (progressBar->Orientation == OBJPROGRESSBAR_ORI_BT)
                barArea.Y1 = barArea.Y2 - barPxl;
        }

        /* salvo l'attuale area attiva del display */
        saveArea = disp->Clip;
        /* riduco l'area attiva all'intersezione con 'barArea' */
        gldcoord_AreaIntersection (&disp->Clip, &disp->Clip, &barArea);

        /* rappresento l'intero contenuto della barra */
        glddraw_FillRound (disp, x1, y1, x2, y2, radius, barClr);

        /* ripristino la precedente area attiva del display */
        disp->Clip = saveArea;
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta il colore del corpo della progress bar.
 Arg: - <obj> la progress bar.
        <bodyClr> il nuovo colore del corpo.
 Ret: - None.
______________________________________________________________________________*/
void objprogressbar_SetBodyClr (gldobj_t *obj, gldcolor_t bodyClr)
{
    GLDASSETS_GET_TYPED (objprogressbar_t, progressBar, L_OBJ_NAME, obj);

    if (progressBar->BodyColor != bodyClr)
    { /* modifico il colore e invalido la rappresentazione della barra */
        progressBar->BodyColor = bodyClr;
        gldobj_Invalidate (&progressBar->Button.Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta il colore del contorno della progress bar.
 Arg: - <obj> la progress bar.
        <border_clr> il nuovo colore del bordo.
 Ret: - None.
______________________________________________________________________________*/
void objprogressbar_SetBorderClr (gldobj_t *obj, gldcolor_t border_clr)
{
    GLDASSETS_GET_TYPED (objprogressbar_t, progressBar, L_OBJ_NAME, obj);

    if (progressBar->BorderColor != border_clr)
    { /* modifico il colore e invalido la rappresentazione del bottone */
        progressBar->BorderColor = border_clr;
        gldobj_Invalidate (&progressBar->Button.Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Imposta la percentuale di prograsso della barra.
 Arg: - <obj>[in] oggetto progressbar.
        <progress>[in] percentuale di progresso [0-100].
 Ret: - None.
______________________________________________________________________________*/
void objprogressbar_SetProgress (gldobj_t *obj, uint8_t progress)
{
    GLDASSETS_GET_TYPED (objprogressbar_t, progressBar, L_OBJ_NAME, obj);

    progress = _MIN_ (100, progress); /* cimo la variabile a 100 */
    if (progress != progressBar->Percentuale)
    {
        progressBar->Percentuale = progress;
        gldobj_Invalidate (&progressBar->Button.Obj);
    }
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la percentuale di prograsso della barra.
 Arg: - <obj>[in] oggetto progressbar.
 Ret: - None.
______________________________________________________________________________*/
uint8_t objprogressbar_GetProgress (gldobj_t *obj)
{
    GLDASSETS_GET_TYPED (objprogressbar_t, progressBar, L_OBJ_NAME, obj);
    return progressBar->Percentuale;
}

//============================================================ PRIVATE FUNCTIONS
