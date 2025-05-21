#ifndef GLDCOLOR_H_INCLUDED
#define GLDCOLOR_H_INCLUDED

//===================================================================== INCLUDES
#include "GldCom.h"

//====================================================================== DEFINES
#define GLDCOLOR_R_OFF (16)
#define GLDCOLOR_G_OFF (8)
#define GLDCOLOR_B_OFF (0)

#define GLDCOLOR_GET_R(clr) (((clr) >> GLDCOLOR_R_OFF) & 0xff)
#define GLDCOLOR_GET_G(clr) (((clr) >> GLDCOLOR_G_OFF) & 0xff)
#define GLDCOLOR_GET_B(clr) (((clr) >> GLDCOLOR_B_OFF) & 0xff)

#define GLDCOLOR_OR_R(red)   ((red & 0xff) << GLDCOLOR_R_OFF)
#define GLDCOLOR_OR_G(green) ((green & 0xff) << GLDCOLOR_G_OFF)
#define GLDCOLOR_OR_B(blue)  ((blue & 0xff) << GLDCOLOR_B_OFF)

#define GLDCOLOR_OPA_0   (0)    // valore di opacità 0 = trasparente
#define GLDCOLOR_OPA_10  (25)
#define GLDCOLOR_OPA_20  (51)
#define GLDCOLOR_OPA_30  (76)
#define GLDCOLOR_OPA_40  (102)
#define GLDCOLOR_OPA_50  (128)
#define GLDCOLOR_OPA_60  (153)
#define GLDCOLOR_OPA_70  (179)
#define GLDCOLOR_OPA_80  (204)
#define GLDCOLOR_OPA_90  (230)
#define GLDCOLOR_OPA_100 (256)  // valore di opacità massima = completamente coprente

typedef uint32_t gldcolor_t;
typedef uint16_t gldcolor_Opa_t;

typedef struct
{
    uint16_t Hue;       /* tonalit� [0-360] */
    uint8_t Saturation; /* saturazione [0-100] */
    uint8_t Brightness; /* luminosit� [0-100] */
} gldcolor_HSB_t;

#define GLDCOLOR_REV(clr) (0xffffffff - clr)

/* Basic useful color definitions */
#define GLDCOLOR_BLACK (0x00000000)
#define GLDCOLOR_WHITE (0x00ffffff)
#define GLDCOLOR_NONE  (0x0060e080)
#define GLDCOLOR_RED   (0x00ff0000)
#define GLDCOLOR_GREEN (0x0000ff00)
#define GLDCOLOR_BLUE  (0x000000ff)

typedef enum
{
    GLDCOLOR_NATIVE,   // formato colore nativo (pixel del framebuffer)
    /* la nomenclatura dei colori rispetta il layout in memoria. es:
       nella rappresentazione GLDCOLOR_ARGB8888 gli 8 bit della componente A
       si trovano all'indirizzo minore.
    */
    GLDCOLOR_BGRA8888,
    GLDCOLOR_RGBA8888,
    GLDCOLOR_RGB565LE, // little endian
} gldcolor_PxlFormat_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
/*______________________________________________________________________________ 
 Desc:  Mixa un colore in primo piano (clr_f [foreground]) con un colore di
        sfondo (clr_b [background]). Puoi indicare l'opacit� del colore in primo
        piano (es. 256 completamente coprente, 0 trasparente).
 Arg: - <clr_f>[in] Colore in primo piano.
        <clr_b>[in] Colore in background.
        <opa>[in] Opacit� del colore in foreground [256-0].
 Ret: - None.
______________________________________________________________________________*/
static inline gldcolor_t gldcolor_Mix (gldcolor_t clr_f, gldcolor_t clr_b, gldcolor_Opa_t opa)
{
    gldcolor_t clr = 0;

    clr |= GLDCOLOR_OR_R ((GLDCOLOR_GET_R (clr_f) * opa + GLDCOLOR_GET_R (clr_b) * (256 - opa)) >> 8);
    clr |= GLDCOLOR_OR_G ((GLDCOLOR_GET_G (clr_f) * opa + GLDCOLOR_GET_G (clr_b) * (256 - opa)) >> 8);
    clr |= GLDCOLOR_OR_B ((GLDCOLOR_GET_B (clr_f) * opa + GLDCOLOR_GET_B (clr_b) * (256 - opa)) >> 8);

    return clr;
}

gldcolor_HSB_t gldcolor_RGB_to_HSB (gldcolor_t color_rgb);
gldcolor_t gldcolor_HSB_to_RGB (gldcolor_HSB_t color_hsb);
uint8_t gldcolor_GetBpp (gldcolor_PxlFormat_t format);

#endif /* GLDCOLOR_H_INCLUDED */
