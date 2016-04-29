// global functions
//

//*  includes
//
#include "stdafx.h"

extern "C" {
#include <stdlib.h>
}

#include "globals.hh"
#include "error.hh"

//*  variables
//
static eap::out_of_mem X;

//*  functions
//
/*void *operator new(size_t size)
{
    void *p;

    p = malloc(size);
    if (!p) throw(X);

    return p;
}

void *operator new [](size_t size)
{
    void *p;

    p = malloc(size);
    if (!p) throw(X);

    return p;
}

void operator delete(void *p)
{
    free(p);
}

void operator delete [](void *p)
{
    free(p);
}
//*/