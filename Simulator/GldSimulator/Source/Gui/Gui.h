#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

#include "GldCore/GldObj.h"
#include "GldCore/GldDisp.h"
#include "GldCore/GldTouch.h"
#include "GldCore/GldDraw.h"

#include "GldAssets/GldActivity.h"
#include "GldAssets/GldButton.h"
#include "GldAssets/GldLabel.h"
//#include "GldAssets/GldCustom.h"
//#include "GldAssets/GldImage.h"
//#include "GldAssets/GldListBox.h"
//#include "GldAssets/GldNumber.h"
//#include "GldAssets/GldChart.h"

#include "Font/GuiFont.h"
#include "Image/GuiImg.h"
#include "Text/GuiTxt.h"

#include "GuiDraw.h"
#include "GuiCreate.h"
#include "GuiEvt.h"

#include "Composite/CmpBtnImg.h"

//====================================================================== DEFINES
#define GUI_X_RES                           (glddisp_Disp->Canvas.X2 - glddisp_Disp->Canvas.X1)
#define GUI_Y_RES                           (glddisp_Disp->Canvas.Y2 - glddisp_Disp->Canvas.Y1)

/* Indice della lingua in uso */
#define GUI_LINGUA                          (0)

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GUI_H_INCLUDED */

