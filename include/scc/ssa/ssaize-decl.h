#ifndef SSAIZE_DECL_H
#define SSAIZE_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssaizer.h"

extern bool ssaize_var_decl(ssaizer* self, const tree_decl* decl);
extern bool ssaize_decl_group(ssaizer* self, const tree_decl* decl);
extern bool ssaize_function_decl(ssaizer* self, tree_decl* decl);
extern bool ssaize_decl(ssaizer* self, tree_decl* decl);

#ifdef __cplusplus
}
#endif

#endif