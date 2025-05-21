//===================================================================== INCLUDES
#include "GldAssets/GldChart.h"

#include <math.h>
#include "GldAssets/GldButton.h"
#include "GldAssets/GldCustom.h"
#include "GldCore/GldTouch.h"

//====================================================================== DEFINES
#define L_ORDINATE_MIN_DIV                          6
#define L_ASCISSE_MIN_DIV                           6

//=========================================================== PRIVATE PROTOTYPES
void DoChartTask (gldobj_t *obj_p);
static void DrawOrdinateY (gldhaldisp_t *disp, gldchart_t *chart);
static void DrawAscisseX (gldhaldisp_t *disp, gldchart_t *chart);
static void FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext);
static void FuncDestroy (gldobj_t *obj);
static void FuncDrawTrace (gldhaldisp_t *disp, gldobj_t *obj);

static void DrawScalaOrdinateY (gldhaldisp_t *disp, gldobj_t *obj);
static void DrawScalaAscisseX (gldhaldisp_t *disp, gldobj_t *obj);

static uint32_t GetPxlOrdinateY (gldchart_t *chart, int32_t value);
static uint32_t GetPxlAscisseX (gldchart_t *chart, int32_t value);

void PrintAsItIs (char *dest, uint16_t dest_len, int32_t value);

static void DrawCursor (gldhaldisp_t *disp, gldobj_t *obj);
static void RefreshCursor (gldchart_t *chart);

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
gldobj_t *gldchart_Create (gldobj_t *parent, gldcoord_t x, gldcoord_t y, gldcoord_t x_size, gldcoord_t y_size)
{
	gldchart_t *chart = GLDCFG_CALLOC (1, sizeof (gldchart_t));
	if (chart != NULL)
	{	
		gldchart_Init (chart, parent);	
		gldobj_InitArea (&chart->Obj, x, y, x_size, y_size);
		chart->DoRefresh = true;
		
		GLDBUTTON_DEF (chart->ObjScalaY = gldbutton_Create (parent, (x + x_size), y, 30, y_size))
		{	/* creo la scala delle ordinate come un pulsante */
			thisObj->FuncDraw = DrawScalaOrdinateY;
			thisButton->Bind = chart; /* bind the button with the chart */
		}
		GLDBUTTON_DEF (chart->ObjScalaX = gldbutton_Create (parent, x, (y + y_size), x_size, 30))
		{	/* creo la scala delle ascisse come un pulsante */
			thisObj->FuncDraw = DrawScalaAscisseX;
			thisButton->Bind = chart; /* bind the button with the chart */
		}
		
		GLDCUSTOM_DEF (chart->ObjCursor = gldcustom_Create (&chart->Obj, 0, 0, 5, 5))
		{	/* questo oggetto rappresenta un piccolo cursore rosso sulla curva del
			   grafico, utilizzabile per identificare un particolare punto */
			thisObj->FuncDraw = DrawCursor;
			thisObj->Hide = true;
			thisCustom->Bind = chart;
		}
		
		gldobj_Invalidate (&chart->Obj);
	}
	
	return &chart->Obj;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldchart_Init (gldchart_t *chart, gldobj_t *parent)
{
	/* Set default values */
	chart->Type = GLDASSETS_TYPE_CHART;
	chart->ClrCanvas = GLDCOLOR_WHITE;
	chart->PrintX = PrintAsItIs;
	chart->PrintY = PrintAsItIs;
	
	/* Init the generic data */
	gldobj_Init (&chart->Obj, parent);
	chart->Obj.Priv = chart; /* bind fra scheletro oggetto e oggetto completo */
	chart->Obj.Click = true;
	
	chart->Obj.FuncEvent = FuncEvent;
	chart->Obj.FuncDraw = gldchart_Draw;
    chart->Obj.FuncDestroy = FuncDestroy;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
__weak void gldchart_Draw (gldhaldisp_t *disp, gldobj_t *obj)
{
	gldchart_t *chart;
	if ((chart = gldassets_CheckType (obj, GLDASSETS_TYPE_CHART)) != NULL)
	{	/* perform the object drawing */
		gldcoord_t x0, y0, x1, y1;
		
		x0 = obj->Area.X1;
		y0 = obj->Area.Y1;
		x1 = obj->Area.X2;
		y1 = obj->Area.Y2;
		
		glddraw_FillArea (disp, x0, y0, x1, y1, chart->ClrCanvas);
		/* rappresento le scale del grafico */
		DrawOrdinateY (disp, chart);
		DrawAscisseX (disp, chart);
		/* rappresento la linea del grafico */
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
gldobj_t *gldchart_TraceCreate (gldobj_t *parent, gldchart_FuncTrace_t getYbyX, gldcolor_t color)
{
	/* the parent must be a chart */
	gldobj_t *obj_p = NULL;
	gldchart_t *chart = gldassets_CheckType (parent, GLDASSETS_TYPE_CHART);
	
	if (chart != NULL)
	{
		GLDCUSTOM_DEF (obj_p = gldcustom_Create (parent, 0, 0, gldcoord_AreaWidth (&parent->Area), gldcoord_AreaHeight (&parent->Area)))
		{
			thisObj->FuncDraw = FuncDrawTrace;
			thisObj->UsrData[0] = color;
			thisObj->UsrPointer = getYbyX;
			thisCustom->Bind = chart; /* bind with the chart structure */
		}
	}
	
	return obj_p;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldchart_Task (void)
{
	if (gldrefr_RootObj != NULL)
	{
		DoChartTask (gldrefr_RootObj);
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void gldchart_Refresh (gldobj_t *obj)
{
	gldchart_t *chart = gldassets_CheckType (obj, GLDASSETS_TYPE_CHART);
	
	if (chart != NULL)
	{
		gldobj_Invalidate (obj);
		gldobj_Invalidate (chart->ObjScalaX);
		gldobj_Invalidate (chart->ObjScalaY);
		chart->DoRefresh = true;
	}
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void FuncEvent (gldobj_t *obj, gldobj_Event_e evt, void *ext)
{
	gldchart_t *chart = gldassets_CheckType (obj, GLDASSETS_TYPE_CHART);
	
	if (evt == GLDOBJ_EVENT_PRESS)
	{	
		gldcoord_Point_t touchPoint;
		
		/* ottengo le coordinate del punto di pressione e le assegno al cursore */
		gldtouch_GetCoords (&touchPoint);
		chart->CursorXPos = obj->Area.X2 - touchPoint.X;
		chart->DoRefresh = true;
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void FuncDestroy (gldobj_t *obj)
{
    gldbutton_t *chart = gldassets_CheckType (obj, GLDASSETS_TYPE_CHART);
    
    GLDCFG_ASSERT (chart);
    GLDCFG_FREE (chart);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DrawOrdinateY (gldhaldisp_t *disp, gldchart_t *chart)
{
	/* Rappresentazione scala e griglia ordinate */
	uint32_t exp = 0;
	int32_t min_division, lineValueSpace, lineValue;
	gldcoord_t x0, x1, y1;
	
	x0 = chart->Obj.Area.X1;
	x1 = chart->Obj.Area.X2;
	y1 = chart->Obj.Area.Y2;
	
	min_division = (chart->MaxOrdinateY - chart->MinOrdinateY) / L_ORDINATE_MIN_DIV;
	
	while (min_division / (uint32_t)pow (10, exp) != 0)
		exp++;
	
	lineValueSpace = min_division / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
	lineValue = chart->MinOrdinateY / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
	
	if (lineValue != chart->MinOrdinateY)
		lineValue += lineValueSpace;
	
	while (lineValue < chart->MaxOrdinateY)
	{
		glddraw_Line (disp, x0, y1 - GetPxlOrdinateY (chart, lineValue), x1, y1 - GetPxlOrdinateY (chart, lineValue), 0xa0a0a0);
		lineValue += lineValueSpace;
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DrawAscisseX (gldhaldisp_t *disp, gldchart_t *chart)
{
	/* Rappresentazione scala e griglia ordinate */
	uint32_t exp = 0;
	int32_t min_division, lineValueSpace, lineValue;
	gldcoord_t y0, x1, y1;
	
	y0 = chart->Obj.Area.Y1;
	x1 = chart->Obj.Area.X2;
	y1 = chart->Obj.Area.Y2;
	
	min_division = (chart->MaxAscisseX - chart->MinAscisseX) / L_ASCISSE_MIN_DIV;
	
	while (min_division / (uint32_t)pow (10, exp) != 0)
		exp++;
	
	lineValueSpace = min_division / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
	lineValue = chart->MinAscisseX / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
	
	if (lineValue != chart->MinAscisseX)
		lineValue += lineValueSpace;
	
	while (lineValue < chart->MaxAscisseX)
	{
		glddraw_Line (disp, x1 - GetPxlAscisseX (chart, lineValue), y0, x1 - GetPxlAscisseX (chart, lineValue), y1, 0xa0a0a0);
		lineValue += lineValueSpace;
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static uint32_t GetPxlOrdinateY (gldchart_t *chart, int32_t value)
{
	uint32_t pixels;
	gldcoord_t y_size;
	
	y_size = gldcoord_AreaHeight (&chart->Obj.Area);
	
	pixels = _MAX_ (0, value - chart->MinOrdinateY);
	pixels = (pixels * y_size) / (chart->MaxOrdinateY - chart->MinOrdinateY);
	
	return pixels;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static uint32_t GetPxlAscisseX (gldchart_t *chart, int32_t value)
{
	uint32_t pixels;
	gldcoord_t x_size;
	
	x_size = gldcoord_AreaWidth (&chart->Obj.Area);
	
	pixels = _MAX_ (0, value - chart->MinAscisseX);
	pixels = (pixels * x_size) / (chart->MaxAscisseX - chart->MinAscisseX);
	
	return pixels;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void FuncDrawTrace (gldhaldisp_t *disp, gldobj_t *obj)
{
	gldcustom_t *custom = gldassets_CheckType (obj, GLDASSETS_TYPE_CUSTOM);
	if (custom != NULL)
	{
		gldchart_t *chart = custom->Bind;
		gldcolor_t traceColor = obj->UsrData[0];
		gldchart_FuncTrace_t traceFunc = (gldchart_FuncTrace_t)obj->UsrPointer;
		
		if (traceFunc != NULL)
		{
			gldcoord_t x1, y1, x_size;
			gldcoord_t lastPointXpxl, lastPointYpxl;
			
			x1 = chart->Obj.Area.X2;
			y1 = chart->Obj.Area.Y2;
			x_size = gldcoord_AreaWidth (&chart->Obj.Area);
			
			/* Rappresentazione la linea del grafico */
			bool doInitLastPoint = true;
			for (int x = 0; x < x_size; x++)
			{	/* ciclo tutti i pixel lungo l'asse x del grafico e per ciascuno
				   calcolo le coordinata del grafico in quel punto */
				gldcoord_t y, pointXpxl, pointYpxl;
				int32_t x_coord, y_coord;
				
				/* calcolo il valore della coordinata 'x_coord' in corrispondenza del
				   pixel 'x' */
				x_coord = (x * (chart->MaxAscisseX - chart->MinAscisseX)) / x_size + chart->MinAscisseX;
				/* ottengo la coordinata 'y_coord' corrispondente a 'x_coord' dalla
				   funzione caratteristica del grafico. */
				if (traceFunc (&y_coord, x_coord, x) == true)
				{
					y = GetPxlOrdinateY (chart, y_coord);
				
					pointXpxl = x1 - x;
					pointYpxl = y1 - y;
					if (doInitLastPoint)
					{	/* inizializza la posizione dell'ultimo punto rappresentato */
						doInitLastPoint = false;
						lastPointXpxl = pointXpxl;
						lastPointYpxl = pointYpxl;
					}
					glddraw_Line (disp, lastPointXpxl, lastPointYpxl, pointXpxl, pointYpxl, traceColor);
					lastPointXpxl = pointXpxl;
					lastPointYpxl = pointYpxl;
					/* salvo la posizione del punto precedente per creare un
					   collegamenteo grafico con il successivo */
				}
				else
				{
					doInitLastPoint = true;
					/* evito che nelle porzioni di x dove non sono disponibili dati
					   sia presente una line di congiunzione */
				}
			}
		}
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DrawScalaOrdinateY (gldhaldisp_t *disp, gldobj_t *obj)
{
	gldcoord_Point_t arrow[3];
	gldbutton_t *chart = ((gldbutton_t *)obj->Priv)->Bind;
	
	glddraw_Line (disp, obj->Area.X1, chart->Obj.Area.Y1, obj->Area.X1, chart->Obj.Area.Y2, GLDCOLOR_BLACK);
	/* rappresento una piccol freccia che definisce la direzione dell'asse */
	arrow[0].X = obj->Area.X1;
	arrow[0].Y = chart->Obj.Area.Y1 - 1;
	arrow[1].X = obj->Area.X1 + 5;
	arrow[1].Y = chart->Obj.Area.Y1 + 10;
	arrow[2].X = obj->Area.X1 - 5;
	arrow[2].Y = chart->Obj.Area.Y1 + 10;
	glddraw_Triangle (disp, arrow, GLDCOLOR_BLACK);
	
	{	/* da rimpiazzare con una soluzione migliore (totto questo segmento) */
		uint32_t exp = 0;
		int32_t min_division, lineValueSpace, lineValue;
		gldchart_t *chart = (gldchart_t *)(((gldbutton_t *)(obj->Priv))->Bind);
		
		min_division = (chart->MaxOrdinateY - chart->MinOrdinateY) / L_ORDINATE_MIN_DIV;
		
		while (min_division / (uint32_t)pow (10, exp) != 0)
			exp++;
		
		lineValueSpace = min_division / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
		lineValue = chart->MinOrdinateY / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
		
		if (lineValue != chart->MinOrdinateY)
			lineValue += lineValueSpace;
		
		while (lineValue < chart->MaxOrdinateY)
		{
			char printed[20];
			/* richiedo la stampa in caratteri di questo valore */
			chart->PrintY (printed, sizeof (printed), lineValue);
			glddraw_Printf (disp, obj->Area.X1 + 2, chart->Obj.Area.Y2 - GetPxlOrdinateY (chart, lineValue), GLDCOORD_ALIGN_LM, chart->Font, GLDCOLOR_BLACK, GLDCOLOR_NONE, "%s", printed); 
			lineValue += lineValueSpace;
		}
		
		/* scrivo l'unit� di misura di questo asse */
		glddraw_Printf (disp, obj->Area.X2 - 2, chart->Obj.Area.Y1 + 2, GLDCOORD_ALIGN_RT, chart->Font, GLDCOLOR_BLACK, GLDCOLOR_NONE, "%s", chart->UdmY);
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DrawScalaAscisseX (gldhaldisp_t *disp, gldobj_t *obj)
{
	gldcoord_Point_t arrow[3];
	gldbutton_t *chart = ((gldbutton_t *)obj->Priv)->Bind;
	
	glddraw_Line (disp, chart->Obj.Area.X1, obj->Area.Y1, chart->Obj.Area.X2, obj->Area.Y1, GLDCOLOR_BLACK);
	/* rappresento una piccol freccia che definisce la direzione dell'asse */
	arrow[0].X = chart->Obj.Area.X1;
	arrow[0].Y = obj->Area.Y1;
	arrow[1].X = chart->Obj.Area.X1 + 10;
	arrow[1].Y = obj->Area.Y1 + 5;
	arrow[2].X = chart->Obj.Area.X1 + 10;
	arrow[2].Y = obj->Area.Y1 - 5;
	glddraw_Triangle (disp, arrow, GLDCOLOR_BLACK);
	
	{	/* da rimpiazzare con una soluzione migliore (totto questo segmento) */
		uint32_t exp = 0;
		int32_t min_division, lineValueSpace, lineValue;
		gldchart_t *chart = (gldchart_t *)(((gldbutton_t *)(obj->Priv))->Bind);
		
		min_division = (chart->MaxAscisseX - chart->MinAscisseX) / L_ASCISSE_MIN_DIV;
		
		while (min_division / (uint32_t)pow (10, exp) != 0)
			exp++;
		
		lineValueSpace = min_division / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
		lineValue = chart->MinAscisseX / (int32_t)pow(10, exp - 1) * (int32_t)pow(10, exp - 1);
		
		if (lineValue != chart->MinAscisseX)
			lineValue += lineValueSpace;
		
		while (lineValue < chart->MaxAscisseX)
		{
			char printed[20];
			/* richiedo la stampa in caratteri di questo valore */
			chart->PrintX (printed, sizeof (printed), lineValue);
			glddraw_Printf (disp, chart->Obj.Area.X2 - GetPxlAscisseX (chart, lineValue), obj->Area.Y1 + 2, GLDCOORD_ALIGN_MT, chart->Font, GLDCOLOR_BLACK, GLDCOLOR_NONE, "%s", printed); 
			lineValue += lineValueSpace;
		}
		
		/* scrivo l'unit� di misura di questo asse */
		glddraw_Printf (disp, chart->Obj.Area.X1 + 2, obj->Area.Y2 - 2, GLDCOORD_ALIGN_LB, chart->Font, GLDCOLOR_BLACK, GLDCOLOR_NONE, "%s", chart->UdmX);
	}
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void PrintAsItIs (char *dest, uint16_t dest_len, int32_t value)
{
	snprintf (dest, dest_len, "%d", value);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void DrawCursor (gldhaldisp_t *disp, gldobj_t *obj)
{
	gldcoord_t radius = gldcoord_AreaWidth (&obj->Area) / 2;
	glddraw_FillCircle (disp, obj->Area.X1 + radius, obj->Area.Y1 + radius, radius, GLDCOLOR_RED);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
static void RefreshCursor (gldchart_t *chart)
{
	gldcoord_t x_pxl, y_pxl;
	int32_t x_coord, y_coord;
	gldobj_t *firstTrace;
	gldchart_FuncTrace_t traceFunc = NULL;
	
	/* il cursore � disposto a cavallo del primo tracciato. Eventuali altri
	   tracciati non possono essere raggiunti e ispezionati dal cursore */
	if ((firstTrace = gldobj_GetChild (&chart->Obj, 1)) != NULL)
		traceFunc = (gldchart_FuncTrace_t)firstTrace->UsrPointer;
	
	x_pxl = chart->CursorXPos;
	/* calcolo il valore della coordinata 'x_coord' in corrispondenza del
	   pixel 'x' */
	x_coord = (x_pxl * (chart->MaxAscisseX - chart->MinAscisseX)) / gldcoord_AreaWidth (&chart->Obj.Area) + chart->MinAscisseX;
	/* ottengo la coordinata 'y_coord' corrispondente a 'x_coord' dalla
	   funzione caratteristica del grafico. */
	if ((traceFunc != NULL) && (traceFunc (&y_coord, x_coord, x_pxl) == true))
	{
		y_pxl = GetPxlOrdinateY (chart, y_coord);
		x_pxl = chart->Obj.Area.X2 - x_pxl;
		x_pxl -= gldcoord_AreaWidth (&chart->ObjCursor->Area) / 2;
		y_pxl = chart->Obj.Area.Y2 - y_pxl;
		y_pxl -= gldcoord_AreaHeight (&chart->ObjCursor->Area) / 2;
		
		gldobj_SetPos (chart->ObjCursor, x_pxl, y_pxl);
		gldobj_SetVisible (chart->ObjCursor, true);
	}
	else
	{
		gldobj_SetVisible (chart->ObjCursor, false);
	}
}

/*______________________________________________________________________________ 
 Desc:  Verifica se 'obj_p', e in modo ricorsivo su tutti i suoi figli, � un
        oggetto di tipo numero. In questo caso aggiorna la label associata
        stampandoci dentro il valore corrente del numero.
 Arg: - <obj_p>[in] puntatore ad oggetto radice.
 Ret: - None.
______________________________________________________________________________*/
void DoChartTask (gldobj_t *obj_p)
{
	gldchart_t *chart;
	gldllist_Node_t *node;
	
	/* verifico se questo oggetto e un tipo numero */
	if (((chart = gldassets_CheckType (obj_p, GLDASSETS_TYPE_CHART)) != NULL))
	{	/* questo oggetto � un grafico */
		if (chart->DoRefresh)
		{
			chart->DoRefresh = false;
			RefreshCursor (chart);
		}
	}
	
	/* do the same on the child */
	node = gldllist_GetFirst (&obj_p->ChildList);
	while (node)
	{
		gldobj_t *child = gldllist_Get (node);
		DoChartTask (child);
		node = gldllist_GetNext (node);
	}
}

