//===================================================================== INCLUDES
#include "GldMisc/GldCoord.h"

#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  Verifica se il punto appartiene all'area indicata. X1-Y1 sono compresi
        mentre X2-Y2 non lo sono.
 Arg: - <x> <y> coordinate punto.
      - <area_p> puntatore ad area.
 Ret: - true: il punto è interno all'area.
      - false: il punto è esterno all'area.
______________________________________________________________________________*/
bool gldcoord_PointIsIn (gldcoord_t x, gldcoord_t y, const gldcoord_Area_t *area_p)
{
    bool is_in = false;

    if (x >= area_p->X1 && x < area_p->X2 && y >= area_p->Y1 && y < area_p->Y2)
        is_in = true;

    return is_in;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni le coordinate dell'area di intersezione tra le due ('a1_p' e 'a2_p').
 Arg: - <uni_p>[out] viene scritta con le coordinate dell'area di intersezione
            fra a1_p e a2_p.
        <a1_p>[in] prima area da intersecare.
        <a2_p>[in] seconda area da intersecare.
 Ret: - True se l'intersezione non è nulla. False altrimenti.
______________________________________________________________________________*/
bool gldcoord_AreaIntersection (gldcoord_Area_t *uni_p, const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p)
{
    bool overlap = false;
    gldcoord_t x1, x2, y1, y2;

    x1 = _MAX_ (a1_p->X1, a2_p->X1);
    y1 = _MAX_ (a1_p->Y1, a2_p->Y1);
    x2 = _MIN_ (a1_p->X2, a2_p->X2);
    y2 = _MIN_ (a1_p->Y2, a2_p->Y2);

    if (x1 < x2 && y1 < y2)
        overlap = true;
    
    if (uni_p) {
        uni_p->X1 = x1;
        uni_p->X2 = x2;
        uni_p->Y1 = y1;
        uni_p->Y2 = y2;
    }
    return overlap;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni le coordinate della più piccola area che contiene entrambe ('a1_p' e 'a2_p').
 Arg: - <join_p>[out] viene scritta con le coordinate della più piccola area che
            contiene entrambe le aree successive.
        <a1_p>[in] prima area da "sommare".
        <a2_p>[in] seconda area da "sommare".
 Ret: - None.
______________________________________________________________________________*/
void gldcoord_AreaJoin (gldcoord_Area_t *join_p, const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p)
{
    if (gldcoord_AreaArea (a2_p) == 0)
        gldcoord_AreaCopy (join_p, a1_p);
    else if (gldcoord_AreaArea (a1_p) == 0)
        gldcoord_AreaCopy (join_p, a2_p);
    else
    {
        join_p->X1 = _MIN_ (a1_p->X1, a2_p->X1);
        join_p->Y1 = _MIN_ (a1_p->Y1, a2_p->Y1);
        join_p->X2 = _MAX_ (a1_p->X2, a2_p->X2);
        join_p->Y2 = _MAX_ (a1_p->Y2, a2_p->Y2);
    }
}

/*______________________________________________________________________________ 
 Desc:  Copia l'area sorgente nell'area di destinazione.
 Arg: - <dest>[out] area di destinazione.
        <src>[in] area sorgente.
 Ret: - None.
______________________________________________________________________________*/
void gldcoord_AreaCopy (gldcoord_Area_t *dest, const gldcoord_Area_t *src)
{
    memcpy (dest, src, sizeof (gldcoord_Area_t));
}

/*______________________________________________________________________________ 
 Desc:  Effettua la comparazione delle aree indicate.
 Arg: - <a1_p>[in] prima area da confrontare.
        <a2_p>[in] seconda area da confrontare.
 Ret: - 0 : le aree sono coincidenti.
        1 : a1 contiene a2.
        2 : a2 contiene a1.
        -1: nessuna delle precedenti condizioni. le aree si intersecano.
______________________________________________________________________________*/
int16_t gldcoord_AreaCmp (const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p)
{
    int16_t ret;

    if (memcmp (a1_p, a2_p, sizeof (gldcoord_Area_t)) == 0)
    { /* le aree sono uguali */
        ret = 0;
    } else
    {
        gldcoord_Area_t area_cmp;
        gldcoord_AreaIntersection (&area_cmp, a1_p, a2_p);
        if (memcmp (&area_cmp, a2_p, sizeof (gldcoord_Area_t)) == 0)
        { /* a1 contiene a2 */
            ret = 1;
        } else if (memcmp (&area_cmp, a1_p, sizeof (gldcoord_Area_t)) == 0)
        { /* a2 contiene a1 */
            ret = 2;
        } else
        { /* nessuna delle precedenti condizioni */
            ret = -1;
        }
    }

    return ret;
}

/*______________________________________________________________________________ 
 Desc:  Subtract area <b> from area <a>. The result from the subraction can be
    a variable number of areas between 0 an 4. This depends on the way the
    two rectangles are layed out. 0 when <a> is a subarea of <b>.
 Arg: - <a>[in] area pointer.
        <b>[in] area pointer.
        <out>[out] an area array with at least 3 elements. This is written with
            the areas (between 0 and 4) resulting from the subtraction.
 Ret: - Number of written areas in <out>.
______________________________________________________________________________*/
uint8_t gldcoord_AreaDiff (const gldcoord_Area_t *a, const gldcoord_Area_t *b, gldcoord_Area_t *out)
{
    uint8_t count = 0;
    gldcoord_Area_t intersec;
    
    if (!gldcoord_AreaIntersection (&intersec, a, b)) {
        // <a> e <b> non si intersecano
        gldcoord_AreaCopy (out, a);
        count = 1;
    } else {
        // <a> e <b> si intersecano
        if (a->Y1 < intersec.Y1) { // top part
            out[count].X1 = a->X1;
            out[count].X2 = a->X2;
            out[count].Y1 = a->Y1;
            out[count].Y2 = intersec.Y1;
            count++;
        }
        if (a->X1 < intersec.X1) { // left part
            out[count].X1 = a->X1;
            out[count].X2 = intersec.X1;
            out[count].Y1 = intersec.Y1;
            out[count].Y2 = intersec.Y2;
            count++;
        }
        if (a->X2 > intersec.X2) { // right part
            out[count].X1 = intersec.X2;
            out[count].X2 = a->X2;
            out[count].Y1 = intersec.Y1;
            out[count].Y2 = intersec.Y2;
            count++;
        }
        if (a->Y2 > intersec.Y2) { // bottom part
            out[count].X1 = a->X1;
            out[count].X2 = a->X2;
            out[count].Y1 = intersec.Y2;
            out[count].Y2 = a->Y2;
            count++;
        }
    }
    
    return count;
}

/*______________________________________________________________________________ 
 Desc:  Ottieni l'atezza di un'area.
 Arg: - <area_p> puntatore ad area.
 Ret: - altezza dell'area.
______________________________________________________________________________*/
gldcoord_t gldcoord_AreaHeight (const gldcoord_Area_t *area_p)
{
    return (area_p->Y2 - area_p->Y1);
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la larghezza di un'area.
 Arg: - <area_p> puntatore ad area.
 Ret: - larghezza dell'area.
______________________________________________________________________________*/
gldcoord_t gldcoord_AreaWidth (const gldcoord_Area_t *area_p)
{
    return (area_p->X2 - area_p->X1);
}

/*______________________________________________________________________________ 
 Desc:  Ottieni la superficie (pixel quadri) di un'area.
 Arg: - <area_p> puntatore ad area.
 Ret: - superficie in pixel quadrati dell'area.
______________________________________________________________________________*/
int64_t gldcoord_AreaArea (const gldcoord_Area_t *area_p)
{
    int64_t area;
    area = ((int32_t)area_p->X2 - (int32_t)area_p->X1);
    area *= ((int32_t)area_p->Y2 - (int32_t)area_p->Y1);
    return area;
}

//============================================================ PRIVATE FUNCTIONS
