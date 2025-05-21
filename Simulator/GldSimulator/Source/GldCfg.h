#ifndef GLDCFG_H_INCLUDED
#define GLDCFG_H_INCLUDED

//===================================================================== INCLUDES

#include <stdlib.h>
#include <stdio.h>
#include "GldCore/Gld.h"

//====================================================================== DEFINES
#define GLDCFG_TXT_ENCODING__ANSI         (0)
#define GLDCFG_TXT_ENCODING__UTF8         (1)

#define GLDCFG_TXT_FONT_FORMAT__LEGACY    (0)
#define GLDCFG_TXT_FONT_FORMAT__NEW       (1)

#define GLDCFG_TXT_EOL_TYPE__LINUX        (0)
#define GLDCFG_TXT_EOL_TYPE__WINDOWS      (1)

#define GLDCFG_MEMFB_ORI__0               (0)
#define GLDCFG_MEMFB_ORI__90              (1)
#define GLDCFG_MEMFB_ORI__180             (2)
#define GLDCFG_MEMFB_ORI__270             (3)

#define GLDCFG_MEMFB_FORMAT__ARGB8888     (0)
#define GLDCFG_MEMFB_FORMAT__RGB888       (1)
#define GLDCFG_MEMFB_FORMAT__RGB565       (2)

//====================================================================== CONFIGS

//*** <<< Use Configuration Wizard in Context Menu >>> ***

// <h> Graphic Lib

//  <h> Text
//   <o> Encoding
//      <0=> ANSI    [GLDCFG_TXT_ENCODING__ANSI]
//      <1=> UTF8    [GLDCFG_TXT_ENCODING__UTF8]
//   <i> Tipo di codifica stringhe di testo
#define GLDCFG_TXT_ENCODING               0
//   <o> Font format
//      <0=> LEGACY  [GLDCFG_TXT_FONT_FORMAT__LEGACY] (deprecated)
//      <1=> NEW     [GLDCFG_TXT_FONT_FORMAT__NEW]
//   <i> Formato dei font utilizzato
#define GLDCFG_TXT_FONT_FORMAT            1
//   <o> End of line
//      <0=> LINUX       [GLDCFG_TXT_EOL_TYPE__LINUX]
//      <1=> WINDOWS     [GLDCFG_TXT_EOL_TYPE__WINDOWS]
//   <i> Sequenza che identifica il termine di una riga
#define GLDCFG_TXT_EOL_TYPE               1
#if (GLDCFG_TXT_EOL_TYPE == GLDCFG_TXT_EOL_TYPE__LINUX)
#define GLDCFG_TXT_EOL                    "\n"
#else
#define GLDCFG_TXT_EOL                    "\r\n"
#endif
//  </h>

//  <h> Touch
//   <o> Debounce time [ms] <0-1000>
#define GLDCFG_TOUCH_DEBOUNCE_TIME        0
//   <o> Long click timeout [ms] <0-5000>
#define GLDCFG_TOUCH_LONG_CLICK_TIME      1000
//   <o> Pixel drag trigger [pxl] <0-20>
//   <i> Devi trascinare il dito sul display almeno di questa quantità di pixel
//   <i> per iniziare un trascinamento.
#define GLDCFG_TOUCH_DRAG_PXL_TRIGGER     6
//   <o> Pixel drag min move [pxl] <0-20>
//   <i> Questo è il minimo spostamento che viene notificato durante un
//   <i> trascinamento
#define GLDCFG_TOUCH_DRAG_PXL_MIN_MOVE    3
//   <e> Mostra tocco
//   <i> Abilita per visualizzare il punto dove stai toccando con il touch.
#define GLDCFG_MOSTRA_TOCCO_ENABLED       1
//    <o> Mostra tocco object size [pxl] <0-1000>
//    <i> Dimensione x e y in pixel dell'oggetto che mostra il punto di tocco.
#define GLDCFG_MOSTRA_TOCCO_SIZE          17
//   </e>
//  </h>

//  <h> Assets
#define GLDCFG_TAG_SIZE                   15
//   <o> (Activity) max nasting [#] <0-1000>
//   <i> Massimo numero di pagine annidate
#define GLDCFG_ACTIVITY_STACK_SIZE        20
//   <o> (Label) buffer size [byte] <0-1000>
//   <i> Massimo numero di caratteri rappresentabili con una label
//#define GLDCFG_LABLE_BYTE_SIZE            50
//   <o> (Number) refresh tick [ms] <0-1000>
//   <i> Intervallo tra un refresh (automatico) e l'altro degli oggetti numero
#define GLDCFG_OBJNUMBER_REFRESH_MS       200
//  </h>

//  <e> Pre-Rendering
#define GLDCFG_MEMFB_ENABLED              1
//   <o> Memory pool address <0x0-0xFFFFFFFF:8>
//   <i> Imposta un valore != 0 se vuoi allocare personalmente la memoria per il
//   <i> framebuffer. Quando == 0 viene allocata staticamente dalla libreria
#define GLDCFG_MEMFB_ADDRESS              0x00000000
//   <o> Buffer size [pixel] <0-100000>
//   <i> Dimensione del framebuffer di rendering in pixel.
#define GLDCFG_MEMFB_PXL_SIZE             13056
//   <o> Orientamento
//      <0=> 0      [GLDCFG_MEMFB_ORI__0]
//      <1=> 90     [GLDCFG_MEMFB_ORI__90]
//      <2=> 180    [GLDCFG_MEMFB_ORI__180]
//      <3=> 270    [GLDCFG_MEMFB_ORI__270]
//   <i> Orientamento del framebuffer
#define GLDCFG_MEMFB_ORI                  0
/* attualmente � supportato solamente un formato colori */
//   <o> Formato colore
//      <0=> ARGB8888    [GLDCFG_MEMFB_FORMAT__ARGB8888] (not yet supported)
//      <1=> RGB888      [GLDCFG_MEMFB_FORMAT__RGB888] (not yet supported)
//      <2=> RGB565      [GLDCFG_MEMFB_FORMAT__RGB565]
#define GLDCFG_MEMFB_FORMAT               2
//  </e>

// </h>

#define GLDCFG_MEMFB_HARDCODED            1

#define GLDCFG_MALLOC                     gld_Malloc
#define GLDCFG_CALLOC                     gld_Calloc
#define GLDCFG_FREE                       gld_Free

#define GLDCFG_PRINTF_PREFIX              "GLD"
#define GLDCFG_PRINTF(format, ...)        printf("%s: "format, GLDCFG_PRINTF_PREFIX, ##__VA_ARGS__)

//*** <<< end of configuration section >>>    ***

//================================================================= CONFIG CHECK
/* inserisci se hai qualche controllo di compatibilità da fare sui parametri di
   configurazione */
#if (GLDCFG_MEMFB_ENABLED != 0)
#if (GLDCFG_MEMFB_FORMAT != GLDCFG_MEMFB_FORMAT__RGB565)
#error "errore: pixel format non supportato"
#endif
#endif

//============================================================ CONFIG DEPENDENCY
/* inderisci qui dipendenze condizionate dai parametri di configurazione */
#if (GLDCFG_MEMFB_FORMAT == GLDCFG_MEMFB_FORMAT__RGB565)
#define GLDCFG_MEMFB_BYTE_PER_PXL         2
#endif

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

#endif /* GLDCFG_H_INCLUDED */
