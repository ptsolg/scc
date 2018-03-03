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

#define EC_NO_ERROR ((errcode)0)
#define EC_ERROR ((errcode)1)

#define EC_SUCCEEDED(E) (((errcode)(E)) == EC_NO_ERROR)
#define EC_FAILED(E) (!EC_SUCCEEDED(E))

#ifdef __cplusplus
}
#endif

#endif
