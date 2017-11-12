#ifndef SSA_BUILDER_H
#define SSA_BUILDER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_block ssa_block;
typedef struct _tree_type tree_type;

typedef struct _ssa_builder
{
        ssa_context* context;
        ssa_block* block;
        ssa_id uid;
} ssa_builder;

extern void ssa_init_builder(ssa_builder* self, ssa_context* context, ssa_block* block);
extern void ssa_dispose_builder(ssa_builder* self);

extern ssa_value* ssa_build_mul(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_div(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_mod(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_add(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_sub(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_shl(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_shr(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_and(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_or(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_xor(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_le(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_gr(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_leq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_geq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_eq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_neq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_cast(ssa_builder* self, tree_type* type, ssa_value* operand);
extern ssa_value* ssa_build_call(ssa_builder* self, ssa_value* res, dseq* operands);
extern ssa_value* ssa_build_getaddr(ssa_builder* self, ssa_value* operand, ssize offset);
extern ssa_value* ssa_build_getptrval(
        ssa_builder* self, ssa_value* operand, ssize index, ssize offset);

static inline void ssa_set_builder_uid(ssa_builder* self, ssa_id uid)
{
        self->uid = uid;
}

static inline ssa_id ssa_get_builder_uid(const ssa_builder* self)
{
        return self->uid;
}

#ifdef __cplusplus
}
#endif

#endif