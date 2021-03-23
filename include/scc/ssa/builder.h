#ifndef SSA_BUILDER_H
#define SSA_BUILDER_H

#include "common.h"
#include "instr.h" // ssa_memorder_kind ...

typedef struct _ssa_context ssa_context;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_branch ssa_branch;
typedef struct _ssa_instr ssa_instr;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;

typedef struct _ssa_builder
{
        ssa_context* context;
        ssa_instr* pos;
} ssa_builder;

extern void ssa_init_builder(ssa_builder* self, ssa_context* context, ssa_instr* pos);
extern void ssa_dispose_builder(ssa_builder* self);

extern ssa_value* ssa_build_mul(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_div(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_mod(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_add(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
extern ssa_value* ssa_build_ptradd(ssa_builder* self, ssa_value* lhs, ssa_value* rhs);
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
extern ssa_value* ssa_build_cast(ssa_builder* self, tree_type* to, ssa_value* operand);
extern ssa_value* ssa_build_cast_ex(ssa_builder* self, ssa_instr* pos, tree_type* to, ssa_value* operand, bool insert_after);
extern ssa_value* ssa_build_cast_to_pvoid(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_cast_to_size_t(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_cast_to_ptrdiff_t(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_call_0(ssa_builder* self, ssa_value* func);
extern ssa_value* ssa_build_call_1(ssa_builder* self, ssa_value* func, ssa_value* a1);
extern ssa_value* ssa_build_call_2(ssa_builder* self, ssa_value* func, ssa_value* a1, ssa_value* a2);
extern ssa_value* ssa_build_call_3(ssa_builder* self, ssa_value* func, ssa_value* a1, ssa_value* a2, ssa_value* a3);
extern ssa_value* ssa_build_call_n(ssa_builder* self, ssa_value* func, ssa_value** args, size_t n);
extern ssa_value* ssa_build_call_n_ex(
        ssa_builder* self, ssa_instr* pos, ssa_value* func, ssa_value** args, size_t n, bool insert_after);

extern ssa_value* ssa_build_alloca(ssa_builder* self, tree_type* type);
extern ssa_value* ssa_build_alloca_ex(
        ssa_builder* self, ssa_instr* pos, tree_type* type, bool insert_after);

extern ssa_value* ssa_build_load(ssa_builder* self, ssa_value* what);
extern ssa_value* ssa_build_store(ssa_builder* self, ssa_value* what, ssa_value* where);
extern ssa_value* ssa_build_getfieldaddr(
        ssa_builder* self, ssa_value* record, tree_decl* field);

extern ssa_value* ssa_build_undef(ssa_builder* self, tree_type* type);
extern ssa_value* ssa_build_string(ssa_builder* self, tree_type* type, tree_id id);
extern ssa_value* ssa_build_int_constant(ssa_builder* self, tree_type* type, uint64_t val);
extern ssa_value* ssa_build_i32_constant(ssa_builder* self, int val);
extern ssa_value* ssa_build_u32_constant(ssa_builder* self, unsigned val);
extern ssa_value* ssa_build_size_t_constant(ssa_builder* self, size_t val);
extern ssa_value* ssa_build_ptrdiff_t_constant(ssa_builder* self, ptrdiff_t val);
extern ssa_value* ssa_build_sp_constant(ssa_builder* self, tree_type* type, float val);
extern ssa_value* ssa_build_dp_constant(ssa_builder* self, tree_type* type, double val);
extern ssa_value* ssa_build_zero(ssa_builder* self, tree_type* type);
extern ssa_value* ssa_build_one(ssa_builder* self, tree_type* type);

extern ssa_value* ssa_build_not(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_log_not(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_neg(ssa_builder* self, ssa_value* operand);
extern ssa_value* ssa_build_neq_zero(ssa_builder* self, ssa_value* operand);

extern ssa_value* ssa_build_phi(ssa_builder* self, tree_type* type);
extern ssa_value* ssa_build_phi_ex(ssa_builder* self, ssa_instr* pos, tree_type* type, bool insert_after);

extern ssa_instr* ssa_build_inderect_jmp(ssa_builder* self, ssa_value* dest);
extern ssa_instr* ssa_build_cond_jmp(
        ssa_builder* self, ssa_value* cond, ssa_value* if_true, ssa_value* if_false);
extern ssa_instr* ssa_build_switch_instr(ssa_builder* self, ssa_value* cond, ssa_value* otherwise);

extern ssa_instr* ssa_build_return(ssa_builder* self, ssa_value* val);

extern ssa_value* ssa_build_function_param(ssa_builder* self, tree_type* type);

extern ssa_value* ssa_build_atomic_add(
        ssa_builder* self, ssa_value* ptr, ssa_value* val, ssa_memorder_kind ordering);

extern ssa_value* ssa_build_atomic_xchg(
        ssa_builder* self, ssa_value* ptr, ssa_value* val, ssa_memorder_kind ordering);

extern ssa_value* ssa_build_fence_instr(
        ssa_builder* self, ssa_syncscope_kind syncscope, ssa_memorder_kind ordering);

extern ssa_value* ssa_build_atomic_cmpxchg(
        ssa_builder* self,
        ssa_value* ptr,
        ssa_value* expected,
        ssa_value* desired,
        ssa_memorder_kind success_ordering,
        ssa_memorder_kind failure_ordering);

#endif