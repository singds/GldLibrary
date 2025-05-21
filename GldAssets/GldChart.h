#ifndef GLDCHART_H_INCLUDED
#define GLDCHART_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldAssets/GldAssets.h"

//====================================================================== DEFINES
#define GLDCHART_PICK(obj, index) ((objbutton_t *)gldassets_PickChild (obj, GLDASSETS_TYPE_CHART, index))

#define GLDCHART_DEF(obj_p) \
	GLDOBJ_DEF (obj_p) \
		for (gldchart_t *thisChart = gldassets_CheckType (thisObj, GLDASSETS_TYPE_CHART); thisChart != NULL; thisChart = NULL)

/* questa funzione deve tornare il valore y del punto alla coordinata x. il
   valore restituito deve essere true se il campione è disponibile, false
   altrimenti */
typedef bool (*gldchart_FuncTrace_t) (int32_t *get_y, int32_t ascisse_x, uint32_t x_pxl);

typedef struct
{
	gldobj_t Obj; /* radice dell'oggetto (comune a tutte le tipologie di oggetto) */
	gldobj_t *ObjScalaX;
	gldobj_t *ObjScalaY;
	gldobj_t *ObjCursor;
	gldcoord_t CursorXPos;
	bool DoRefresh;
	
	gldfont_t *Font; /* font */
	
	gldcolor_t ClrCanvas; /* colore di sfondo del grafico */
	
	int32_t MaxOrdinateY; /* massimo valore rappresentato nella scala delle ordinate */
	int32_t MinOrdinateY; /* minimo valore rappresentato nella scala delle ordinate */
	char UdmY[10]; /* unità di misura sull'asse Y */
	void (*PrintX) (char *dest, uint16_t dest_len, int32_t value);
	
	int32_t MaxAscisseX; /* massimo valore delle ascisse visualizzato */
	int32_t MinAscisseX; /* minimo valore delle ascisse visualizzato */
	char UdmX[10]; /* unità di misura sull'asse X */
	void (*PrintY) (char *dest, uint16_t dest_len, int32_t value);
	
	void *Bind;
} gldchart_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
gldobj_t *gldchart_Create (gldobj_t *parent, gldcoord_t x, gldcoord_t y, gldcoord_t x_size, gldcoord_t y_size);
void gldchart_Init (gldchart_t *button, gldobj_t *parent);
__weak void gldchart_Draw (gldhaldisp_t *disp, gldobj_t *obj);

gldobj_t *gldchart_TraceCreate (gldobj_t *parent, gldchart_FuncTrace_t getYbyX, gldcolor_t color);
void gldchart_Task (void);
void gldchart_Refresh (gldobj_t *obj);

#endif /* GLDCHART_H_INCLUDED */
