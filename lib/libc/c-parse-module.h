#ifndef CPARSE_MODULE_H
#define CPARSE_MODULE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"

// c99 6.9 translation-unit:
//      external-declaration
//      translation-unit external-declaration
extern tree_module* cparse_module(cparser* self);

#ifdef __cplusplus
}
#endif

#endif // !CPARSE_MODULE_H