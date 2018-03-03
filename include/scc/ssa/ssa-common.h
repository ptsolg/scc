#ifndef SSA_COMMON_H
#define SSA_COMMON_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/core/value.h"
#include "scc/tree/tree-common.h"

// a unique identifier
typedef uint ssa_id;

typedef tree_array ssa_array;

#define ssa_init_array tree_init_array

#ifdef __cplusplus
}
#endif

#endif