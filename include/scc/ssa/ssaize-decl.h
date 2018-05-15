#ifndef SSAIZE_DECL_H
#define SSAIZE_DECL_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssaizer.h"

extern void ssaize_type(ssaizer* self, tree_type* type);
extern ssa_value* ssaize_alloca(ssaizer* self, tree_type* t);

extern bool ssaize_var_decl(ssaizer* self, tree_decl* decl);
extern bool ssaize_decl_group(ssaizer* self, const tree_decl* decl);
extern bool ssaize_function_decl(ssaizer* self, tree_decl* decl);
extern bool ssaize_param_decl(ssaizer* self, tree_decl* param);
extern bool ssaize_decl(ssaizer* self, tree_decl* decl);
extern bool ssaize_decl_n(ssaizer* self, tree_decl** decls, size_t n);

#ifdef __cplusplus
}
#endif

#endif