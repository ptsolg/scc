#ifndef SSAIZE_STMT_H
#define SSAIZE_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssaizer.h"

typedef struct _tree_stmt tree_stmt;

extern ssa_value* ssaize_expr_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_decl_stmt(ssaizer* self, const tree_stmt* stmt);
extern ssa_block* ssaize_compound_stmt(ssaizer* self, const tree_stmt* stmt);

extern bool ssaize_stmt(ssaizer* self, const tree_stmt* stmt);

#ifdef __cplusplus
}
#endif

#endif