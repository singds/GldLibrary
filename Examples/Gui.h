#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldCore/GldObj.h"
#include "GldCore/GldDisp.h"
#include "GldCore/GldTouch.h"
#include "GldCore/GldBoost.h"
#include "GldCore/GldDraw.h"

#include "GldAssets/GldActivity.h"
#include "GldAssets/ObjCustom.h"
#include "GldAssets/ObjLabel.h"
#include "GldAssets/ObjButton.h"
#include "GldAssets/ObjImage.h"
#include "GldAssets/ObjNumber.h"
#include "GldAssets/ObjProgressBar.h"

#include "GuiDb.h"
#include "GuiBridge.h"

#include "Font/GuiFont.h"
#include "Image/GuiImg.h"
#include "Text/GuiTxt.h"

#include <cmsis_os2.h>

//====================================================================== DEFINES
#define GUI_X_RES                           (glddisp_Disp->Canvas.X2 - glddisp_Disp->Canvas.X1)
#define GUI_Y_RES                           (glddisp_Disp->Canvas.Y2 - glddisp_Disp->Canvas.Y1)

#define GUI_LINGUA                          0

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
int32_t gui_InitModule (void);

const char *gui_Translate (const char *const *textArray);
const char *gui_TranslateIn (const char *const *textArray, uint8_t lingua);

void *gui_Malloc (size_t size);
void gui_Free (void *ptr);
void *gui_Calloc (size_t nmemb, size_t size);
void *gui_Realloc (void *ptr, size_t size);

#endif /* GUI_H_INCLUDED */

