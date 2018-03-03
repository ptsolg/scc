#ifndef SCC_CORE_ERROR_H
#define SCC_CORE_ERROR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef int errcode;

#define S_NO_ERROR ((errcode)0)
#define S_ERROR ((errcode)1)

#define S_SUCCEEDED(E) (((errcode)(E)) == S_NO_ERROR)
#define S_FAILED(E) (!S_SUCCEEDED(E))

#ifdef __cplusplus
}
#endif

#endif
