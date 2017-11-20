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
typedef struct _ssa_branch ssa_branch;
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

extern ssa_value* ssa_build_alloca(ssa_builder* self, tree_type* type);
extern ssa_value* ssa_build_load(ssa_builder* self, ssa_value* what);
extern ssa_value* ssa_build_store(ssa_builder* self, ssa_value* what, ssa_value* where);

extern ssa_value* ssa_build_int_constant(ssa_builder* self, tree_type* type, suint64 val);
extern ssa_value* ssa_build_sp_constant(ssa_builder* self, tree_type* type, float val);
extern ssa_value* ssa_build_dp_constant(ssa_builder* self, tree_type* type, double val);

extern ssa_value* ssa_build_inc(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_dec(ssa_builder* self, ssa_value* operand);

extern ssa_value* ssa_build_not(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_log_not(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_neg(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_neq_zero(ssa_builder* self, ssa_value* operand);

extern ssa_value* ssa_build_phi(ssa_builder* self, tree_type* type);

extern ssa_branch* ssa_build_jmp(ssa_builder* self, ssa_value* dest);
extern ssa_branch* ssa_build_if(
        ssa_builder* self, ssa_value* cond, ssa_value* if_true, ssa_value* if_false);

extern ssa_branch* ssa_build_return(ssa_builder* self, ssa_value* val);

static inline void ssa_builder_set_uid(ssa_builder* self, ssa_id uid)
{
        self->uid = uid;
}

static inline ssa_id ssa_builder_get_uid(const ssa_builder* self)
{
        return self->uid;
}

static inline ssa_id ssa_builder_gen_uid(ssa_builder* self)
{
        return self->uid++;
}

static inline void ssa_builder_set_block(ssa_builder* self, ssa_block* block)
{
        self->block = block;
}

static inline ssa_block* ssa_builder_get_block(const ssa_builder* self)
{
        return self->block;
}

static inline ssa_context* ssa_get_builder_context(const ssa_builder* self)
{
        return self->context;
}

#ifdef __cplusplus
}
#endif

#endif