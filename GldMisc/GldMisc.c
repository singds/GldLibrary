//===================================================================== INCLUDES
#include "GldMisc/GldMisc.h"

#include <string.h>

//====================================================================== DEFINES

//=========================================================== PRIVATE PROTOTYPES

//============================================================= STATIC VARIABLES

//============================================================= GLOBAL VARIABLES

//============================================================= GLOBAL FUNCTIONS

/*______________________________________________________________________________ 
 Desc:  In <inAddr> si trovano i byte che rappresentano un valore a 32bit in
    formato Big Endian. Questa funzione legge quel valore a 32bit e lo scrive
    in <v32_p>.
 Arg: - <inAddr> dove si trova il valore 32bit in Big Endian.
        <v32_t> dove salvare il valore 32bit con gli indiani della cpu.
 Ret: - il puntatore <inAddr> avanzato di 4 byte.
______________________________________________________________________________*/
void *gldmisc_Get32Be (void *v32_p, const void *inAddr)
{
    const char *addr = inAddr;
    uint32_t v32;
    v32 = (addr[0] << 24) | (addr[1] << 16) | (addr[2] << 8) | (addr[3] << 0);
    *(uint32_t *)v32_p = v32;
    return (char *)(addr + 4);
}

/*______________________________________________________________________________ 
 Desc:  In <inAddr> si trovano i byte che rappresentano un valore a 32bit in
    formato Little Endian. Questa funzione legge quel valore a 32bit e lo scrive
    in <v32_p>.
 Arg: - <inAddr> dove si trova il valore 32bit in Little Endian.
        <v32_t> dove salvare il valore 32bit con gli indiani della cpu.
 Ret: - il puntatore <inAddr> avanzato di 4 byte.
______________________________________________________________________________*/
void *gldmisc_Get32Le (void *v32_p, const void *inAddr)
{
    const char *addr = inAddr;
    uint32_t v32;
    v32 = (addr[0] << 0) | (addr[1] << 8) | (addr[2] << 16) | (addr[3] << 24);
    *(uint32_t *)v32_p = v32;
    return (char *)(addr + 4);
}

/*______________________________________________________________________________ 
 Desc:  In <inAddr> si trovano i byte che rappresentano un valore a 16bit in
    formato Big Endian. Questa funzione legge quel valore a 16bit e lo scrive
    in <v16_p>.
 Arg: - <inAddr> dove si trova il valore 16bit in Big Endian.
        <v16_t> dove salvare il valore 16bit con gli indiani della cpu.
 Ret: - il puntatore <inAddr> avanzato di 2 byte.
______________________________________________________________________________*/
void *gldmisc_Get16Be (void *v16_p, const void *inAddr)
{
    const char *addr = inAddr;
    uint16_t v16;
    v16 = (addr[0] << 8) | (addr[1] << 0);
    *(uint16_t *)v16_p = v16;
    return (char *)(addr + 2);
}

/*______________________________________________________________________________ 
 Desc:  In <inAddr> si trovano i byte che rappresentano un valore a 16bit in
    formato Little Endian. Questa funzione legge quel valore a 16bit e lo scrive
    in <v16_p>.
 Arg: - <inAddr> dove si trova il valore 16bit in Little Endian.
        <v16_t> dove salvare il valore 16bit con gli indiani della cpu.
 Ret: - il puntatore <inAddr> avanzato di 2 byte.
______________________________________________________________________________*/
void *gldmisc_Get16Le (void *v16_p, const void *inAddr)
{
    const char *addr = inAddr;
    uint16_t v16;
    v16 = (addr[0] << 0) | (addr[1] << 8);
    *(uint16_t *)v16_p = v16;
    return (char *)(addr + 2);
}

/*______________________________________________________________________________ 
 Desc:  In <addr> si trova un byte.
    Questa funzione legge quel valore a 8bit e lo scrive in <v8_p>.
    Quasi completamente inutile. Usata per uniformità con le funzioni precedenti.
 Arg: - <addr> dove si trova il byte.
        <v8_p> dove salvare il byte.
 Ret: - il puntatore <addr> avanzato di 1 byte.
______________________________________________________________________________*/
void *gldmisc_Get8 (void *v8_p, const void *addr)
{
    *(char *)v8_p = *(char *)addr;
    return ((char *)addr + 1);
}

/*______________________________________________________________________________ 
 Desc:  Legge una stringa di <size> byte da <addr> e la copia in <char_p>
 Arg: - <char_p> dove scrivere la stringa che viene lette.
        <addr> dove si trova la stringa.
        <size> la dimensione in byte della stringa.
 Ret: - Il puntatore <addr> avanzato di <size> byte (quindi si salta la stringa).
______________________________________________________________________________*/
void *gldmisc_GetString (void *char_p, const void *addr, size_t size)
{
    memcpy (char_p, addr, size);
    return ((char *)addr + size);
}

//============================================================ PRIVATE FUNCTIONS
