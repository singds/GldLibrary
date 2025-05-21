#ifndef BKEHALDISP_H_INCLUDED
#define BKEHALDISP_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

#include "GldHal/GldHalDisp.h"

//====================================================================== DEFINES
#define BKEHALDISP_FRAME_BUFFER_ORI__0                    0 /* tested */
#define BKEHALDISP_FRAME_BUFFER_ORI__90                   1 /* tested */
#define BKEHALDISP_FRAME_BUFFER_ORI__180                  2 /* tested */
#define BKEHALDISP_FRAME_BUFFER_ORI__270                  3 /* tested */

//*** <<< Use Configuration Wizard in Context Menu >>> ***
//  <h> Display STM32F7-H7

//   <o> Orientamento
//      <0=> 0      [BKEHALDISP_FRAME_BUFFER_ORI__0]
//      <1=> 90     [BKEHALDISP_FRAME_BUFFER_ORI__90]
//      <2=> 180    [BKEHALDISP_FRAME_BUFFER_ORI__180]
//      <3=> 270    [BKEHALDISP_FRAME_BUFFER_ORI__270]
//   <i> Orientamento del framebuffer
#define BKEHALDISP_FRAME_BUFFER_ORI                      0

//   <o> Framebuffer x resolution [pxl] <0-3840>
//   <i> Width in pixel del frame buffer
#define BKEHALDISP_FRAME_BUFFER_X_RES                    800
//   <o> Framebuffer y resolution [pxl] <0-3840>
//   <i> Height in pixel del frame buffer
#define BKEHALDISP_FRAME_BUFFER_Y_RES                    480

/* decommenta per abilitare l'utilizzo del DMA2D */
//   <e> Dma2d
//   <i> Abilita l'utilizzo del dma2d per accelerare le operazioni copiatura o
//   <i> fill di aree di memoria.
#define BKEHALDISP_ENABLE_DMA2D                          0
//    <o> Min to dma2d fill [pxl] <0-3840>
//    <i> Minima dimensione dell'area da riempire per cui sia conveniente sfruttare
//    <i> l'accelerazione del dma2d
#define BKEHALDISP_DMA2D_FILBKEHALDISP_MIN_AREA          (50)
//    <o> Min to dma2d copy [pxl] <0-3840>
//    <i> Minima dimensione dell'area da copiare per cui sia conveniente sfruttare
//    <i> l'accelerazione del dma2d
#define BKEHALDISP_DMA2D_FLUSH_MIN_AREA                  (50)
//   </e>

//  </h>
//*** <<< end of configuration section >>>    ***

/* frame buffer byte per pixel */
#define BKEHALDISP_BYTE_PER_PXL                          4

//============================================================= GLOBAL VARIABLES
extern gldhaldisp_t bkehaldisp_Display;

//============================================================ GLOBAL PROTOTYPES
int bkehaldisp_Init (void);
const void *bkehaldisp_GetFramebuffer (void);

#endif /* BKEHALDISP_H_INCLUDED */

