#ifndef CMPBTNIMG_H_INCLUDED
#define CMPBTNIMG_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldAssets/GldAssets.h"
#include "GldAssets/GldButton.h"
#include "GldAssets/GldLabel.h"

//====================================================================== DEFINES
#define CMPBTNIMG_DEF(pCmp, pParent) GLDASSETS_DEF (cmpbtnimg_t, pCmp, BcButton.BcObj, pParent)

typedef struct
{
	gldbutton_t BcButton; /* Base class */

	gldlabel_t ObjLabel;
	gldcolor_t ClrLabelRelease;
	gldcolor_t ClrLabelPress;
	const char *ImgRelease;
	const char *ImgPress;
} cmpbtnimg_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
void cmpbtnimg_Init (cmpbtnimg_t *pCmpbtnimg);
cmpbtnimg_t *cmpbtnimg_Create (void);
void cmpbtnimg_Draw (glddraw_Dev_t *disp, gldobj_t *pObj);

void cmpbtnimg_SetImg (cmpbtnimg_t *cmpbtnimg, const char *imgRelease, const char *imgPress);

#endif /* CMPBTNIMG_H_INCLUDED */

