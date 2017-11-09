#ifndef SERROR_H
#define SERROR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef int serrcode;

#define S_NO_ERROR ((serrcode)0)
#define S_ERROR ((serrcode)1)

#define S_SUCCEEDED(E) (((serrcode)(E)) == S_NO_ERROR)
#define S_FAILED(E) (!S_SUCCEEDED(E))

#ifdef __cplusplus
}
#endif

#endif // !SERROR_H
