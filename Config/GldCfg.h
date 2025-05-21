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
#define GLDCFG_TXT_FONT_FORMAT__PERFECT   (2)

#define GLDCFG_TXT_EOL_TYPE__LINUX        (0)
#define GLDCFG_TXT_EOL_TYPE__WINDOWS      (1)

#define GLDCFG_MEMFB_ORI__0               (0)
#define GLDCFG_MEMFB_ORI__90              (1)
#define GLDCFG_MEMFB_ORI__180             (2)
#define GLDCFG_MEMFB_ORI__270             (3)

//====================================================================== CONFIGS

//*** <<< Use Configuration Wizard in Context Menu >>> ***

// <h> Graphic Lib

//  <h> Text
//   <o> Encoding
//      <0=> ANSI    [GLDCFG_TXT_ENCODING__ANSI]
//      <1=> UTF8    [GLDCFG_TXT_ENCODING__UTF8]
//   <i> Tipo di codifica stringhe di testo
#define GLDCFG_TXT_ENCODING               1
//   <o> Font format
//      <0=> LEGACY  [GLDCFG_TXT_FONT_FORMAT__LEGACY] (deprecated)
//      <1=> NEW     [GLDCFG_TXT_FONT_FORMAT__NEW]
//      <2=> NEW     [GLDCFG_TXT_FONT_FORMAT__PERFECT]
//   <i> Formato dei font utilizzato
#define GLDCFG_TXT_FONT_FORMAT            2
//   <o> End of line
//      <0=> LINUX       [GLDCFG_TXT_EOL_TYPE__LINUX]
//      <1=> WINDOWS     [GLDCFG_TXT_EOL_TYPE__WINDOWS]
//   <i> Sequenza che identifica il termine di una riga
#define GLDCFG_TXT_EOL_TYPE               0
#if (GLDCFG_TXT_EOL_TYPE == GLDCFG_TXT_EOL_TYPE__LINUX)
#define GLDCFG_TXT_EOL                    "\n"
#else
#define GLDCFG_TXT_EOL                    "\r\n"
#endif
//  </h>

//  <h> Touch
//   <o> Debounce time [ms] <0-1000>
#define GLDCFG_TOUCH_DEBOUNCE_TIME        20
//   <o> Touch area expand [pxl] <0-100>
//   <i> Dopo aver premuto un'oggetto, espandi la sua area tattile di questo
//   <i> numero di pixel in tutte le direzioni, fino a quando non lo rilasci.
#define GLDCFG_TOUCH_AREA_EXPAND          10
//   <o> Pixel drag trigger [pxl] <0-20>
//   <i> Devi trascinare il dito sul display almeno di questa quantità di pixel
//   <i> per iniziare un trascinamento.
#define GLDCFG_TOUCH_DRAG_PXL_TRIGGER     8
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
//    <h> Activity
//      <o> Stack size [byte] <0-10000>
//      <i> Dimensione dello stack delle activity
//      <i> Default 512
#define GLDCFG_ACTIVITY_STACK_SIZE        512
//    </h>
//    <h> ObjNumber
//      <o> Refresh tick [ms] <0-1000>
//      <i> Intervallo tra un refresh (automatico) e l'altro degli oggetti numero
//    </h>
#define GLDCFG_OBJNUMBER_REFRESH_MS       200
//  </h>

//  <h> Rendering
//   <o> Max invalid areas num [#] <0-100>
//   <i> Dimensione dell'array di aree invalide.
#define GLDCFG_INVALID_AREAS_NUM          40
//   <e> Highlight redrawn areas
//   <i> Se abiliti questa funzione ogni volta che viene ridisegnata una porzione
//   <i> del display viene mostrato un blocco monocolore su quell'area.
//   <i> Utile per trovare bug che causano frequenti refresh.
#define GLDCFG_HIGHLIGHT_REDRAWN_AREAS    0
//    <o> Highlight color [rgb888 color] <0x0-0xFFFFFF:6>
//    <i> Colore dell'highlight
#define GLDCFG_HIGHLIGHT_REDRAWN_COLOR    0xb427fb
//   </e>
//   <o> Framebuffer byte per pixel
//   <i> Il numero di byte che occupa ciascun pixel del framebuffer.
//   <i> Questa impostazione ha effetto solamente se é abilitata l'opzione
//   <i> GLDCFG_DOUBLE_BUFFER oppure l'opzione GLDCFG_MEMFB_ENABLED.
//   <i> Questo valore è legato anche alle funzioni gldcfg_WritePxl e gldcfg_ReadPxl,
//   <i> vedere anche quelle.
//      <1=> 1 Byte/pxl
//      <2=> 2 Byte/pxl
//      <3=> 3 Byte/pxl
//      <4=> 4 Byte/pxl
#define GLDCFG_MEMFB_BYTE_PER_PXL         4
//   <e> Double buffer
//   <i> Il rendering viene effettuato direttamente sulla memoria dei framebuffer
//   <i> se il pre-rendering e' disabilitato. Altrimenti viene usato il buffer di
//   <i> pre-rendering per copiarlo poi sul framebuffer.
//   <i> Se la memoira dove risiedono i framebuffer e' particolarmente lenta,
//   <i> un piccolo buffer di pre-rendering in memoria veloce, potrebbe
//   <i> incrementare di molto le prestazioni. Per questo l'opzione double buffer
//   <i> non esclude la possibilità di abilitare anche il pre-rendering.
#define GLDCFG_DOUBLE_BUFFER              1
//    <e> Lazy back buffer copy
//    <i> Con questa impostazione modifiche la politica con cui il buffer visibile
//    <i> e il back buffer vengono sincronizzati.
//    <i> Di default appena il nuovo buffer visibile viene aggiornato e mostrato,
//    <i> il contenuto del back buffer viene subito sincronizzato.
//    <i> Se abiliti questa opzione il back buffer viene aggiornato prima della fase
//    <i> di rendering. questo permette di minimizzare lo operazioni di copiatura
//    <i> fra i due buffer, migliorando la velocitá delle animazioni e dei cambio pagina.
#define GLDCFG_LAZY_BACK_BUFFER_COPY      1
//    </e>
//   </e>
//   <e> Rendering speed optimization
//   <i> Quando possibile bypassa il driver lcd per renderizzare direttamente
//   <i> nel buffer di pre-rendering.
//   <i> * -50% tempo di rendering immagini interne.
//   <i> * -30% tempo di rendering immagini su file system (SDCard).
#define GLDCFG_DRAW_OPTIMIZATION          1
//   </e>
//   <e> Pre-Rendering
#define GLDCFG_MEMFB_ENABLED              0
//    <o> Memory pool address <0x0-0xFFFFFFFF:8>
//    <i> Imposta un valore != 0 se vuoi allocare personalmente la memoria per il
//    <i> framebuffer. Quando == 0 viene allocata staticamente dalla libreria
#define GLDCFG_MEMFB_ADDRESS              0xA0400000
//    <o> Buffer size [pixel] <0-1000000>
//    <i> Dimensione del framebuffer di rendering in pixel.
//    <i> Se pensi di abilitare la cache su questo buffer, probabilemnte vorrai
//    <i> assicurarti che sia allineato alla dimensione di una linea di cache,
//    <i> e che abbia dimensione multiplo di una linea di cache. Su cortex-M7
//    <i> una linea di cache è 32byte.
#define GLDCFG_MEMFB_PXL_SIZE             100000
//    <o> Orientamento
//       <0=> 0      [GLDCFG_MEMFB_ORI__0]
//       <1=> 90     [GLDCFG_MEMFB_ORI__90]
//       <2=> 180    [GLDCFG_MEMFB_ORI__180]
//       <3=> 270    [GLDCFG_MEMFB_ORI__270]
//    <i> Orientamento del framebuffer
#define GLDCFG_MEMFB_ORI                  0
//   </e>
//  </h>

// </h>

#define GLDCFG_MALLOC                     gld_Malloc
#define GLDCFG_CALLOC                     gld_Calloc
#define GLDCFG_REALLOC                    gld_Realloc
#define GLDCFG_FREE                       gld_Free

#define GLDCFG_PRINTF_PREFIX              "GLD"
#define GLDCFG_PRINTF(format, ...)        //printf("%s: "format, GLDCFG_PRINTF_PREFIX, ##__VA_ARGS__)

#define GLDCFG_ASSERT(exp)                { if ((exp) == 0) __breakpoint (0); }

//*** <<< end of configuration section >>>    ***

#include "GldMisc/GldColor.h"

//================================================================= CONFIG CHECK

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

/*______________________________________________________________________________ 
Le funzioni che seguono sono utilizzate solamente quando e' attivo il
prerendering oppure il double buffer. Se non hai attivato nessuna di queste
funzioni, puoi lasciare <gldcfg_WritePxl> e <gldcfg_ReadPxl> con un empty body.

La libreria supporta solamente framebuffer con pixel da 1-2-3 o 4 byte.
La libreria non gestisce in nessun modo l'arrangiamento dei colori all'interno
di un singolo pixel (es; RGB o BRG). Ti viene chiesto solamente di specificare
il numero di byte che occupa ciascun pixel, questo viene usato internamente
per esempio per avanzare da un pixel al successivo.
Sei tu, con le funzioni sottostanti, a dover arrangiare correttamente i campi
colore all'interno di un pixel.
______________________________________________________________________________*/

/*______________________________________________________________________________ 
 Desc:  Scrivi un pixel nel framebuffer.
 Arg: - <pos>[in] Posizione in memoria del pixel.
        <color>[in] Colore del pixel. Questo e' sempre un uint32_t RGB, dove il
          campo B occupa il byte meno significatovo.
 Ret: - None.
______________________________________________________________________________*/
static __forceinline void gldcfg_WritePxl (void *pos, gldcolor_t color)
{
    *(uint32_t *)pos = color;
}

/*______________________________________________________________________________ 
 Desc:  Leggi un pixel dal framebuffer.
 Arg: - <pos>[in] Posizione del pixel in memoria.
 Ret: - Colore del pixel. Questo deve sempre essere un uint32_t RGB, dove il
          campo B occupa il byte meno significatovo.
______________________________________________________________________________*/
static __forceinline gldcolor_t gldcfg_ReadPxl (void *pos)
{
    return *(uint32_t *)pos;
}

#endif /* GLDCFG_H_INCLUDED */
