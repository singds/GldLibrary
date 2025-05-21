//===================================================================== INCLUDES
#include "GldCore/GldDraw.h"

#include "GldCore/GldMemFb.h"
#include "GldCore/GldDrawStd.h"
#include "GldCore/GldDrawOpt.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES
static void DrawVLine (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y1, gldcoord_t y2, gldcolor_t clr);
static void DrawHLine (gldhaldisp_t *disp, gldcoord_t y, gldcoord_t x1, gldcoord_t x2, gldcolor_t clr);
static int16_t DrawChar (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldtxt_Letter_t ltr,
                         const gldfont_t *font, gldcolor_t f_clr, gldcolor_t b_clr);
static void PointSwap (gldcoord_Point_t *p1, gldcoord_Point_t *p2);

static void BmpAlign (const gldimg_BmpHeaderImage_t *bmpih, gldcoord_t *x, gldcoord_t *y, uint8_t align);
static void BmpFilter (const gldimg_BmpHeaderImage_t *bmpih, char *palette, const glddraw_BmpFilter_t *filter);
static void DrawBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y,
                     const gldimg_BmpHeaderImage_t *bmpih, const char *pxlmap_p, const char *palette_p);
static void DrawFsBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y,
                       const gldimg_BmpHeaderImage_t *bmpih, FILE *file, const char *palette_p);

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= INLINE FUNCTIONS
/*______________________________________________________________________________ 
 Desc:  Scrivi un particolare pixel sul display. Il pixel viene scritto
        solamente se è all'interno dell'area attiva del display 'Clip'.
 Arg: - <disp>[in] Puntatore al display su cui scrivere.
        <x>[in] Coordinata x del punto.
        <y>[in] Coordinata y del punto.
 Ret: - None.
______________________________________________________________________________*/
static inline void DrawPxl (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcolor_t clr)
{
    if ((x >= disp->Clip.X1 && x < disp->Clip.X2) && (y >= disp->Clip.Y1 && y < disp->Clip.Y2))
    { /* Draw only in inside the clip area */
        disp->SetPxl (x, y, clr);
    }
}

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Scrive un pixel sul display.
 Arg: - <disp> il display su cui settare il pixel.
        <x> coordinata x del pixel.
        <y> coordinata y del pixel.
        <clr> colore del pixel in RGB888.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Pxl (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcolor_t clr)
{
    DrawPxl (disp, x, y, clr);
}

/*______________________________________________________________________________ 
 Desc:  Colora un pixel fondendolo con il contenuto attuale del buffer.
        Il display driver deve supportare la funzione GetPxl.
 Arg: - <disp>[in] display driver.
        <x>[in] coordinata x.
        <y>[in] coordinata y.
        <clr>[in] colore del pixel.
        <opacity>[in] opacità del colore. GLDCOLOR_OPA_100 per coprire
            totalmente lo sfondo.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_PxlOpa (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcolor_t clr, gldcolor_Opa_t opacity)
{
    if ((x >= disp->Clip.X1 && x < disp->Clip.X2) && (y >= disp->Clip.Y1 && y < disp->Clip.Y2))
    { /* Draw only in inside the clip area */
        clr = gldcolor_Mix (clr, disp->GetPxl (x, y), opacity);
        disp->SetPxl (x, y, clr);
    }
}

/*______________________________________________________________________________ 
 Desc:  Riempie un'area rettangolare con un colore specificato.
 Arg: - <disp> il display driver.
        <x1> coordinata x del pixel top-left (incluso).
        <y1> coordinata y del pixel top-left (incluso).
        <x2> coordinata x del pixel bottom-right (escluso).
        <y2> coordinata y del pixel bottom-right (escluso).
        <clr> il colore da usare.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_FillArea (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcolor_t clr)
{
    gldcoord_Area_t draw_a;

    draw_a.X1 = x1;
    draw_a.Y1 = y1;
    draw_a.X2 = x2;
    draw_a.Y2 = y2;

    if (gldcoord_AreaIntersection (&draw_a, &draw_a, &disp->Clip))
    { /* Draw inside the cropped area */

        if (disp->FillArea != NULL && (disp->FillArea (draw_a.X1, draw_a.Y1, gldcoord_AreaWidth (&draw_a), gldcoord_AreaHeight (&draw_a), clr) == 0))
        {
            /* Utilizzato il fill con accelerazione hardware */
        } else
        { /* Worst case: draw every single pixel */
            for (int y_pos = draw_a.Y1; y_pos < draw_a.Y2; y_pos++)
                for (int x_pos = draw_a.X1; x_pos < draw_a.X2; x_pos++)
                    disp->SetPxl (x_pos, y_pos, clr);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Riempie un'area rettangolare con un colore specificato.
    Il rettangolo può avere i bordi arrotondati.
 Arg: - <disp> il display driver.
        <x1> coordinata x del pixel top-left (incluso).
        <y1> coordinata y del pixel top-left (incluso).
        <x2> coordinata x del pixel bottom-right (escluso).
        <y2> coordinata y del pixel bottom-right (escluso).
        <radius> raggio in pixel dei quattro angoli.
        <clr> il colore da usare.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_FillRound (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcoord_t radius, gldcolor_t clr)
{
    /* main body */
    glddraw_FillArea (disp, (x1), (y1 + radius), (x2), (y2 - radius), clr);
    glddraw_FillArea (disp, (x1 + radius), (y1), (x2 - radius), (y1 + radius), clr);
    glddraw_FillArea (disp, (x1 + radius), (y2 - radius), (x2 - radius), (y2), clr);

    if (radius > 0)
    { /* round sections */
        glddraw_ThickArc (disp, (x1 + radius), (y1 + radius), 0, radius, 90, 180, clr);
        glddraw_ThickArc (disp, (x1 + radius), (y2 - radius - 1), 0, radius, 180, 270, clr);
        glddraw_ThickArc (disp, (x2 - radius - 1), (y2 - radius - 1), 0, radius, 270, 360, clr);
        glddraw_ThickArc (disp, (x2 - radius - 1), (y1 + radius), 0, radius, 0, 90, clr);
    }
}

/*______________________________________________________________________________ 
 Desc:  Traccia il contorno di un rettangolo con un colore specificato.
    Il rettangolo può avere i bordi arrotondati.
    IL BORDO E' TRACCIATO ALL'INTERNO del rettangolo specificato.
 Arg: - <disp> il display driver.
        <x1> coordinata x del pixel top-left (incluso).
        <y1> coordinata y del pixel top-left (incluso).
        <x2> coordinata x del pixel bottom-right (escluso).
        <y2> coordinata y del pixel bottom-right (escluso).
        <size> dimensione del bordo.
        <radius> raggio in pixel dei quattro angoli.
        <clr> il colore da usare.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_EmptyRound (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcoord_t size, gldcoord_t radius, gldcolor_t clr)
{
    if (size > 0)
    {
        /* main border */
        glddraw_FillArea (disp, (x1 + radius), (y1), (x2 - radius), (y1 + size), clr);
        glddraw_FillArea (disp, (x1 + radius), (y2 - size), (x2 - radius), (y2), clr);
        glddraw_FillArea (disp, (x1), (y1 + radius), (x1 + size), (y2 - radius), clr);
        glddraw_FillArea (disp, (x2 - size), (y1 + radius), (x2), (y2 - radius), clr);

        if (radius > 0)
        { /* round sections */
            glddraw_ThickArc (disp, (x1 + radius), (y1 + radius), (radius - size + 1), radius, 90, 180, clr);
            glddraw_ThickArc (disp, (x1 + radius), (y2 - radius - 1), (radius - size + 1), radius, 180, 270, clr);
            glddraw_ThickArc (disp, (x2 - radius - 1), (y2 - radius - 1), (radius - size + 1), radius, 270, 360, clr);
            glddraw_ThickArc (disp, (x2 - radius - 1), (y1 + radius), (radius - size + 1), radius, 0, 90, clr);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Traccia una linea di 1 pixel da un punto A ad un punto B.
 Arg: - <disp> il display driver.
        <x1> x del punto A.
        <y1> y del punto A.
        <x2> x del punto B.
        <y2> y del punto B.
        <clr> colore della linea.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Line (gldhaldisp_t *disp, gldcoord_t x1, gldcoord_t y1, gldcoord_t x2, gldcoord_t y2, gldcolor_t clr)
{ /* Algorithm provided by uGFX library: https://ugfx.io/ */
    int16_t dy, dx;
    int16_t addx, addy;
    int16_t P, diff, i;
    int16_t x_pos, y_pos;

    x_pos = x1;
    y_pos = y1;

    /* Is this a horizontal line (or a point) */
    if (y_pos == y2)
    {
        DrawHLine (disp, y_pos, x1, x2, clr);
        return;
    }

    /* Is this a vertical line (or a point) */
    if (x_pos == x2)
    {
        DrawVLine (disp, x_pos, y1, y2, clr);
        return;
    }

    /* Not horizontal or vertical */

    /* Use Bresenham's line drawing algorithm.
       This should be replaced with fixed point slope based line drawing
       which is more efficient on modern processors as it branches less.
       When clipping is needed, all the clipping could also be done up front
       instead of on each pixel. */

    if (x2 >= x_pos)
    {
        dx = x2 - x_pos;
        addx = 1;
    } else
    {
        dx = x_pos - x2;
        addx = -1;
    }
    if (y2 >= y_pos)
    {
        dy = y2 - y_pos;
        addy = 1;
    } else
    {
        dy = y_pos - y2;
        addy = -1;
    }

    if (dx >= dy)
    {
        dy <<= 1;
        P = dy - dx;
        diff = P - dx;

        for (i = 0; i <= dx; ++i)
        {
            DrawPxl (disp, x_pos, y_pos, clr);
            if (P < 0)
            {
                P += dy;
                x_pos += addx;
            } else
            {
                P += diff;
                x_pos += addx;
                y_pos += addy;
            }
        }
    } else
    {
        dx <<= 1;
        P = dx - dy;
        diff = P - dy;

        for (i = 0; i <= dy; ++i)
        {
            DrawPxl (disp, x_pos, y_pos, clr);
            if (P < 0)
            {
                P += dx;
                y_pos += addy;
            } else
            {
                P += diff;
                x_pos += addx;
                y_pos += addy;
            }
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Circle (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius, gldcolor_t clr)
{ /* Algorithm provided by uGFX library: https://ugfx.io/ */
    gldcoord_t a, b, P;

    /* Calculate intermediates */
    a = 1;
    b = radius;
    P = 4 - radius;

    /* Away we go using Bresenham's circle algorithm
       Optimized to prevent double drawing */
    DrawPxl (disp, x, y + b, clr);
    DrawPxl (disp, x, y - b, clr);
    DrawPxl (disp, x + b, y, clr);
    DrawPxl (disp, x - b, y, clr);
    do
    {
        DrawPxl (disp, x + a, y + b, clr);
        DrawPxl (disp, x + a, y - b, clr);
        DrawPxl (disp, x + b, y + a, clr);
        DrawPxl (disp, x - b, y + a, clr);
        DrawPxl (disp, x - a, y + b, clr);
        DrawPxl (disp, x - a, y - b, clr);
        DrawPxl (disp, x + b, y - a, clr);
        DrawPxl (disp, x - b, y - a, clr);
        if (P < 0)
            P += 3 + 2 * a++;
        else
            P += 5 + 2 * (a++ - b--);
    } while (a < b);

    DrawPxl (disp, x + a, y + b, clr);
    DrawPxl (disp, x + a, y - b, clr);
    DrawPxl (disp, x - a, y + b, clr);
    DrawPxl (disp, x - a, y - b, clr);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_FillCircle (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius, gldcolor_t clr)
{ /* Algorithm provided by uGFX library: https://ugfx.io/ */
    gldcoord_t a, b, P;

    /* Calculate intermediates */
    a = 1;
    b = radius;
    P = 4 - radius;

    /* Away we go using Bresenham's circle algorithm
       This is optimized to prevent overdrawing by drawing a line only when a
       variable is about to change value */
    DrawHLine (disp, y, x - b, x + b, clr);

    DrawPxl (disp, x, y + b, clr);
    DrawPxl (disp, x, y - b, clr);

    do
    {
        DrawHLine (disp, y + a, x - b, x + b, clr);
        DrawHLine (disp, y - a, x - b, x + b, clr);

        if (P < 0)
        {
            P += 3 + 2 * a++;
        } else
        {
            DrawHLine (disp, y + b, x - a, x + a, clr);
            DrawHLine (disp, y - b, x - a, x + a, clr);

            P += 5 + 2 * (a++ - b--);
        }
    } while (a < b);

    DrawHLine (disp, y + a, x - b, x + b, clr);
    DrawHLine (disp, y - a, x - b, x + b, clr);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Arc (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldcoord_t radius,
                  gldcoord_t start, gldcoord_t end, gldcolor_t clr)
{
    gldcoord_t a, b, P, sedge, eedge;
    uint8_t full, sbit, ebit, tbit;

    /* Normalize the angles */
    if (start < 0)
        start -= (start / 360 - 1) * 360;
    else if (start >= 360)
        start %= 360;
    if (end < 0)
        end -= (end / 360 - 1) * 360;
    else if (end >= 360)
        end %= 360;

    sbit = 1 << (start / 45);
    ebit = 1 << (end / 45);
    full = 0;
    if (start == end)
    {
        full = 0xFF;
    } else if (end < start)
    {
        for (tbit = sbit << 1; tbit; tbit <<= 1)
            full |= tbit;
        for (tbit = ebit >> 1; tbit; tbit >>= 1)
            full |= tbit;
    } else if (sbit < 0x80)
    {
        for (tbit = sbit << 1; tbit < ebit; tbit <<= 1)
            full |= tbit;
    }
    tbit = start % 45 == 0 ? sbit : 0;

    if (full)
    {
        /* Draw full sectors
           Optimized to prevent double drawing */
        a = 1;
        b = radius;
        P = 4 - radius;

        if (full & 0x60)
            DrawPxl (disp, x, y + b, clr);
        if (full & 0x06)
            DrawPxl (disp, x, y - b, clr);
        if (full & 0x81)
            DrawPxl (disp, x + b, y, clr);
        if (full & 0x18)
            DrawPxl (disp, x - b, y, clr);
        do
        {
            if (full & 0x01)
                DrawPxl (disp, x + b, y - a, clr);
            if (full & 0x02)
                DrawPxl (disp, x + a, y - b, clr);
            if (full & 0x04)
                DrawPxl (disp, x - a, y - b, clr);
            if (full & 0x08)
                DrawPxl (disp, x - b, y - a, clr);
            if (full & 0x10)
                DrawPxl (disp, x - b, y + a, clr);
            if (full & 0x20)
                DrawPxl (disp, x - a, y + b, clr);
            if (full & 0x40)
                DrawPxl (disp, x + a, y + b, clr);
            if (full & 0x80)
                DrawPxl (disp, x + b, y + a, clr);
            if (P < 0)
                P += 3 + 2 * a++;
            else
                P += 5 + 2 * (a++ - b--);
        } while (a < b);
        if (full & 0xC0)
            DrawPxl (disp, x + a, y + b, clr);
        if (full & 0x0C)
            DrawPxl (disp, x - a, y - b, clr);
        if (full & 0x03)
            DrawPxl (disp, x + a, y - b, clr);
        if (full & 0x30)
            DrawPxl (disp, x - a, y + b, clr);
        if (full == 0xFF)
        {
            return;
        }
    }

    sedge = floor (radius * ((sbit & 0x99) ? sin (start * _PI_ / 180) : cos (start * _PI_ / 180)) + 0.5);
    eedge = floor (radius * ((ebit & 0x99) ? sin (end * _PI_ / 180) : cos (end * _PI_ / 180)) + 0.5);

    if (sbit & 0xB4)
        sedge = -sedge;
    if (ebit & 0xB4)
        eedge = -eedge;

    if (sbit != ebit)
    {
        /* Draw start and end sectors
           Optimized to prevent double drawing */
        a = 1;
        b = radius;
        P = 4 - radius;
        if ((sbit & 0x20) || (tbit & 0x40) || (ebit & 0x40))
            DrawPxl (disp, x, y + b, clr);
        if ((sbit & 0x02) || (tbit & 0x04) || (ebit & 0x04))
            DrawPxl (disp, x, y - b, clr);
        if ((sbit & 0x80) || (tbit & 0x01) || (ebit & 0x01))
            DrawPxl (disp, x + b, y, clr);
        if ((sbit & 0x08) || (tbit & 0x10) || (ebit & 0x10))
            DrawPxl (disp, x - b, y, clr);
        do
        {
            if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge))
                DrawPxl (disp, x + b, y - a, clr);
            if (((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge))
                DrawPxl (disp, x + a, y - b, clr);
            if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge))
                DrawPxl (disp, x - a, y - b, clr);
            if (((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge))
                DrawPxl (disp, x - b, y - a, clr);
            if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge))
                DrawPxl (disp, x - b, y + a, clr);
            if (((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge))
                DrawPxl (disp, x - a, y + b, clr);
            if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge))
                DrawPxl (disp, x + a, y + b, clr);
            if (((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge))
                DrawPxl (disp, x + b, y + a, clr);
            if (P < 0)
                P += 3 + 2 * a++;
            else
                P += 5 + 2 * (a++ - b--);
        } while (a < b);
        if (((sbit & 0x40) && a >= sedge) || ((ebit & 0x40) && a <= eedge) || ((sbit & 0x80) && a <= sedge) || ((ebit & 0x80) && a >= eedge))
            DrawPxl (disp, x + a, y + b, clr);
        if (((sbit & 0x04) && a >= sedge) || ((ebit & 0x04) && a <= eedge) || ((sbit & 0x08) && a <= sedge) || ((ebit & 0x08) && a >= eedge))
            DrawPxl (disp, x - a, y - b, clr);
        if (((sbit & 0x01) && a >= sedge) || ((ebit & 0x01) && a <= eedge) || ((sbit & 0x02) && a <= sedge) || ((ebit & 0x02) && a >= eedge))
            DrawPxl (disp, x + a, y - b, clr);
        if (((sbit & 0x10) && a >= sedge) || ((ebit & 0x10) && a <= eedge) || ((sbit & 0x20) && a <= sedge) || ((ebit & 0x20) && a >= eedge))
            DrawPxl (disp, x - a, y + b, clr);
    } else if (end < start)
    {
        /* Draw start/end sector where it is a non-internal angle
           Optimized to prevent double drawing */
        a = 1;
        b = radius;
        P = 4 - radius;
        if ((sbit & 0x60) || (tbit & 0xC0))
            DrawPxl (disp, x, y + b, clr);
        if ((sbit & 0x06) || (tbit & 0x0C))
            DrawPxl (disp, x, y - b, clr);
        if ((sbit & 0x81) || (tbit & 0x03))
            DrawPxl (disp, x + b, y, clr);
        if ((sbit & 0x18) || (tbit & 0x30))
            DrawPxl (disp, x - b, y, clr);
        do
        {
            if ((sbit & 0x01) && (a >= sedge || a <= eedge))
                DrawPxl (disp, x + b, y - a, clr);
            if ((sbit & 0x02) && (a <= sedge || a >= eedge))
                DrawPxl (disp, x + a, y - b, clr);
            if ((sbit & 0x04) && (a >= sedge || a <= eedge))
                DrawPxl (disp, x - a, y - b, clr);
            if ((sbit & 0x08) && (a <= sedge || a >= eedge))
                DrawPxl (disp, x - b, y - a, clr);
            if ((sbit & 0x10) && (a >= sedge || a <= eedge))
                DrawPxl (disp, x - b, y + a, clr);
            if ((sbit & 0x20) && (a <= sedge || a >= eedge))
                DrawPxl (disp, x - a, y + b, clr);
            if ((sbit & 0x40) && (a >= sedge || a <= eedge))
                DrawPxl (disp, x + a, y + b, clr);
            if ((sbit & 0x80) && (a <= sedge || a >= eedge))
                DrawPxl (disp, x + b, y + a, clr);
            if (P < 0)
                P += 3 + 2 * a++;
            else
                P += 5 + 2 * (a++ - b--);
        } while (a < b);
        if (((sbit & 0x04) && (a >= sedge || a <= eedge)) || ((sbit & 0x08) && (a <= sedge || a >= eedge)))
            DrawPxl (disp, x - a, y - b, clr);
        if (((sbit & 0x40) && (a >= sedge || a <= eedge)) || ((sbit & 0x80) && (a <= sedge || a >= eedge)))
            DrawPxl (disp, x + a, y + b, clr);
        if (((sbit & 0x01) && (a >= sedge || a <= eedge)) || ((sbit & 0x02) && (a <= sedge || a >= eedge)))
            DrawPxl (disp, x + a, y - b, clr);
        if (((sbit & 0x10) && (a >= sedge || a <= eedge)) || ((sbit & 0x20) && (a <= sedge || a >= eedge)))
            DrawPxl (disp, x - a, y + b, clr);
    } else
    {
        /* Draw start/end sector where it is a internal angle
           Optimized to prevent double drawing */
        a = 1;
        b = radius;
        P = 4 - radius;
        if (((sbit & 0x20) && !eedge) || ((sbit & 0x40) && !sedge))
            DrawPxl (disp, x, y + b, clr);
        if (((sbit & 0x02) && !eedge) || ((sbit & 0x04) && !sedge))
            DrawPxl (disp, x, y - b, clr);
        if (((sbit & 0x80) && !eedge) || ((sbit & 0x01) && !sedge))
            DrawPxl (disp, x + b, y, clr);
        if (((sbit & 0x08) && !eedge) || ((sbit & 0x10) && !sedge))
            DrawPxl (disp, x - b, y, clr);
        do
        {
            if (((sbit & 0x01) && a >= sedge && a <= eedge))
                DrawPxl (disp, x + b, y - a, clr);
            if (((sbit & 0x02) && a <= sedge && a >= eedge))
                DrawPxl (disp, x + a, y - b, clr);
            if (((sbit & 0x04) && a >= sedge && a <= eedge))
                DrawPxl (disp, x - a, y - b, clr);
            if (((sbit & 0x08) && a <= sedge && a >= eedge))
                DrawPxl (disp, x - b, y - a, clr);
            if (((sbit & 0x10) && a >= sedge && a <= eedge))
                DrawPxl (disp, x - b, y + a, clr);
            if (((sbit & 0x20) && a <= sedge && a >= eedge))
                DrawPxl (disp, x - a, y + b, clr);
            if (((sbit & 0x40) && a >= sedge && a <= eedge))
                DrawPxl (disp, x + a, y + b, clr);
            if (((sbit & 0x80) && a <= sedge && a >= eedge))
                DrawPxl (disp, x + b, y + a, clr);
            if (P < 0)
                P += 3 + 2 * a++;
            else
                P += 5 + 2 * (a++ - b--);
        } while (a < b);
        if (((sbit & 0x04) && a >= sedge && a <= eedge) || ((sbit & 0x08) && a <= sedge && a >= eedge))
            DrawPxl (disp, x - a, y - b, clr);
        if (((sbit & 0x40) && a >= sedge && a <= eedge) || ((sbit & 0x80) && a <= sedge && a >= eedge))
            DrawPxl (disp, x + a, y + b, clr);
        if (((sbit & 0x01) && a >= sedge && a <= eedge) || ((sbit & 0x02) && a <= sedge && a >= eedge))
            DrawPxl (disp, x + a, y - b, clr);
        if (((sbit & 0x10) && a >= sedge && a <= eedge) || ((sbit & 0x20) && a <= sedge && a >= eedge))
            DrawPxl (disp, x - a, y + b, clr);
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_ThickArc (gldhaldisp_t *disp, gldcoord_t xc, gldcoord_t yc,
                       gldcoord_t radiusStart, gldcoord_t radiusEnd, gldcoord_t start, gldcoord_t end, gldcolor_t clr)
{ /* Algorithm provided by uGFX library: https://ugfx.io/ */
    gldcoord_t x, y, d, r;
    gldcoord_t startTan, endTan, curangle;
    gldcoord_t precision = 512;

    /* Normalize the angles */
    if (start < 0)
        start -= (start / 360 - 1) * 360;
    else if (start >= 360)
        start %= 360;
    if (end < 0)
        end -= (end / 360 - 1) * 360;
    else if (end >= 360)
        end %= 360;


    if ((start / 45) % 2 == 0)
        startTan = (tan ((start % 45) * _PI_ / 180) + start / 45) * precision;
    else
        startTan = (1 + tan ((start % 45 - 45) * _PI_ / 180) + start / 45) * precision;

    if ((end / 45) % 2 == 0)
        endTan = (tan ((end % 45) * _PI_ / 180) + end / 45) * precision;
    else
        endTan = (1 + tan ((end % 45 - 45) * _PI_ / 180) + end / 45) * precision;

    /* Draw concentric circles using Andres algorithm */
    for (r = radiusStart; r <= radiusEnd; r++)
    {
        x = 0;
        y = r;
        d = r - 1;

        while (y != 0 && y >= x)
        {
            /* approximate tan */
            curangle = x * precision / y;

            if (end > start)
            {
                /* Draw points by symmetry */
                if (curangle > startTan && curangle < endTan)
                    DrawPxl (disp, xc + y, yc - x, clr);
                if (curangle + 2 * precision > startTan && curangle + 2 * precision < endTan)
                    DrawPxl (disp, xc - x, yc - y, clr);
                if (curangle + 4 * precision > startTan && curangle + 4 * precision < endTan)
                    DrawPxl (disp, xc - y, yc + x, clr);
                if (curangle + 6 * precision > startTan && curangle + 6 * precision < endTan)
                    DrawPxl (disp, xc + x, yc + y, clr);

                curangle = precision - curangle;

                if (curangle + precision > startTan && curangle + precision < endTan)
                    DrawPxl (disp, xc + x, yc - y, clr);
                if (curangle + 3 * precision > startTan && curangle + 3 * precision < endTan)
                    DrawPxl (disp, xc - y, yc - x, clr);
                if (curangle + 5 * precision > startTan && curangle + 5 * precision < endTan)
                    DrawPxl (disp, xc - x, yc + y, clr);
                if (curangle + 7 * precision > startTan && curangle + 7 * precision < endTan)
                    DrawPxl (disp, xc + y, yc + x, clr);

            } else
            {
                /* Draw points by symmetry */
                if (curangle > startTan || curangle < endTan)
                    DrawPxl (disp, xc + y, yc - x, clr);
                if (curangle + 2 * precision > startTan || curangle + 2 * precision < endTan)
                    DrawPxl (disp, xc - x, yc - y, clr);
                if (curangle + 4 * precision > startTan || curangle + 4 * precision < endTan)
                    DrawPxl (disp, xc - y, yc + x, clr);
                if (curangle + 6 * precision > startTan || curangle + 6 * precision < endTan)
                    DrawPxl (disp, xc + x, yc + y, clr);

                curangle = precision - curangle;

                if (curangle + precision > startTan || curangle + precision < endTan)
                    DrawPxl (disp, xc + x, yc - y, clr);
                if (curangle + 3 * precision > startTan || curangle + 3 * precision < endTan)
                    DrawPxl (disp, xc - y, yc - x, clr);
                if (curangle + 5 * precision > startTan || curangle + 5 * precision < endTan)
                    DrawPxl (disp, xc - x, yc + y, clr);
                if (curangle + 7 * precision > startTan || curangle + 7 * precision < endTan)
                    DrawPxl (disp, xc + y, yc + x, clr);
            }

            /* Compute next point */
            if (d >= 2 * x)
            {
                d -= 2 * x + 1;
                x++;
            } else if (d < 2 * (r - y))
            {
                d += 2 * y - 1;
                y--;
            } else
            {
                d += 2 * (y - x - 1);
                y--;
                x++;
            }
        }
    }
}

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY) || (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
int16_t glddraw_Printf (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align,
                        gldfont_t *font, gldcolor_t f_clr, gldcolor_t b_clr, const char *format, ...)
{
    gldcoord_t y_pos;
    int16_t lenght, line_cnt;
    va_list va;
    char *line_p, *line_end_p, *next_line_p;

    /* get the required buffer size */
    va_start (va, format);
    lenght = vsnprintf (NULL, 0, format, va);
    va_end (va);

#ifndef _MSC_VER
    char buff[lenght + 1];
#else
    char buff[2048 + 1];
#endif

    /* print on buffer */
    va_start (va, format);
    vsnprintf (buff, lenght + 1, format, va);
    va_end (va);

    line_cnt = gldtxt_GetLineCnt (buff);

    /* vertical text alignament */
    y_pos = y;
    if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_BOTTOM)
        y_pos = y - font->Height * line_cnt;
    else if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_VMID)
        y_pos = y - ((font->Height * line_cnt) >> 1);

    line_p = buff;
    while (line_cnt--)
    {
        gldcoord_t x_pos;
        int16_t line_size;
        gldtxt_Letter_t ltr;

        next_line_p = gldtxt_GetLine (line_p, &line_size); /* get line */

        /* horizontal text alignament */
        x_pos = x;
        if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_RIGHT)
            x_pos = x - gldfont_GetTxtWidth (font, line_p, line_size);
        else if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_HMID)
            x_pos = x - (gldfont_GetTxtWidth (font, line_p, line_size) >> 1);


        line_end_p = line_p + line_size;
        line_p = gldtxt_GetLetter (&ltr, line_p);
        while (line_p != NULL && line_p <= line_end_p)
        {
            DrawChar (disp, x_pos, y_pos, ltr, font, f_clr, b_clr);

            /* move cursor to the next letter */
            x_pos += gldfont_GetLetterWidth (font, ltr);
            line_p = gldtxt_GetLetter (&ltr, line_p);
        }

        /* move cursor to the next line */
        y_pos += font->Height;
        line_p = next_line_p;
    }

    return lenght;
}
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
int16_t glddraw_Printf (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align,
                        gldfont_t *font, gldcolor_t f_clr, gldcolor_t b_clr, const char *format, ...)
{
    gldcoord_t cursor_y_pos; /* coordinata y del cursore (baseline dei glifi) */
    int16_t lenght, line_cnt;
    va_list va;
    char *line_p, *line_end_p, *next_line_p;
    const FONTBUILDERFORC_TYPE_CHARACTER *glyphA;
    char *buff;

    /* get the required buffer size */
    va_start (va, format);
    lenght = vsnprintf (NULL, 0, format, va);
    va_end (va);

    buff = GLDCFG_MALLOC (lenght + 1);
    GLDCFG_ASSERT (buff);

    /* print on buffer */
    va_start (va, format);
    vsnprintf (buff, lenght + 1, format, va);
    va_end (va);

    line_cnt = gldtxt_GetLineCnt (buff);

    /* vertical text alignament */
    glyphA = gldfont_GetCharacter (font, 'A');
    if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_BOTTOM)
    {
        y -= font->pxl_baseline_to_baseline * (line_cnt - 1);
        y -= glyphA->bmp_pxl_height;
    } else if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_VMID)
    {
        y -= (font->pxl_baseline_to_baseline * (line_cnt - 1) + glyphA->bmp_pxl_height) >> 1;
    }
    cursor_y_pos = y + glyphA->bmp_pxl_height;

    line_p = buff;
    while (line_cnt--)
    {
        gldcoord_t cursor_x_pos;
        int16_t line_size;
        gldtxt_Letter_t ltr, previus_ltr = 0;

        next_line_p = gldtxt_GetLine (line_p, &line_size); /* get line */

        /* horizontal text alignament */
        cursor_x_pos = x;
        if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_RIGHT)
            cursor_x_pos = x - gldfont_GetTxtWidth (font, line_p, line_size);
        else if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_HMID)
            cursor_x_pos = x - (gldfont_GetTxtWidth (font, line_p, line_size) >> 1);


        line_end_p = line_p + line_size;
        line_p = gldtxt_GetLetter (&ltr, line_p);
        while (line_p != NULL && line_p <= line_end_p)
        { /* check for kerning adjustment between the last letter and this one */
            if (previus_ltr)
                cursor_x_pos += gldfont_GetKerning (font, previus_ltr, ltr);
            /* draw letter and move cursor to the next letter */
            cursor_x_pos += DrawChar (disp, cursor_x_pos, cursor_y_pos, ltr, font, f_clr, b_clr);
            previus_ltr = ltr; /* momorizzo il glifo che ho appena renderizzato */
            line_p = gldtxt_GetLetter (&ltr, line_p);
        }

        /* move cursor to the next line */
        cursor_y_pos += font->pxl_baseline_to_baseline;
        line_p = next_line_p;
    }
    GLDCFG_FREE (buff);

    return lenght;
}
#endif

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Bmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align,
                  const char *bmp_p, const glddraw_BmpFilter_t *filter)
{
    if (bmp_p)
    {
        char *palette = NULL; // rimane null con bitmap a 32bit
        gldimg_BmpHeaderFile_t bmpfh;
        gldimg_BmpHeaderImage_t bmpih;
        const char *map_p;
        uint32_t numClrPalette;

        gldimg_BmpGetHeaderFile (&bmpfh, bmp_p);
        gldimg_BmpGetHeaderImage (&bmpih, bmp_p);

        /* create the bitmap color palette. If a filter is provided, can be
           dynamically modified. */
        /* BiClrUsed è a 0 per immagini non indicizzate (32bit x pxl) e di conseguenza
        palette rimane NULL. Questo non è un problema, il resto del codice è preparato
        a gestire palette = NULL. */
        numClrPalette = bmpih.BiClrUsed;
        if (numClrPalette == 0 && (bmpih.BiBitCount == 4 || bmpih.BiBitCount == 8))
            numClrPalette = 0x01UL << bmpih.BiBitCount;
        if (numClrPalette) {
            palette = GLDCFG_MALLOC (numClrPalette * 4);
            GLDCFG_ASSERT (palette);
            memcpy (palette, bmp_p + GLDIMG_BMP_PALETTE_OFF, numClrPalette * 4);
        }

        /* gestisco l'allinamento */
        BmpAlign (&bmpih, &x, &y, align);
        /* gestisco i filtri sulla paletta colori */
        BmpFilter (&bmpih, palette, filter);

        /* get the bitmap pixel map start pointer */
        map_p = bmp_p + bmpfh.BfOffBits;

        DrawBmp (disp, x, y, &bmpih, map_p, palette);
        if (palette) {
            GLDCFG_FREE (palette);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_FsBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, uint8_t align,
                    const char *f_name, const glddraw_BmpFilter_t *filter)
{
    if (f_name)
    {
        FILE *file;
        if ((file = fopen (f_name, "rb")) != NULL)
        {
            char *palette = NULL; // rimane null con bitmap a 32bit
            gldimg_BmpHeaderFile_t bmpfh;
            gldimg_BmpHeaderImage_t bmpih;
            int32_t rdSize;
            char bmph[GLDIMG_BMP_SIZE_FILEH + GLDIMG_BMP_SIZE_IMAGEH];

            GLDCFG_ASSERT (bmph);

            rdSize = GLDIMG_BMP_SIZE_FILEH + GLDIMG_BMP_SIZE_IMAGEH;
            if (fread (bmph, 1, rdSize, file) == rdSize)
            {
                bool paletteOk = true;
                uint32_t numClrPalette;
                gldimg_BmpGetHeaderFile (&bmpfh, bmph);
                gldimg_BmpGetHeaderImage (&bmpih, bmph);

                /* create the bitmap color palette. If a filter is provided, can be
                   dynamically modified. */
                /* BiClrUsed è a 0 per immagini non indicizzate (32bit x pxl) e di conseguenza
                palette rimane NULL. Questo non è un problema, il resto del codice è preparato
                a gestire palette = NULL. */
                numClrPalette = bmpih.BiClrUsed;
                if (numClrPalette == 0 && (bmpih.BiBitCount == 4 || bmpih.BiBitCount == 8))
                    numClrPalette = 0x01UL << bmpih.BiBitCount;
                if (numClrPalette) {
                    palette = GLDCFG_MALLOC (numClrPalette * 4);
                    GLDCFG_ASSERT (palette);
                    rdSize = numClrPalette * 4;
                    if (fread (palette, 1, rdSize, file) != rdSize)
                        paletteOk = false;
                }
                if (paletteOk) {
                    /* gestisco l'allinamento */
                    BmpAlign (&bmpih, &x, &y, align);
                    /* gestisco i filtri sulla paletta colori */
                    BmpFilter (&bmpih, palette, filter);

                    /* set the file pointer to the start of the pixel map */
                    fseek (file, bmpfh.BfOffBits, SEEK_SET);

                    DrawFsBmp (disp, x, y, &bmpih, file, palette);
                }
                if (palette) {
                    GLDCFG_FREE (palette);
                }
            }
            
            fclose (file);
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - None.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_Triangle (gldhaldisp_t *disp, const gldcoord_Point_t *points, gldcolor_t color)
{ /* Algoritmo copiato da littlevgl https://littlevgl.com/ */
    gldcoord_Point_t tri[3];

    memcpy (tri, points, sizeof (tri));

    /*Sort the vertices according to their y coordinate (0: y max, 1: y mid, 2:y min)*/
    if (tri[1].Y < tri[0].Y)
        PointSwap (&tri[1], &tri[0]);
    if (tri[2].Y < tri[1].Y)
        PointSwap (&tri[2], &tri[1]);
    if (tri[1].Y < tri[0].Y)
        PointSwap (&tri[1], &tri[0]);

    /*Return is the triangle is degenerated*/
    if (tri[0].X == tri[1].X && tri[0].Y == tri[1].Y)
        return;
    if (tri[1].X == tri[2].X && tri[1].Y == tri[2].Y)
        return;
    if (tri[0].X == tri[2].X && tri[0].Y == tri[2].Y)
        return;

    if (tri[0].X == tri[1].X && tri[1].X == tri[2].X)
        return;
    if (tri[0].Y == tri[1].Y && tri[1].Y == tri[2].Y)
        return;

    /*Draw the triangle*/
    gldcoord_Point_t edge1;
    gldcoord_t dx1 = _ABS_ (tri[0].X - tri[1].X);
    gldcoord_t sx1 = tri[0].X < tri[1].X ? 1 : -1;
    gldcoord_t dy1 = _ABS_ (tri[0].Y - tri[1].Y);
    gldcoord_t sy1 = tri[0].Y < tri[1].Y ? 1 : -1;
    gldcoord_t err1 = (dx1 > dy1 ? dx1 : -dy1) / 2;
    gldcoord_t err_tmp1;

    gldcoord_Point_t edge2;
    gldcoord_t dx2 = _ABS_ (tri[0].X - tri[2].X);
    gldcoord_t sx2 = tri[0].X < tri[2].X ? 1 : -1;
    gldcoord_t dy2 = _ABS_ (tri[0].Y - tri[2].Y);
    gldcoord_t sy2 = tri[0].Y < tri[2].Y ? 1 : -1;
    gldcoord_t err2 = (dx1 > dy2 ? dx2 : -dy2) / 2;
    gldcoord_t err_tmp2;

    gldcoord_t y1_tmp;
    gldcoord_t y2_tmp;

    edge1.X = tri[0].X;
    edge1.Y = tri[0].Y;
    edge2.X = tri[0].X;
    edge2.Y = tri[0].Y;
    gldcoord_Area_t act_area;
    gldcoord_Area_t draw_area;

    while (1)
    {
        act_area.X1 = edge1.X;
        act_area.X2 = edge2.X;
        act_area.Y1 = edge1.Y;
        act_area.Y2 = edge2.Y;


        draw_area.X1 = _MIN_ (act_area.X1, act_area.X2);
        draw_area.X2 = _MAX_ (act_area.X1, act_area.X2);
        draw_area.Y1 = _MIN_ (act_area.Y1, act_area.Y2);
        draw_area.Y2 = _MAX_ (act_area.Y1, act_area.Y2);
        draw_area.X2--; /*Do not draw most right pixel because it will be drawn by the adjacent triangle*/
                        //        fill_fp(&draw_area, mask, color, LV_OPA_50);
        glddraw_FillArea (disp, draw_area.X1, draw_area.Y1, draw_area.X2 + 1, draw_area.Y2 + 1, color);

        /*Calc. the next point of edge1*/
        y1_tmp = edge1.Y;
        do
        {
            if (edge1.X == tri[1].X && edge1.Y == tri[1].Y)
            {

                dx1 = _ABS_ (tri[1].X - tri[2].X);
                sx1 = tri[1].X < tri[2].X ? 1 : -1;
                dy1 = _ABS_ (tri[1].Y - tri[2].Y);
                sy1 = tri[1].Y < tri[2].Y ? 1 : -1;
                err1 = (dx1 > dy1 ? dx1 : -dy1) / 2;
            } else if (edge1.X == tri[2].X && edge1.Y == tri[2].Y)
                return;
            err_tmp1 = err1;
            if (err_tmp1 > -dx1)
            {
                err1 -= dy1;
                edge1.X += sx1;
            }
            if (err_tmp1 < dy1)
            {
                err1 += dx1;
                edge1.Y += sy1;
            }
        } while (edge1.Y == y1_tmp);

        /*Calc. the next point of edge2*/
        y2_tmp = edge2.Y;
        do
        {
            if (edge2.X == tri[2].X && edge2.Y == tri[2].Y)
                return;
            err_tmp2 = err2;
            if (err_tmp2 > -dx2)
            {
                err2 -= dy2;
                edge2.X += sx2;
            }
            if (err_tmp2 < dy2)
            {
                err2 += dx2;
                edge2.Y += sy2;
            }
        } while (edge2.Y == y2_tmp);
    }
}

/*______________________________________________________________________________
 Desc:  Scrivi sul display una mappa di pixel. La mappa di pixel deve essere
    completamente impaccata: non può esserci offset tra una linea di pixel e la
    successiva.
 Arg: - <disp>[in] display di destinazione.
        <pxlMap>[in] indirizzo della mappa di pixel.
        <x>[in] coordinate angolo top-left.
        <y>[in] coordinate angolo top-left.
        <xSz>[in] larghezza della mappa.
        <ySz>[in] altezza della mappa.
        <ori>[in] orientamento del pixel all'interno dell'area di <pxlMap>.
        <format>[in] formato dei pixel della mappa. Non tutti i formati colore
            sono attualmente supportati.
 Ret: - None.
______________________________________________________________________________*/
void glddraw_PxlMap (gldhaldisp_t *disp, const uint8_t *pxlMap,
                     gldcoord_t x, gldcoord_t y,
                     uint32_t xSz, uint32_t ySz,
                     gldcoord_Ori_e ori, gldcolor_PxlFormat_t format)
{
    gldrect_t imgRect; // rettangolo di memoria dell'immagine
    gldrect_t subImgRect; // rettangolo di memoria della sottoimmagine
    
    imgRect.MemStart = (void *)pxlMap;
    imgRect.Ori = ori;
    imgRect.Bpp = gldcolor_GetBpp (format);
    imgRect.Area.X1 = x;
    imgRect.Area.Y1 = y;
    imgRect.Area.X2 = x + xSz;
    imgRect.Area.Y2 = y + ySz;
    // set height width offset
    gldrect_SetWidthHeightAsFull (&imgRect);
    
    if (gldcoord_AreaIntersection (&subImgRect.Area, &imgRect.Area, &disp->Clip))
    {
        bool drawDone = false;
        
        gldrect_GetFrameMemoryRect (&imgRect, &subImgRect);
        
#if (GLDCFG_DRAW_OPTIMIZATION == 1)
        if (disp == &gldmemfb_Disp)
        {   // ciclo ottimizzato
            glddrawopt_MemRect_t memRect;
            gldmemfb_GetMemRect (&memRect);
            if (glddrawopt_PxlMap (&memRect, &subImgRect, format) == 0)
                drawDone = true;
        }
#endif
        if (!drawDone)
        {   // ciclo normale non ottimizzato
            glddrawstd_PxlMap (disp, &subImgRect, format);
        }
    }
}

//============================================================ PRIVATE FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void DrawVLine (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y1, gldcoord_t y2, gldcolor_t clr)
{
    gldcoord_t y_min, y_max;

    y_min = _MIN_ (y1, y2);
    y_max = _MAX_ (y1, y2);
    /* Always goes from min to max */
    while (y_min <= y_max)
    {
        DrawPxl (disp, x, y_min, clr);
        y_min++;
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void DrawHLine (gldhaldisp_t *disp, gldcoord_t y, gldcoord_t x1, gldcoord_t x2, gldcolor_t clr)
{
    gldcoord_t x_min, x_max;

    x_min = _MIN_ (x1, x2);
    x_max = _MAX_ (x1, x2);
    /* Always goes from min to max */
    while (x_min <= x_max)
    {
        DrawPxl (disp, x_min, y, clr);
        x_min++;
    }
}

/*______________________________________________________________________________ 
 Desc:  Restituisce il numero di pixel di cui far avanzare il cursore.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static int16_t DrawChar (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y, gldtxt_Letter_t ltr,
                         const gldfont_t *font, gldcolor_t f_clr, gldcolor_t b_clr)
{
    int32_t pxl_advance = 0;

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__LEGACY)
    const char *map_p;
    int16_t width, byte_width;

    /* retrieve letter glyph info */
    width = gldfont_GetLetterWidth (font, ltr);
    map_p = gldfont_GetLetterDrawInfo (font, ltr);

    /* get the character pixel map byte width */
    byte_width = width / 8;
    if (width & 0x07)
        byte_width++;

    /* cycle the whole pixel map */
    for (int y_pos = 0; y_pos < font->Height; y_pos++)
    {
        /* se the pointer to the map new line */
        const char *byte = map_p + (y_pos * byte_width);

        for (int x_pos = 0; x_pos < width;)
        {
            uint8_t mask = 0x01 << (x_pos & 0x07);
            /* test the pixel value */
            if (mask & *byte)
                DrawPxl (disp, x_pos + x, y_pos + y, f_clr);
            else if (b_clr != GLDCOLOR_NONE)
                DrawPxl (disp, x_pos + x, y_pos + y, b_clr);

            if ((++x_pos & 0x07) == 0)
                byte++;
        }
    }
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__NEW)
    const uint8_t *map_p;
    const gldfont_NFontInfo_t *ltrInfo;
    int16_t byte_width;
    uint8_t colorMask = 0x00;

    /* retrieve letter glyph info */
    ltrInfo = gldfont_GetLetterDrawInfo (font, ltr, &map_p);
    if (ltrInfo != NULL)
    {
        /* get the character pixel map byte width */
        byte_width = (ltrInfo->Width * font->BitColor) / 8;
        if ((ltrInfo->Width * font->BitColor) & 0x07)
            byte_width++;

        /* Creo una maschera per prelevare il valore di ogni pixel.
           I bpp del font sono sempre divisori di 8. */
        for (int k = 0; k < font->BitColor; k++)
            colorMask = (colorMask << 1) | 0x01;

        /* cycle the whole pixel map */
        for (int y_pos = 0; y_pos < ltrInfo->DataHeight; y_pos++)
        {
            uint8_t byte_shift = 0;
            /* se the pointer to the map new line */
            const uint8_t *byte = map_p + (y_pos * byte_width);

            for (int x_pos = 0; x_pos < ltrInfo->Width; x_pos++)
            {
                uint8_t pxl_value;
                gldcoord_t x_disp, y_disp;

                pxl_value = (*byte >> byte_shift) & colorMask;

                x_disp = x + x_pos;
                y_disp = y + ltrInfo->EmptyTopHeight + y_pos;

                /* Verifico se il pixel è all'interno dell'area attiva del display */
                if ((x_disp >= disp->Clip.X1 && x_disp < disp->Clip.X2) && (y_disp >= disp->Clip.Y1 && y_disp < disp->Clip.Y2))
                {
                    /* test the pixel value */
                    if (pxl_value)
                    {
                        gldcolor_t color;

                        if (pxl_value == GLDCOLOR_OPA_100 || font->BitColor == 1)
                        { /* velocizzo il codice quando NON devo fondere il colore con lo
                             sfondo */
                            color = f_clr;
                        } else
                        {
                            /* qui devo rappresentare il pixel con la giusta opacità,
                               fondendo il colore del testo con quello dello sfondo */
                            gldcolor_Opa_t opacity;

                            opacity = (pxl_value * GLDCOLOR_OPA_100) / colorMask;
                            if (b_clr != GLDCOLOR_NONE)
                            { /* devo fondere con il colore di sfondo impostato */
                                color = gldcolor_Mix (f_clr, b_clr, opacity);
                            } else
                            { /* devo fondere con i pixel attualmente visualizzati */
                                color = gldcolor_Mix (f_clr, disp->GetPxl (x_disp, y_disp), opacity);
                            }
                        }
                        disp->SetPxl (x_disp, y_disp, color);
                    } else if (b_clr != GLDCOLOR_NONE)
                        disp->SetPxl (x_disp, y_disp, b_clr);
                }

                byte_shift += font->BitColor;
                if (byte_shift >= 8)
                { /* mi sposto all'inizio del byte successivo */
                    byte_shift = 0;
                    byte++;
                }
            }
        }
    }
#endif

#if (GLDCFG_TXT_FONT_FORMAT == GLDCFG_TXT_FONT_FORMAT__PERFECT)
    const uint8_t *map_p;
    const FONTBUILDERFORC_TYPE_CHARACTER *ltrInfo;
    int16_t byte_width;
    uint8_t colorMask = 0x00;

    /* retrieve letter glyph info */
    ltrInfo = gldfont_GetCharacter (font, ltr);
    if (ltrInfo != NULL)
    {
        pxl_advance = ltrInfo->pxl_advance;
        map_p = (const uint8_t *)&font->bitmaps_table[ltrInfo->bmp_offset];

        /* get the character pixel map byte width */
        byte_width = (ltrInfo->bmp_pxl_width * font->bpp) / 8;
        if ((ltrInfo->bmp_pxl_width * font->bpp) & 0x07)
            byte_width++;

        /* Creo una maschera per prelevare il valore di ogni pixel.
           I bpp del font sono sempre divisori di 8. */
        for (int k = 0; k < font->bpp; k++)
            colorMask = (colorMask << 1) | 0x01;

        /* cycle the whole pixel map */
        for (int y_pos = 0; y_pos < ltrInfo->bmp_pxl_height; y_pos++)
        {
            int8_t byte_shift = 8 - font->bpp;
            /* se the pointer to the map new line */
            const uint8_t *byte = map_p + (y_pos * byte_width);

            for (int x_pos = 0; x_pos < ltrInfo->bmp_pxl_width; x_pos++)
            {
                uint8_t pxl_value;
                gldcoord_t x_disp, y_disp;

                pxl_value = (*byte >> byte_shift) & colorMask;

                x_disp = x + x_pos + ltrInfo->pxl_left;
                y_disp = y - ltrInfo->pxl_top + y_pos;

                /* Verifico se il pixel è all'interno dell'area attiva del display */
                if ((x_disp >= disp->Clip.X1 && x_disp < disp->Clip.X2) && (y_disp >= disp->Clip.Y1 && y_disp < disp->Clip.Y2))
                {
                    /* test the pixel value */
                    if (pxl_value)
                    {
                        gldcolor_t color;

                        if (font->bpp == 1)
                        { /* velocizzo il codice quando NON devo fondere il colore con lo
                             sfondo */
                            color = f_clr;
                        } else
                        {
                            /* qui devo rappresentare il pixel con la giusta opacità,
                               fondendo il colore del testo con quello dello sfondo */
                            gldcolor_Opa_t opacity;

                            opacity = (pxl_value * GLDCOLOR_OPA_100) / colorMask;
                            if (b_clr != GLDCOLOR_NONE)
                            { /* devo fondere con il colore di sfondo impostato */
                                color = gldcolor_Mix (f_clr, b_clr, opacity);
                            } else
                            { /* devo fondere con i pixel attualmente visualizzati */
                                color = gldcolor_Mix (f_clr, disp->GetPxl (x_disp, y_disp), opacity);
                            }
                        }
                        disp->SetPxl (x_disp, y_disp, color);
                    } else if (b_clr != GLDCOLOR_NONE)
                        disp->SetPxl (x_disp, y_disp, b_clr);
                }

                byte_shift -= font->bpp;
                if (byte_shift < 0)
                { /* mi sposto all'inizio del byte successivo */
                    byte_shift = 8 - font->bpp;
                    byte++;
                }
            }
        }
    }
#endif
    return pxl_advance;
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void BmpAlign (const gldimg_BmpHeaderImage_t *bmpih, gldcoord_t *x, gldcoord_t *y, uint8_t align)
{
    /* x align */
    if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_HMID)
        x -= bmpih->BiWidth >> 1;
    else if (GLDCOORD_GET_HALIGN (align) == GLDCOORD_ALIGN_RIGHT)
        x -= bmpih->BiWidth;

    /* y align */
    if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_VMID)
        y -= _ABS_ (bmpih->BiHeight) >> 1;
    else if (GLDCOORD_GET_VALIGN (align) == GLDCOORD_ALIGN_BOTTOM)
        y -= _ABS_ (bmpih->BiHeight);
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void BmpFilter (const gldimg_BmpHeaderImage_t *bmpih, char *palette, const glddraw_BmpFilter_t *filter)
{
    if (filter)
    { /* applico il filtro richiesto alla paletta colore */

        switch (filter->Type)
        {
        case GLDDRAW_BMPFILTER_GRAYSCALE:
            for (int idx = 0; idx < (bmpih->BiClrUsed << 2); idx += 4)
            {
                if (GLDIMG_BMP_GET_COLOR (&palette[idx]) != GLDCOLOR_NONE)
                {
                    uint32_t shade = palette[idx] + palette[idx + 1] + palette[idx + 2];
                    shade /= 3;
                    palette[idx] = shade;
                    palette[idx + 1] = shade;
                    palette[idx + 2] = shade;
                }
            }
            break;

        case GLDDRAW_BMPFILTER_BLEND:
            for (int idx = 0; idx < (bmpih->BiClrUsed << 2); idx += 4)
            {
                if (GLDIMG_BMP_GET_COLOR (&palette[idx]) != GLDCOLOR_NONE)
                {
                    gldcolor_t clr = GLDCOLOR_OR_B (palette[idx]) | GLDCOLOR_OR_G (palette[idx + 1]) | GLDCOLOR_OR_R (palette[idx + 2]);
                    clr = gldcolor_Mix (filter->BlendClr, clr, filter->BlendOpa);
                    palette[idx] = GLDCOLOR_GET_B (clr);
                    palette[idx + 1] = GLDCOLOR_GET_G (clr);
                    palette[idx + 2] = GLDCOLOR_GET_R (clr);
                }
            }
            break;

        case GLDDRAW_BMPFILTER_NONE:
        default:
            break;
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void DrawBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y,
                     const gldimg_BmpHeaderImage_t *bmpih, const char *map_p, const char *palette_p)
{
    gldcoord_Area_t a_view;

    /* get the image covered area */
    a_view.X1 = x;
    a_view.X2 = x + bmpih->BiWidth;
    a_view.Y1 = y;
    a_view.Y2 = y + _ABS_ (bmpih->BiHeight);

    if (gldcoord_AreaIntersection (&a_view, &a_view, &disp->Clip))
    { /* Draw inside the cropped area */
        int line_off;
        /* dimensione in byte di una linea di pixel del bitmap */
        int line_sz;
        /* il pixel più a sinistra che devi cominciare a rappresentare per ogni
           linea, si trova a 'l_byte_off' offset all'interno della linea */
        int l_byte_off;
        /* all'interno del byte che identifichi con 'l_byte_off', il pixel
           comincia al bit di offset 'l_bit_off' */
        int l_bit_off;

        line_sz = (bmpih->BiWidth * bmpih->BiBitCount) >> 3 /* [/ 8] */;
        if (bmpih->BiWidth * bmpih->BiBitCount & 0x07 /* [% 8] */)
            line_sz++;
        /* the line size must be multiple of 4 */
        while (line_sz & 0x03 /* [% 4] */)
            line_sz++;

        { /* configuro le variabili di partenza per il ciclo di
             rappresentazione dell'immagine */
            int x_off, bit_cnt;

            x_off = a_view.X1 - x;
            bit_cnt = (x_off * bmpih->BiBitCount);
            l_byte_off = bit_cnt >> 3 /* [/ 8] */;
            l_bit_off = bit_cnt & 0x07 /* [% 8] */;
        }


        if (bmpih->BiHeight >= 0)
        { /* bottom-up */
            map_p += line_sz * (bmpih->BiHeight - 1);
            line_off = -line_sz;
        } else
        { /* top-down */
            line_off = line_sz;
        }

        {
            glddraw_BmpData_t data;
            bool drawDone = false;
            
            data.BmpIh = bmpih;
            data.AreaView = &a_view;
            data.MapPtr = map_p;
            data.Paletta = palette_p;
            data.LineOff = line_off;
            data.LeftByteOff = l_byte_off;
            data.LeftBitOff = l_bit_off;
            
#if (GLDCFG_DRAW_OPTIMIZATION == 1)
            if (disp == &gldmemfb_Disp)
            {   // ciclo ottimizzato
                glddrawopt_MemRect_t memRect;
                gldmemfb_GetMemRect (&memRect);
                if (glddrawopt_Bmp (&memRect, x, y, &data) == 0)
                    drawDone = true;
            }
#endif
            if (!drawDone)
            {   // ciclo normale non ottimizzato
                glddrawstd_Bmp (disp, x, y, &data);
            }
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Descrizione funzione.
 Arg: - Nessun argomento.
 Ret: - None.
______________________________________________________________________________*/
static void DrawFsBmp (gldhaldisp_t *disp, gldcoord_t x, gldcoord_t y,
                       const gldimg_BmpHeaderImage_t *bmpih, FILE *file, const char *palette_p)
{
    gldcoord_Area_t a_view;

    /* get the image covered area */
    a_view.X1 = x;
    a_view.X2 = x + bmpih->BiWidth;
    a_view.Y1 = y;
    a_view.Y2 = y + _ABS_ (bmpih->BiHeight);

    if (gldcoord_AreaIntersection (&a_view, &a_view, &disp->Clip))
    { /* Draw inside the cropped area */
        int line_sz, line_off, l_byte_off, l_bit_off, map_off;

        line_sz = (bmpih->BiWidth * bmpih->BiBitCount) >> 3 /* [/ 8] */;
        if (bmpih->BiWidth * bmpih->BiBitCount & 0x07 /* [% 8] */)
            line_sz++;
        /* the line size must be multiple of 4 */
        while (line_sz & 0x03 /* [% 4] */)
            line_sz++;

        { /* configuro le variabili di partenza per il ciclo di
             rappresentazione dell'immagine */
            int x_off, bit_cnt;

            x_off = a_view.X1 - x;
            bit_cnt = (x_off * bmpih->BiBitCount);
            l_byte_off = bit_cnt >> 3 /* [/ 8] */;
            l_bit_off = bit_cnt & 0x07 /* [% 8] */;
        }

        map_off = ftell (file);
        if (bmpih->BiHeight >= 0)
        { /* bottom-up */
            map_off += line_sz * (bmpih->BiHeight - 1);
            line_off = -line_sz;
        } else
        { /* top-down */
            line_off = line_sz;
        }
        
        {
            glddraw_BmpFsData_t data;
            bool drawDone = false;
            
            data.BmpIh = bmpih;
            data.AreaView = &a_view;
            data.File = file;
            data.MapOff = map_off;
            data.LineSz = line_sz;
            data.Paletta = palette_p;
            data.LineOff = line_off;
            data.LeftByteOff = l_byte_off;
            data.LeftBitOff = l_bit_off;
            
#if (GLDCFG_DRAW_OPTIMIZATION == 1)
            if (disp == &gldmemfb_Disp)
            {   // ciclo ottimizzato
                glddrawopt_MemRect_t memRect;
                gldmemfb_GetMemRect (&memRect);
                if (glddrawopt_BmpFs (&memRect, x, y, &data) == 0)
                    drawDone = true;
            }
#endif
            if (!drawDone)
            {   // ciclo normale non ottimizzato
                glddrawstd_BmpFs (disp, x, y, &data);
            }
        }
    }
}

/*______________________________________________________________________________ 
 Desc:  Swap two points.
 Arg: - <p1> pointer to the first point.
      - <p2> pointer to the second point.
 Ret: - None.
______________________________________________________________________________*/
static void PointSwap (gldcoord_Point_t *p1, gldcoord_Point_t *p2)
{
    gldcoord_Point_t tmp;
    tmp.X = p1->X;
    tmp.Y = p1->Y;

    p1->X = p2->X;
    p1->Y = p2->Y;

    p2->X = tmp.X;
    p2->Y = tmp.Y;
}
