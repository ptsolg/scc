#ifndef C_PARSE_MODULE_H
#define C_PARSE_MODULE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "parser.h"

// c99 6.9 translation-unit:
// external-declaration
// translation-unit external-declaration
extern tree_module* c_parse_module(c_parser* self);

#ifdef __cplusplus
}
#endif

#endif