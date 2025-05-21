//===================================================================== INCLUDES
#include "BkeHalTouch.h"

#include "FT5336.h"

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
	uint16_t x_pos, y_pos;
	
	if (ft5336_RilevaNumTouchate ( ) == 1)
	{
		touch = true;
		ft5336_GetXYTouchata (&x_pos, &y_pos);
		/* una lettura ogni tanto torna dei valori fuori scala (superiori a 480
		   o 272). Non � un problema ma sarebbe il caso di controllarlo.
		   Ad ogni modo li limito per assicurarne la consistenza. */
		x_pos = _MIN_ (480, x_pos);
		x_pos = _MAX_ (0, x_pos);
		y_pos = _MIN_ (272, y_pos);
		y_pos = _MAX_ (0, y_pos);
		
		#ifdef L_X_FLIP /* ribalto l'asse x */
		x_pos = 480 - x_pos;
		#endif
		
		#ifdef L_Y_FLIP /* ribalto l'asse y */
		y_pos = 272 - y_pos;
		#endif
		
		#ifdef L_XY_SWAP /* inverto x con y */
		*x = y_pos;
		*y = x_pos;
		#else
		*x = x_pos;
		*y = y_pos;
		#endif
	}
	
	return touch;
}
