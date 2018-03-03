#ifndef SCC_CORE_STRING_H
#define SCC_CORE_STRING_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "common.h"

extern bool streq(const char* a, const char* b, const char* ignore);
extern char* strprecat(char* string, const char* other);
extern char* strcatn(char* string, size_t n, ...);
extern char* strprecatn(char* string, size_t n, ...);
extern char* strwrap(const char* prefix, char* string, const char* suffix);
extern char* strend(char* string);
extern const char* cstrend(const char* string);
extern char* strfill(char* string, int v, size_t n);

#ifdef __cplusplus
}
#endif

#endif
