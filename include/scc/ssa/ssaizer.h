#ifndef SSAIZER_H
#define SSAIZER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _ssa_context ssa_context;

typedef struct _ssaizer
{
        ssa_context* context;
} ssaizer;

extern void ssaizer_init(ssaizer* self);

#ifdef __cplusplus
}
#endif

#endif