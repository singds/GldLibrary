//===================================================================== INCLUDES
#include "BkeHalTouch.h"

#include "GldSimulator.h"

//====================================================================== DEFINES
//*** <<< Use Configuration Wizard in Context Menu >>> ***
//  <h> Touch FT5336

// <c1> Enable X Flip
// <i> Inverti le coordinate dell'asse x (come restituito dal touch).
#define L_X_FLIP
// </c>
// <c1> Enable Y Flip
// <i> Inverti le coordinate dell'asse y (come restituito dal touch).
//#define L_Y_FLIP
// </c>
// <c1> Enable X Y Swap
// <i> Inverti l'asse x con l'asse y.
#define L_XY_SWAP
// </c>

//  </h>
//*** <<< end of configuration section >>>    ***

//=========================================================== PRIVATE PROTOTYPES
static bool BkeGetTouch (int16_t *x, int16_t *y);

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES
gldhaltouch_t bkehaltouch_Touch =
{
	.GetTouch = BkeGetTouch,
};

//============================================================= GLOBAL FUNCTIONS

//============================================================ PRIVATE FUNCTIONS
static bool BkeGetTouch (int16_t *x, int16_t *y)
{
	bool touch = false;

	touch = gldsimulator_GetTouch (x, y);
	return touch;
}
