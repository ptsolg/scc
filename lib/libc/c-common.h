#ifndef CCOMMON_H
#define CCOMMON_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-limits.h"
#include <libscl/hash.h>
#include <libscl/sstring.h>

#ifndef CHASHL
#define _CHASH_SEED 391585969
#define CHASHL(S, L) ((hval)murmurhash3_86_32((s), _CHASH_SEED, (int)(L))
#endif

#define CHASH(S) CHASHL(S, strlen(S))

#ifdef __cplusplus
}
#endif

#endif // !CCOMMON_H