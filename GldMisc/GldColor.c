//===================================================================== INCLUDES
#include "GldMisc/GldColor.h"
#include "GldCfg.h"

#include <math.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Converti un colore dal formato RGB al formato HSB.
 Arg: - <color_rgb>[in] Colore in formato RGB888.
 Ret: - struttura colore del formato HSB.
______________________________________________________________________________*/
gldcolor_HSB_t gldcolor_RGB_to_HSB (gldcolor_t color_rgb)
{
    float r, g, b, max, min, delta;
    float hue, saturation, brightness;
    gldcolor_HSB_t result;

    r = GLDCOLOR_GET_R (color_rgb) / 255.0;
    g = GLDCOLOR_GET_G (color_rgb) / 255.0;
    b = GLDCOLOR_GET_B (color_rgb) / 255.0;

    max = _MAX_ (_MAX_ (r, g), b);
    min = _MIN_ (_MIN_ (r, g), b);
    delta = max - min;

    /* Get the hue */
    if (delta == 0)
        hue = 0;
    if (max == r)
        hue = (g - b) / delta;
    else if (max == g)
        hue = (b - r) / delta + 2;
    else if (max == b)
        hue = (r - g) / delta + 4;
    if (hue >= 0)
        hue = fmod (hue, 6);
    else
        hue = fmod (hue, 6) + 6;
    result.Hue = lroundf (hue * 60.0f); /* range [0-360] */

    /* Get the brightness */
    brightness = max;
    result.Brightness = lroundf (brightness * 100.0f); /* range [0-100] */

    /* Get the saturation */
    if (result.Brightness == 0)
        saturation = 0;
    else
        saturation = delta / brightness;
    result.Saturation = lroundf (saturation * 100.0f); /* range [0-100] */

    return result;
}

/*______________________________________________________________________________ 
 Desc:  Converti un colore dal formato HSB a RGB888.
 Arg: - <color_hsb>[in] colore in formato HSB.
 Ret: - colore in formato RGB888.
______________________________________________________________________________*/
gldcolor_t gldcolor_HSB_to_RGB (gldcolor_HSB_t color_hsb)
{
    float h, s, b, alpha, beta, gamma;
    float red, green, blue;
    int i;
    gldcolor_t result;

    h = color_hsb.Hue / 60.0;
    s = color_hsb.Saturation / 100.0;
    b = color_hsb.Brightness / 100.0;

    alpha = b * (1 - s);
    beta = b * (1 - (h - floorf (h)) * s);
    gamma = b * (1 - (1 - (h - floorf (h))) * s);

    i = floorf (h);
    switch (i)
    {
    case 0:
        red = b;
        green = gamma;
        blue = alpha;
        break;
    case 1:
        red = beta;
        green = b;
        blue = alpha;
        break;
    case 2:
        red = alpha;
        green = b;
        blue = gamma;
        break;
    case 3:
        red = alpha;
        green = beta;
        blue = b;
        break;
    case 4:
        red = gamma;
        green = alpha;
        blue = b;
        break;
    case 5:
        red = b;
        green = alpha;
        blue = beta;
        break;
    }

    result = GLDCOLOR_OR_R (lroundf (red * 255));
    result |= GLDCOLOR_OR_G (lroundf (green * 255));
    result |= GLDCOLOR_OR_B (lroundf (blue * 255));

    return result;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni il numero di byte per pixel di un particolare formato.
 Arg: - <format>[in] pixel format.
 Ret: - numero di byte che costituiscono un singolo pixel.
______________________________________________________________________________*/
uint8_t gldcolor_GetBpp (gldcolor_PxlFormat_t format)
{
    switch (format)
    {
        case GLDCOLOR_NATIVE:
            return GLDCFG_MEMFB_BYTE_PER_PXL;
        case GLDCOLOR_RGBA8888:
        case GLDCOLOR_BGRA8888:
            return 4;
        case GLDCOLOR_RGB565LE:
            return 2;
        default:
        {   /* Hai scordato di aggiungere i bpp per questo colore. */
            GLDCFG_ASSERT (NULL);
            return 0;
        }
    }
}

//============================================================ PRIVATE FUNCTIONS
