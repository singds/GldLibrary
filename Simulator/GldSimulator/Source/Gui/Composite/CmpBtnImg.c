//===================================================================== INCLUDES
#include "Composite/CmpBtnImg.h"

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void cmpbtnimg_Init (cmpbtnimg_t *pCmpbtnimg)
{
	if (pCmpbtnimg != NULL)
	{
		/* reset the entire structure */
		memset (pCmpbtnimg, 0, sizeof (cmpbtnimg_t));

		gldbutton_Init (&pCmpbtnimg->BcButton);
		{
			gldbutton_t *pButton = &pCmpbtnimg->BcButton;
			{
				gldobj_t *pObj = &pButton->BcObj;

				pObj->FuncDraw = cmpbtnimg_Draw;
				gldobj_SetSize (pObj, 100, 100);
			}
			pButton->BodyColor = GLDCOLOR_NONE;
			pButton->BorderColor = GLDCOLOR_NONE;
		}
	}
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
cmpbtnimg_t *cmpbtnimg_Create (void)
{
	cmpbtnimg_t *pCmpbtnimg = GLDCFG_CALLOC (1, sizeof (cmpbtnimg_t));
	cmpbtnimg_Init (pCmpbtnimg);
	return pCmpbtnimg;
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void cmpbtnimg_Draw (glddraw_Dev_t *disp, gldobj_t *pObj)
{
	cmpbtnimg_t *pCmpbtnimg = (cmpbtnimg_t *)pObj;
	bool press = pCmpbtnimg->BcButton.StatePress;

	gldbutton_Draw (disp, &pCmpbtnimg->BcButton.BcObj);
	glddraw_FsBmp (disp, pObj->Area.X1, pObj->Area.Y1, GLDCOORD_ALIGN_LT, press ? pCmpbtnimg->ImgPress : pCmpbtnimg->ImgRelease, NULL);
}

/*______________________________________________________________________________
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void cmpbtnimg_SetImg (cmpbtnimg_t *cmpbtnimg, const char *imgRelease, const char *imgPress)
{
	if (cmpbtnimg != NULL)
	{
		cmpbtnimg->ImgRelease = imgRelease;
		cmpbtnimg->ImgPress = imgPress;
	}
}

//============================================================ PRIVATE FUNCTIONS
