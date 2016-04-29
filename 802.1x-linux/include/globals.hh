// 802.1x global definitions
//
//	$Id: globals.hh,v 1.6 2002/08/25 17:28:25 rw Exp $
//
#ifndef _8021x_globals_hh
#define _8021x_globals_hh

//*  type macros
//
//	'typedef' is not used, because that would
//	require exzessive casting because of C++'s
//	strong typing
//
#define ui8 unsigned char
#define ui16 unsigned short
#define ui32 unsigned long

//*  includes
//
extern "C" {
#include <stddef.h>
}

void *operator new(size_t size);
void *operator new [](size_t size);

void operator delete(void *p);
void operator delete [](void *p);

#endif
