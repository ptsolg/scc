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

#define CHASH(S) CHASHL(S, strlen(S))

#ifdef __cplusplus
}
#endif

#endif // !CCOMMON_H
