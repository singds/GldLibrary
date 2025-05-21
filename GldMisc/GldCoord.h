#ifndef GLDCOORD_H_INCLUDED
#define GLDCOORD_H_INCLUDED

//===================================================================== INCLUDES
#include "ComDef.h"

//====================================================================== DEFINES
#define GLDCOORD_ALIGN_LEFT   (0x00 << 0)
#define GLDCOORD_ALIGN_RIGHT  (0x01 << 0)
#define GLDCOORD_ALIGN_HMID   (0x02 << 0)
#define GLDCOORD_ALIGN_TOP    (0x00 << 2)
#define GLDCOORD_ALIGN_BOTTOM (0x01 << 2)
#define GLDCOORD_ALIGN_VMID   (0x02 << 2)

#define GLDCOORD_ALIGN_LT (GLDCOORD_ALIGN_LEFT | GLDCOORD_ALIGN_TOP)
#define GLDCOORD_ALIGN_LM (GLDCOORD_ALIGN_LEFT | GLDCOORD_ALIGN_VMID)
#define GLDCOORD_ALIGN_LB (GLDCOORD_ALIGN_LEFT | GLDCOORD_ALIGN_BOTTOM)
#define GLDCOORD_ALIGN_RT (GLDCOORD_ALIGN_RIGHT | GLDCOORD_ALIGN_TOP)
#define GLDCOORD_ALIGN_RM (GLDCOORD_ALIGN_RIGHT | GLDCOORD_ALIGN_VMID)
#define GLDCOORD_ALIGN_RB (GLDCOORD_ALIGN_RIGHT | GLDCOORD_ALIGN_BOTTOM)
#define GLDCOORD_ALIGN_MT (GLDCOORD_ALIGN_HMID | GLDCOORD_ALIGN_TOP)
#define GLDCOORD_ALIGN_MM (GLDCOORD_ALIGN_HMID | GLDCOORD_ALIGN_VMID)
#define GLDCOORD_ALIGN_MB (GLDCOORD_ALIGN_HMID | GLDCOORD_ALIGN_BOTTOM)

/* get the alignment components from a complete alignment description var */
#define GLDCOORD_GET_HALIGN(align) (align & 0x03)
#define GLDCOORD_GET_VALIGN(align) (align & 0x0C)

#define GLDCOORD_MAX        INT16_MAX
#define GLDCOORD_MIN        INT16_MIN

typedef enum
{
    GLDCOORD_ORI_0,     // nessuna rotazione
    GLDCOORD_ORI_90,    // rotazione di 90° in senso orario
    GLDCOORD_ORI_180,   // rotazione di 180° in senso orario
    GLDCOORD_ORI_270,   // rotazione di 270° in senso orario
} gldcoord_Ori_e; // orientation

typedef enum
{
    GLDCOORD_HRALIGN_LL = 0,    // allineamento orizzontale Left-Left
    GLDCOORD_HRALIGN_LM,        // allineamento orizzontale Left-Mid point
    GLDCOORD_HRALIGN_LR,        // allineamento orizzontale Left-Right

    GLDCOORD_HRALIGN_ML,        // allineamento orizzontale Mid point-Left
    GLDCOORD_HRALIGN_MM,        // allineamento orizzontale Mid point-Mid point
    GLDCOORD_HRALIGN_MR,        // allineamento orizzontale Mid point-Right

    GLDCOORD_HRALIGN_RL,        // allineamento orizzontale Right-Left
    GLDCOORD_HRALIGN_RM,        // allineamento orizzontale Right-Mid point
    GLDCOORD_HRALIGN_RR,        // allineamento orizzontale Right-Right
} gldcoord_HRAlign_e;

typedef enum
{
    GLDCOORD_VRALIGN_TT = 0,    // allineamento orizzontale Top-Top
    GLDCOORD_VRALIGN_TM,        // allineamento orizzontale Top-Mid point
    GLDCOORD_VRALIGN_TB,        // allineamento orizzontale Top-Bottom

    GLDCOORD_VRALIGN_MT,        // allineamento orizzontale Mid point-Top
    GLDCOORD_VRALIGN_MM,        // allineamento orizzontale Mid point-Mid point
    GLDCOORD_VRALIGN_MB,        // allineamento orizzontale Mid point-Bottom

    GLDCOORD_VRALIGN_BT,        // allineamento orizzontale Bottom-Top
    GLDCOORD_VRALIGN_BM,        // allineamento orizzontale Bottom-Mid point
    GLDCOORD_VRALIGN_BB,        // allineamento orizzontale Bottom-Bottom
} gldcoord_VRAlign_e;


typedef int16_t gldcoord_t;

typedef struct
{
    gldcoord_t X;
    gldcoord_t Y;
} gldcoord_Point_t;

typedef struct
{
    gldcoord_t X1;
    gldcoord_t Y1;
    gldcoord_t X2;
    gldcoord_t Y2;
} gldcoord_Area_t;

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES
bool gldcoord_PointIsIn (gldcoord_t x, gldcoord_t y, const gldcoord_Area_t *area_p);
bool gldcoord_AreaIntersection (gldcoord_Area_t *uni_p, const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p);
void gldcoord_AreaJoin (gldcoord_Area_t *join_p, const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p);
void gldcoord_AreaCopy (gldcoord_Area_t *dest, const gldcoord_Area_t *src);
int16_t gldcoord_AreaCmp (const gldcoord_Area_t *a1_p, const gldcoord_Area_t *a2_p);
uint8_t gldcoord_AreaDiff (const gldcoord_Area_t *a, const gldcoord_Area_t *b, gldcoord_Area_t *out);

gldcoord_t gldcoord_AreaHeight (const gldcoord_Area_t *area_p);
gldcoord_t gldcoord_AreaWidth (const gldcoord_Area_t *area_p);
int64_t gldcoord_AreaArea (const gldcoord_Area_t *area_p);

#endif /* GLDCOORD_H_INCLUDED */
