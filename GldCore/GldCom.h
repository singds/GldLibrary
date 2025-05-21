#ifndef GLDCOM_H_INCLUDED
#define GLDCOM_H_INCLUDED

//===================================================================== INCLUDES
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//====================================================================== DEFINES
#ifndef _NELEMENTS_
#define _NELEMENTS_(arg) (sizeof (arg) / sizeof (arg[0]))
#endif

#ifndef _ASSERT
#ifdef _ASSERT_ON
#define _ASSERT(x) {(x)?(void)0:__breakpoint(0);}
#else
#define _ASSERT(x) {(void)(x);}
#endif
#endif

#ifndef _SIZEOF_FIELD
#define _SIZEOF_FIELD(type, member) sizeof((((type *)0)->member))
#endif

#define _CEIL_DIV_(a, b) (((a) + ((b) - 1) / (b))
#define _MIN_(a, b) ((a) < (b) ? (a) : (b))
#define _MAX_(a, b) ((a) > (b) ? (a) : (b))
#define _ABS_(a)    ((a) < 0 ? -(a) : (a))

#define GET_BIT(val, n) ((val >> n) & 0x01)

#define _GRADIGIRO_  360
#define _PI_         3.141592654 // Pigreco

#define _STATUS_OK_   0  // Operation succeeded
#define _STATUS_FAIL_ -1 // Unspecified error

#define B0  0x00000001
#define B1  0x00000002
#define B2  0x00000004
#define B3  0x00000008
#define B4  0x00000010
#define B5  0x00000020
#define B6  0x00000040
#define B7  0x00000080
#define B8  0x00000100
#define B9  0x00000200
#define B10 0x00000400
#define B11 0x00000800
#define B12 0x00001000
#define B13 0x00002000
#define B14 0x00004000
#define B15 0x00008000
#define B16 0x00010000
#define B17 0x00020000
#define B18 0x00040000
#define B19 0x00080000
#define B20 0x00100000
#define B21 0x00200000
#define B22 0x00400000
#define B23 0x00800000
#define B24 0x01000000
#define B25 0x02000000
#define B26 0x04000000
#define B27 0x08000000
#define B28 0x10000000
#define B29 0x20000000
#define B30 0x40000000
#define B31 0x80000000

//============================================================= GLOBAL VARIABLES

//============================================================ GLOBAL PROTOTYPES

//============================================================= INLINE FUNCTIONS

#endif /* GLDCOM_H_INCLUDED */
