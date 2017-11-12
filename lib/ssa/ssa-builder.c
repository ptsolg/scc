#include "scc/ssa/ssa-builder.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-instr.h"

extern void ssa_init_builder(ssa_builder* self, ssa_context* context, ssa_block* block)
{
        self->context = context;
        self->block = block;
        self->uid = 0;
}

extern void ssa_dispose_builder(ssa_builder* self)
{
}

extern ssa_value* ssa_build_value(ssa_builder* self, tree_type* type, ssa_instr* init)
{
        S_ASSERT(init && self->block);

        ssa_value* v = ssa_new_var(self->context, type, self->uid, init);
        if (!v)
                return NULL;
        // todo
        self->uid++;
        return v;
}

static void ssa_builder_check_arith_binop_operands(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt));
        S_ASSERT(tree_types_are_same(lt, rt));
}

static inline ssa_value* ssa_build_arith_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        ssa_builder_check_arith_binop_operands(self, lhs, rhs);
        ssa_instr* i = ssa_new_binop(self->context, opcode, lhs, rhs);
        if (!i)
                return NULL;

        return ssa_build_value(self, ssa_get_value_type(lhs), i);
}

extern ssa_value* ssa_build_mul(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_MUL, lhs, rhs);
}

extern ssa_value* ssa_build_div(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_DIV, lhs, rhs);
}

extern ssa_value* ssa_build_mod(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_MOD, lhs, rhs);
}

extern ssa_value* ssa_build_add(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_ADD, lhs, rhs);
}

extern ssa_value* ssa_build_sub(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_SUB, lhs, rhs);
}

static void ssa_builder_check_bitwise_binop_operands(
        ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_integer(lt) && tree_type_is_integer(rt));
        S_ASSERT(tree_types_are_same(lt, rt));
}

static inline ssa_value* ssa_build_bitwise_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        ssa_builder_check_bitwise_binop_operands(self, lhs, rhs);
        ssa_instr* i = ssa_new_binop(self->context, opcode, lhs, rhs);
        if (!i)
                return NULL;

        return ssa_build_value(self, ssa_get_value_type(lhs), i);
}

extern ssa_value* ssa_build_shl(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_bitwise_binop(self, SBIK_SHL, lhs, rhs);
}

extern ssa_value* ssa_build_shr(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_bitwise_binop(self, SBIK_SHR, lhs, rhs);
}

extern ssa_value* ssa_build_and(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_bitwise_binop(self, SBIK_AND, lhs, rhs);
}

extern ssa_value* ssa_build_or(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_bitwise_binop(self, SBIK_OR, lhs, rhs);
}

extern ssa_value* ssa_build_xor(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_bitwise_binop(self, SBIK_XOR, lhs, rhs);
}

static void ssa_builder_check_cmp_binop_operands(
        ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_scalar(lt) && tree_type_is_scalar(rt));
        S_ASSERT(tree_types_are_same(lt, rt));
}

static inline ssa_value* ssa_build_cmp_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        ssa_builder_check_cmp_binop_operands(self, lhs, rhs);
        ssa_instr* i = ssa_new_binop(self->context, opcode, lhs, rhs);
        if (!i)
                return NULL;

        return ssa_build_value(self, ssa_get_value_type(lhs), i);
}

extern ssa_value* ssa_build_le(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_LE, lhs, rhs);
}

extern ssa_value* ssa_build_gr(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_GR, lhs, rhs);
}

extern ssa_value* ssa_build_leq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_LEQ, lhs, rhs);
}

extern ssa_value* ssa_build_geq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_GEQ, lhs, rhs);
}

extern ssa_value* ssa_build_eq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_EQ, lhs, rhs);
}

extern ssa_value* ssa_build_neq(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_cmp_binop(self, SBIK_NEQ, lhs, rhs);
}

extern ssa_value* ssa_build_cast(ssa_builder* self, tree_type* type, ssa_value* operand)
{
        S_ASSERT(type && tree_type_is_scalar(type));
        ssa_instr* c = ssa_new_cast(self->context, type, operand);
        if (!c)
                return NULL;

        return ssa_build_value(self, type, c);
}

extern ssa_value* ssa_build_call(ssa_builder* self, ssa_value* res, dseq* operands)
{
        S_UNREACHABLE();
        return NULL;
}

extern ssa_value* ssa_build_getaddr(ssa_builder* self, ssa_value* operand, ssize offset)
{
        S_UNREACHABLE();
        return NULL;
}

extern ssa_value* ssa_build_getptrval(
        ssa_builder* self, ssa_value* operand, ssize index, ssize offset)
{
        S_UNREACHABLE();
        return NULL;
}
