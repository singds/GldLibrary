//===================================================================== INCLUDES
#include "GldCore/GldRefr.h"

#include "GldCfg.h"
#include "GldCore/GldDisp.h"
#include "GldCore/GldMemFb.h"
#include "GldMisc/GldRect.h"
#include <string.h>

//====================================================================== DEFINES
#if (GLDCFG_DOUBLE_BUFFER == 1)
#define L_FRAMEBUFFER_NUM 2
#else
#define L_FRAMEBUFFER_NUM 1
#endif

//=========================================================== PRIVATE PROTOTYPES
static void RefrInvalidAreas (gldhaldisp_t *disp, bool probe);
static void RefrAll (gldhaldisp_t *disp, gldcoord_Area_t *area);
static void RefrObj (gldhaldisp_t *disp, gldobj_t *obj, gldcoord_Area_t *area);
static void CopyValidAreas (uint8_t scrFrame, uint8_t dstFrame);

//============================================================= STATIC VARIABLES
struct
{
    gldcoord_Area_t List[GLDCFG_INVALID_AREAS_NUM]; // lista delle aree invalide
    uint16_t Num; // dimensione della lista
} static InvalidAreas[L_FRAMEBUFFER_NUM];
/* l'indice del framebuffer su cui sta avvenendo il redering della scena. */
static uint8_t WorkingFb;
/* l'indice del framebuffer attualmente visibile */
static uint8_t VisibleFb;

//============================================================= GLOBAL VARIABLES
/* l'oggetto che sta alla radice di tutti gli oggetti visibili.
Contiene tutti gli oggetti definiti dall'utente e dal sistema.
 */
gldobj_t *gldrefr_RootObj;
/* l'oggetto che sta alla radice di tutti gli oggetti definiti dall'utente */
gldobj_t *gldrefr_UserLayer;
/* l'oggetto che sta alla radice di tutti gli oggetti che il sistema sovrappone
agli oggetti definiti dall'utente.
Un oggetto definito dal sistema è per esempio il puntatore del touch, che mostra
dove l'utente sta premendo.
*/
gldobj_t *gldrefr_SystemLayer;

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Inizializza questo modulo. Chiama questa funzione prima di utilizzare le
        altre funzioni del modulo.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldrefr_Init (void)
{
    /* Creo l'oggetto radice del refresh. Ha la dimensione del display. */
    GLDOBJ_DEF (gldrefr_RootObj = gldobj_Create (0), NULL)
    {
        thisObj->Area.X1 = GLDCOORD_MIN;
        thisObj->Area.Y1 = GLDCOORD_MIN;
        thisObj->Area.X2 = GLDCOORD_MAX;
        thisObj->Area.Y2 = GLDCOORD_MAX;
        
        /* layer utente. � soltanto un contenitore globale, che non viene mai
        distrutto, di tutti gli oggetti utente */
        GLDOBJ_DEF (gldrefr_UserLayer = gldobj_Create (0), thisObj)
        {
            thisObj->Area.X1 = GLDCOORD_MIN;
            thisObj->Area.Y1 = GLDCOORD_MIN;
            thisObj->Area.X2 = GLDCOORD_MAX;
            thisObj->Area.Y2 = GLDCOORD_MAX;
        }
        /* il layer di sistema � in primo piano rispetto a quello utente */
        GLDOBJ_DEF (gldrefr_SystemLayer = gldobj_Create (0), thisObj)
        {
            thisObj->Area.X1 = GLDCOORD_MIN;
            thisObj->Area.Y1 = GLDCOORD_MIN;
            thisObj->Area.X2 = GLDCOORD_MAX;
            thisObj->Area.Y2 = GLDCOORD_MAX;
        }
    }
    WorkingFb = 0;
    VisibleFb = 0;
}

/*______________________________________________________________________________ 
 Desc:  Invalida la visualizzazione di un'area.
        Con invalidare un'area si intende dire che quell'area necessita di
        essere ridisegnata. Per esempio, quando viene spostato un'oggetto, l'area
        che occupava nella vecchia e nuova posizione vengono invalidate.
        Cos� il motore di refresh 'gldrefr_Task' quando messo in esecuzione, per 
        ciascuna area invalidata, andr� a ridisegnare bottom-up tutti gli oggetti
        che si trovano sopra almeno in parte ad essa.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldrefr_InvalidateArea (gldcoord_Area_t *area)
{
    /* in double framebuffer invalido l'area per tutti i buffer.
    tengo liste separate per le aree dirty dei due buffer.
    */
    for (uint8_t fbidx = 0; fbidx < L_FRAMEBUFFER_NUM; fbidx++)
    {
        bool joined; // l'area é stata fusa con una di quelle giá presenti nella lista
        bool isInside; // l'area é completamente contenuta in una di quelle giá presenti
        gldcoord_Area_t area_add; /* area da aggiungere alla lista */
        gldcoord_Area_t join_area;
        
        gldcoord_AreaCopy (&area_add, area);
        do
        {
            joined = false;
            isInside = false;

            for (int k = 0; k < InvalidAreas[fbidx].Num; k++)
            {
                int16_t cmp_result;

                /* verifico se l'area da aggiungere � inclusa in una delle aree gi�
                   presenti. In questo caso ho terminato. */
                cmp_result = gldcoord_AreaCmp (&InvalidAreas[fbidx].List[k], &area_add);
                if (cmp_result == 0 || cmp_result == 1) { /* 'InvalidAreas[k]' "contiene" oppure "� pari a" 'area' */
                    isInside = true;
                    break;
                }

                /* Calcolo le coordinate della pi� piccola area che contiene
                   'InvalidAreas[k]' e 'area'. */
                gldcoord_AreaJoin (&join_area, &area_add, &InvalidAreas[fbidx].List[k]);

                /* Se l'area risultante dalla "fusione" � vantaggiosa: se ha un'area
                   minore della somma delle aree che l'hanno generata. */
                if (gldcoord_AreaArea (&join_area) < (gldcoord_AreaArea (&InvalidAreas[fbidx].List[k]) + gldcoord_AreaArea (&area_add)))
                { /* Imposto 'area_add' come la "fusione" delle due e mi ricordo
                     di aver joinato ("fuso" due aree in una). */
                    gldcoord_AreaCopy (&area_add, &join_area);
                    joined = true;

                    /* Rimuovo la k-esima area dalla lista delle invalide: � inclusa
                       in 'area_add'. */
                    memmove (&InvalidAreas[fbidx].List[k], &InvalidAreas[fbidx].List[k + 1], (InvalidAreas[fbidx].Num - (k + 1)) * sizeof (gldcoord_Area_t));
                    InvalidAreas[fbidx].Num--;

                    /* Interrompo la spazzolata delle 'InvalidAreas' e la ricomincio
                       con la nuova 'area_add' */
                    break;
                }
            }
        } while (joined == true);
        
        if (isInside == false)
        {
            /* Arrivati a questo punto 'area_add' non pu� fondersi vantaggiosamente con
               nessuna delle 'InvalidAreas'. Inserisco 'area_add' all'ultima posizione
               della lista. */
            if (InvalidAreas[fbidx].Num < GLDCFG_INVALID_AREAS_NUM)
            { /* add the area to the dirty list */
                gldcoord_AreaCopy (&InvalidAreas[fbidx].List[InvalidAreas[fbidx].Num], &area_add);
                InvalidAreas[fbidx].Num++;
            } else
            { /* no free dirty area record */
                /* Unisco quest'area alla prima invalidata e la invalido. Prender� cos�
                   almeno il posto di 'InvalidAreas[0]' */
                gldcoord_AreaJoin (&join_area, &area_add, &InvalidAreas[fbidx].List[0]);
                gldrefr_InvalidateArea (&join_area);
            }
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldrefr_Task (void)
{
    /* quando il frame che sto visualizzando é dirty eseguo un refresh.
    in double buffer cambio frame ogni volta che devo rinfrescare una qualsiasi area
    */
    if (InvalidAreas[VisibleFb].Num != 0)
    {
#if (GLDCFG_DOUBLE_BUFFER == 1)
#if (GLDCFG_LAZY_BACK_BUFFER_COPY == 1)
        CopyValidAreas (VisibleFb, WorkingFb);
        /* le aree invalide del back frame sono ora identiche a quelle del frame visibile */
        memcpy (InvalidAreas[WorkingFb].List, InvalidAreas[VisibleFb].List,
            sizeof (gldcoord_Area_t) * InvalidAreas[VisibleFb].Num);
        InvalidAreas[WorkingFb].Num = InvalidAreas[VisibleFb].Num;
#endif
#endif
        
#if (GLDCFG_HIGHLIGHT_REDRAWN_AREAS == 1 && GLDCFG_DOUBLE_BUFFER == 0)
        /* highlight delle aree rinfrescate */
        RefrInvalidAreas (glddisp_Disp, true);
#endif
        RefrInvalidAreas (glddisp_Disp, false);
        InvalidAreas[WorkingFb].Num = 0;
        
#if (GLDCFG_DOUBLE_BUFFER == 1)
        /* ora il working buffer � completamente aggiornato.
           1) rendo visibile il working buffer.
           2) sincronizzo il back buffer con il working buffer. Copio le aree
              che sono invalide, dal working buffer, al back buffer.
           3) alla fine della procedura il back buffer � una copia esatta del
              working buffer.
        */
        glddisp_Disp->SetVisibleFrame (WorkingFb);
        VisibleFb = WorkingFb;
        WorkingFb = !WorkingFb;

#if (GLDCFG_LAZY_BACK_BUFFER_COPY == 0)
        CopyValidAreas (VisibleFb, WorkingFb);
        /* le aree invalide del back frame sono ora identiche a quelle del frame visibile.
        in questo particolare caso, il frame visibile non ha aree dirty perché
        é appena stato aggiornato.
        */
        InvalidAreas[WorkingFb].Num = 0;
#endif
#endif
    }
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il numero totale di pixel che saranno renderizzati. Uno stesso
        pixel pu� essere conteggiato pi� di una volta se compare in molteplici
        aree.
 Arg: - None.
 Ret: - Dimensione dell'area in pixel.
______________________________________________________________________________*/
int64_t gldrefr_GetTotalInvalidArea (void)
{
    int64_t totalInvalid = 0;

    for (int k = 0; k < InvalidAreas[VisibleFb].Num; k++)
        totalInvalid += gldcoord_AreaArea (&InvalidAreas[VisibleFb].List[k]);
    return totalInvalid;
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________
 Desc:  Rinfresca tutte le aree invalidate del working buffer.
 Arg: - <disp>[in] display driver su cui effettuare le operazioni di drawing.
        <highlightAreas>[in] true per disegnare dei semplici rettangoli
            monocolore nelle aree invalidate. Questo flag viene utilizzato
            soltanto dall'opzione 'GLDCFG_HIGHLIGHT_REDRAWN_AREAS'. Generalmente
            � a 0.
 Ret: - None.
______________________________________________________________________________*/
static void RefrInvalidAreas (gldhaldisp_t *disp, bool highlightAreas)
{
    for (int k = 0; k < InvalidAreas[WorkingFb].Num; k++)
    { /* eseguo il refresh di tutte le aree invalidate */
        gldcoord_Area_t invalid;

        /* clip the invalid area to the display canvas */
        if (gldcoord_AreaIntersection (&invalid, &InvalidAreas[WorkingFb].List[k], &disp->Canvas) == true)
        {
            /* solo se l'area invalidata interseca il display */
#if (GLDCFG_DOUBLE_BUFFER == 1 && GLDCFG_MEMFB_ENABLED == 0)
            
            gldmemfb_SetMemFb (disp, WorkingFb);
            RefrAll (&gldmemfb_Disp, &invalid);
            
#elif (GLDCFG_MEMFB_ENABLED == 1)
            
            gldcoord_Area_t rend_buff;
            void *mem_fb;

            /* ottengo la pi� grande sotto-area di 'invalid' che riesco a
               rappresentare nel buffer di appoggio */
            mem_fb = gldmemfb_GetMemFb (&rend_buff, &invalid);
            while (gldcoord_AreaArea (&rend_buff) > 0)
            { /* disegno gli oggetti sul memory display ottenuto */
                if (highlightAreas) {
                    /* se devo highlightare l'area la riempio semplicemente con
                    un plain color */
                    glddisp_SetClipArea (&gldmemfb_Disp, &rend_buff);
                    glddraw_FillArea (&gldmemfb_Disp, rend_buff.X1, rend_buff.Y1,
                                      rend_buff.X2, rend_buff.Y2, GLDCFG_HIGHLIGHT_REDRAWN_COLOR);
                } else {
                    RefrAll (&gldmemfb_Disp, &rend_buff);
                }
                
#if (GLDCFG_DOUBLE_BUFFER == 1)
                /* copio il contenuto del display virtuale direttamente nel back
                framebuffer */
                gldrect_CopyMemArea_t prm;
                
                prm.SrcCanvas.MemStart = mem_fb;
                gldcoord_AreaCopy (&prm.SrcCanvas.Area, &rend_buff);
                prm.SrcCanvas.Ori = (gldcoord_Ori_e)GLDCFG_MEMFB_ORI;
                prm.SrcCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
                gldrect_SetWidthHeightAsFull (&prm.SrcCanvas);
                
                prm.DstCanvas.MemStart = glddisp_Disp->GetFramePtr (WorkingFb);
                gldcoord_AreaCopy (&prm.DstCanvas.Area, &glddisp_Disp->Canvas);
                prm.DstCanvas.Ori = glddisp_Disp->Ori;
                prm.DstCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
                gldrect_SetWidthHeightAsFull (&prm.DstCanvas);
                prm.DstCanvas.Offset = glddisp_Disp->LineOffset;
                
                prm.AreaSrcPoint.X = rend_buff.X1;
                prm.AreaSrcPoint.Y = rend_buff.Y1;
                prm.AreaDstPoint.X = rend_buff.X1;
                prm.AreaDstPoint.Y = rend_buff.Y1;
                prm.AreaXSize = gldcoord_AreaWidth (&rend_buff);
                prm.AreaYSize = gldcoord_AreaHeight (&rend_buff);
                
                gldrect_CopyMemArea (&prm);
#else
                /* copio il contenuto del display virtuale (residente in memoria)
                sul display vero e proprio */
                disp->FlushArea (mem_fb, rend_buff.X1, rend_buff.Y1, gldcoord_AreaWidth (&rend_buff), gldcoord_AreaHeight (&rend_buff));
#endif
                /* Sottraggo dall'area invalidata, la sua sottoporzione che ho
                appena rinfrescato. Dell'area che mi rimane, ne ottengo un'altra
                porzione e via cos� fino a che non la completo. */
                invalid.Y1 = rend_buff.Y2;
                mem_fb = gldmemfb_GetMemFb (&rend_buff, &invalid);
            }
#else 
            if (highlightAreas) {
                glddisp_SetClipArea (disp, &invalid);
                glddraw_FillArea (disp, invalid.X1, invalid.Y1,
                                  invalid.X2, invalid.Y2, GLDCFG_HIGHLIGHT_REDRAWN_COLOR);
            } else {
                RefrAll (disp, &invalid);
            }
#endif
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Esegue il refresh di tutto ci� che � visibile all'interno dell'area
        specificata. Il refresh viene eseguito utilizzando il display driver
        indicato.
 Arg: - <disp> [in] Display driver con cui portare a termine il refresh.
        <area> [in] Superfice da rappresentare.
 Ret: - None.
______________________________________________________________________________*/
static void RefrAll (gldhaldisp_t *disp, gldcoord_Area_t *area)
{
    /* eseguo il refresh di tutti gli oggetti creati dall'utente */
    RefrObj (disp, gldrefr_RootObj, area);
}

/*______________________________________________________________________________ 
 Desc:  Ridisegna ricorsivamente l'oggetto specificato e tutti i suoi figli. I
        figli risultano in foreground rispetto al padre e la loro visualizzazione
        � limitata all'area del padre. Il tutto viene disegnato solamente
        all'interno dell'area fornita.
 Arg: - <disp> [in] Display driver con cui portare a termine il refresh.
        <obj> [in] Oggetto da disegnare.
        <area> [in] Superfice da rappresentare.
 Ret: - None.
______________________________________________________________________________*/
static void RefrObj (gldhaldisp_t *disp, gldobj_t *obj, gldcoord_Area_t *area)
{
    gldcoord_Area_t area_view;
    if (gldcoord_AreaIntersection (&area_view, &obj->Area, area) && obj->Hide == 0)
    {
        gldllist_Node_t *child;

        glddisp_SetClipArea (disp, &area_view);
        if (obj->FuncDraw != NULL)
            obj->FuncDraw (disp, obj, GLDOBJ_DRAW_MAIN);
        /* eseguo il refresh di tutti i figli da background a foreground.
           L'area di visibilit� dei figli � limitata all'area del padre */

        child = gldllist_GetLast (&obj->ChildList);
        while (child != NULL)
        {
            gldobj_t *childObj;

            childObj = gldllist_Get (child);
            RefrObj (disp, childObj, &area_view);
            child = gldllist_GetPrev (child);
        }
        glddisp_SetClipArea (disp, &area_view);
        if (obj->FuncDraw != NULL)
            obj->FuncDraw (disp, obj, GLDOBJ_DRAW_POST);
    }
}

#if (0)
/*______________________________________________________________________________
 Desc:  Funzione usata solo in double framebuffer.
        Copia le aree del frame di destinazione che risultano dirty, attingendo
        dal frame sorgente. Il frame sorgente deve essere completamente valido.
 Arg: - <scrFrame> numero del frame sorgente.
        <dstFrame> numero frame di destinazione.
 Ret: - None.
______________________________________________________________________________*/
static void CopyValidAreas (uint8_t scrFrame, uint8_t dstFrame)
{
    for (int k = 0; k < InvalidAreas[dstFrame].Num; k++)
    { /* per tutte le aree invalidi del frame di destinazione */
        gldcoord_Area_t invalid;

        /* clip the invalid area to the display canvas */
        if (gldcoord_AreaIntersection (&invalid, &InvalidAreas[dstFrame].List[k], &glddisp_Disp->Canvas) == true)
        { /* eseguo il refresh di tutte le aree invalidate */
            
            gldrect_CopyMemArea_t prm;
            
            prm.SrcCanvas.MemStart = glddisp_Disp->GetFramePtr (scrFrame);
            gldcoord_AreaCopy (&prm.SrcCanvas.Area, &glddisp_Disp->Canvas);
            prm.SrcCanvas.Ori = glddisp_Disp->Ori;
            prm.SrcCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
            gldrect_SetWidthHeightAsFull (&prm.SrcCanvas);
            prm.SrcCanvas.Offset = glddisp_Disp->LineOffset;
            
            prm.DstCanvas.MemStart = glddisp_Disp->GetFramePtr (dstFrame);
            gldcoord_AreaCopy (&prm.DstCanvas.Area, &glddisp_Disp->Canvas);
            prm.DstCanvas.Ori = glddisp_Disp->Ori;
            prm.DstCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
            gldrect_SetWidthHeightAsFull (&prm.DstCanvas);
            prm.DstCanvas.Offset = glddisp_Disp->LineOffset;
            
            prm.AreaSrcPoint.X = invalid.X1;
            prm.AreaSrcPoint.Y = invalid.Y1;
            prm.AreaDstPoint.X = invalid.X1;
            prm.AreaDstPoint.Y = invalid.Y1;
            prm.AreaXSize = gldcoord_AreaWidth (&invalid);
            prm.AreaYSize = gldcoord_AreaHeight (&invalid);
            
            gldrect_CopyMemArea (&prm);
        }
    }
}
#else
/*______________________________________________________________________________
 Desc:  Funzione usata solo in double framebuffer.
        Copia le aree del frame di destinazione che risultano dirty, attingendo
        dal frame sorgente. Il frame sorgente puó anche essere non completamente
        valido. In questo caso, vengono copiate dal buffer sorgente soltanto le
        aree che risultano dirty nel buffer di destinazione, e non dirty nel
        buffer sorgente.
        Al termine della funzione, le aree dirty del frame di destinazione sono
        esattamente pari alle aree dirty del frame sorgente. Quando la funzione
        viene chiamata, le aree dirty del frame sorgente sono un sottoinsieme
        delle aree dirty del frame di destinazione.
 Arg: - <scrFrame> numero del frame sorgente.
        <dstFrame> numero frame di destinazione.
 Ret: - None.
______________________________________________________________________________*/
static void CopyValidAreas (uint8_t scrFrame, uint8_t dstFrame)
{
    for (int k = 0; k < InvalidAreas[dstFrame].Num; k++)
    {   /* ciclo tutte le aree dirty del frame di destinazione */
        gldcoord_Area_t invalid;

        /* clip the dirty area to the display canvas */
        if (gldcoord_AreaIntersection (&invalid, &InvalidAreas[dstFrame].List[k], &glddisp_Disp->Canvas) == true)
        {
            /* lista di aree valide nel frame sorgente e invalide nel frame di destinazione.
            le aree di questa lista, dopo essere state computate, vengono copiate dal
            frame sorgente a quello di destinazione.
            
            tieni presente che il risultato della sottrazione di due aree é una
            lista di aree di dimensione variabile fra 0 e 4.
            di conseguenza la 
            */
            static gldcoord_Area_t ValidArea[30];
            static uint8_t ValidAreaNum;
            gldcoord_Area_t sub[4];
            bool spaceOk = true;
            
            /* se copiassi tutta quest'area <invalid> dal frame sorgente a quello
            di destinazione raggiungerei brutalmente il risultato.
            voglio peró minimizzare il numero di aree copiate.
            allora sottraggo da quest'area tutte le aree che sono dirty nel
            frame sorgente. Copiare queste aree infatti non ha senso, perché non
            hanno un contenuto valido.
            
            parto dall'intera area dirty, e sottraggo mano a mano le aree invalide
            del frame sorgente. Se non sono in grado di fare ulteriori sottrazioni,
            interrompo. Se dovesse succedere, copieró qualche porzione del frame
            sorgente non necessaria (perché dirty), ma non é un problema.
            */
            gldcoord_AreaCopy (&ValidArea[0], &invalid);
            ValidAreaNum = 1;
            for (int v = 0; v < ValidAreaNum && spaceOk; v++)
            {
                for (int i = 0; i < InvalidAreas[scrFrame].Num; i++)
                {
                    /* se le aree non si intersecano non ha senso fare la sottrazione */
                    if (gldcoord_AreaIntersection (NULL, &ValidArea[v], &InvalidAreas[scrFrame].List[i]))
                    {
                        uint8_t num;
                        num = gldcoord_AreaDiff (&ValidArea[v], &InvalidAreas[scrFrame].List[i], sub);
                        
                        if (num == 0) {
                            memmove (&ValidArea[v], &ValidArea[v + 1],
                                sizeof (gldcoord_Area_t) * (ValidAreaNum - v - 1));
                            ValidAreaNum--;
                            /* devo rielaborare questo indice perché adesso contiene
                            un'altra area */
                            v--;
                            break;
                            
                        } else if (num > _NELEMENTS_ (ValidArea) - ValidAreaNum) {
                            /* non ho spazio disponibile per salvare il risultato della
                            sottrazione. mi fermo qua, copieró qualche zona in piú, poco
                            male. */
                            spaceOk = false;
                            break;
                            
                        } else {
                            gldcoord_AreaCopy (&ValidArea[v], &sub[0]);
                            
                            for (int j = 1; j < num; j++) {
                                gldcoord_AreaCopy (&ValidArea[ValidAreaNum], &sub[j]);
                                ValidAreaNum++;
                            }
                        }
                    }
                }
            }
            
            /* a questo punto, l'area iniziale <invalid> é stata scomposta in una
            lista di aree. le aree che fanno parte di questa lista hanno le
            seguenti caratteristiche:
                - sono sottoaree di <invalid>.
                - non si intersecano fra di loro.
                - hanno un contenuto valido nel frame sorgente.
                  Fa eccezzione il caso che si verifica quando esauriamo lo spazio
                  per effettuare le sottrazioni (spaceOk), ma non dovrebbe mai
                  accadere.
            
            allora copio questa lista di aree dal frame sorgente al frame di destinazione
            */
            for (int i = 0; i < ValidAreaNum; i++)
            {
                gldrect_CopyMemArea_t prm;
                
                prm.SrcCanvas.MemStart = glddisp_Disp->GetFramePtr (scrFrame);
                gldcoord_AreaCopy (&prm.SrcCanvas.Area, &glddisp_Disp->Canvas);
                prm.SrcCanvas.Ori = glddisp_Disp->Ori;
                prm.SrcCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
                gldrect_SetWidthHeightAsFull (&prm.SrcCanvas);
                prm.SrcCanvas.Offset = glddisp_Disp->LineOffset;
                
                prm.DstCanvas.MemStart = glddisp_Disp->GetFramePtr (dstFrame);
                gldcoord_AreaCopy (&prm.DstCanvas.Area, &glddisp_Disp->Canvas);
                prm.DstCanvas.Ori = glddisp_Disp->Ori;
                prm.DstCanvas.Bpp = GLDCFG_MEMFB_BYTE_PER_PXL;
                gldrect_SetWidthHeightAsFull (&prm.DstCanvas);
                prm.DstCanvas.Offset = glddisp_Disp->LineOffset;
                
                prm.AreaSrcPoint.X = ValidArea[i].X1;
                prm.AreaSrcPoint.Y = ValidArea[i].Y1;
                prm.AreaDstPoint.X = ValidArea[i].X1;
                prm.AreaDstPoint.Y = ValidArea[i].Y1;
                prm.AreaXSize = gldcoord_AreaWidth (&ValidArea[i]);
                prm.AreaYSize = gldcoord_AreaHeight (&ValidArea[i]);
                
                gldrect_CopyMemArea (&prm);
            }
        }
    }
}
#endif
