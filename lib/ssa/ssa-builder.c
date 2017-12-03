#include "scc/ssa/ssa-builder.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

extern void ssa_init_builder(ssa_builder* self, ssa_context* context, ssa_block* block)
{
        self->context = context;
        self->block = block;
        self->uid = 0;
}

extern void ssa_dispose_builder(ssa_builder* self)
{
}

static ssa_value* ssa_build_instr(ssa_builder* self, ssa_instr* i)
{
        S_ASSERT(i && self->block);
        ssa_add_block_instr(self->block, i);
        return ssa_get_instr_var(i);
}

static ssa_value* ssa_build_binop(ssa_builder* self,
        ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        ssa_instr* i = ssa_new_binop(self->context,
                ssa_builder_gen_uid(self), ssa_get_value_type(lhs), opcode, lhs, rhs);
        if (!i)
                return NULL;

        return ssa_build_instr(self, i);
}

static inline ssa_value* ssa_build_arith_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt));
        S_ASSERT(tree_types_are_same(lt, rt));

        return ssa_build_binop(self, opcode, lhs, rhs);
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

static inline ssa_value* ssa_build_bitwise_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_integer(lt) && tree_type_is_integer(rt));
        S_ASSERT(tree_types_are_same(lt, rt));

        return ssa_build_binop(self, opcode, lhs, rhs);
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

static inline ssa_value* ssa_build_cmp_binop(
        ssa_builder* self, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        S_ASSERT(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        S_ASSERT(lt && rt);
        S_ASSERT(tree_type_is_scalar(lt) && tree_type_is_scalar(rt));
        S_ASSERT(tree_types_are_same(lt, rt));

        return ssa_build_binop(self, opcode, lhs, rhs);
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

        ssa_instr* c = ssa_new_cast(self->context, ssa_builder_gen_uid(self), type, operand);
        if (!c)
                return NULL;

        return ssa_build_instr(self, c);
}

extern ssa_value* ssa_build_call(ssa_builder* self, ssa_value* func, dseq* args)
{
        S_ASSERT(func && args);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));
        S_ASSERT(tree_type_is(func_type, TTK_FUNCTION));
        ssa_instr* call = ssa_new_call(self->context, ssa_builder_gen_uid(self), func);
        if (!call)
                return NULL;

        ssa_set_call_args(call, args);
        return ssa_build_instr(self, call);
}

extern ssa_value* ssa_build_getaddr(
        ssa_builder* self,
        tree_type* target_type,
        ssa_value* operand,
        ssa_value* index,
        ssa_value* offset)
{
        if (!index)
                index = ssa_build_zero_u32(self);
        if (!offset)
                offset = ssa_build_zero_u32(self);
        if (!index || !offset)
                return NULL;

        S_ASSERT(tree_type_is_pointer(ssa_get_value_type(operand)));
        S_ASSERT(tree_type_is_integer(ssa_get_value_type(index)));
        S_ASSERT(tree_type_is_integer(ssa_get_value_type(offset)));

        tree_type* value_type = tree_new_pointer_type(ssa_get_tree(self->context), target_type);
        if (!value_type)
                return NULL;

        ssa_instr* i = ssa_new_getaddr(self->context,
                ssa_builder_gen_uid(self), value_type, operand, index, offset);
        if (!i)
                return NULL;

        return ssa_build_instr(self, i);
}

extern ssa_value* ssa_build_alloca(ssa_builder* self, tree_type* type)
{
        tree_type* p = tree_new_pointer_type(ssa_get_tree(self->context), type);
        if (!p)
                return NULL;

        ssa_instr* i = ssa_new_alloca(self->context, ssa_builder_gen_uid(self), p);
        if (!i)
                return NULL;

        return ssa_build_instr(self, i);
}

extern ssa_value* ssa_build_load(ssa_builder* self, ssa_value* what)
{
        tree_type* what_type = ssa_get_value_type(what);
        S_ASSERT(what_type && tree_type_is_object_pointer(what_type));

        tree_type* pointee = tree_get_pointer_target(what_type);
        ssa_instr* i = ssa_new_load(self->context, ssa_builder_gen_uid(self), pointee, what);
        if (!i)
                return NULL;

        return ssa_build_instr(self, i);
}

extern ssa_value* ssa_build_store(ssa_builder* self, ssa_value* what, ssa_value* where)
{
        tree_type* what_type = ssa_get_value_type(what);
        tree_type* where_type = ssa_get_value_type(where);
        S_ASSERT(what_type && where_type);
        S_ASSERT(tree_type_is_object_pointer(where_type));

        tree_type* uu1 = tree_get_unqualified_type(what_type);
        tree_type* uu2 = tree_get_unqualified_type(where_type);
        S_ASSERT(tree_types_are_same(what_type, tree_get_pointer_target(where_type)));


        ssa_instr* i = ssa_new_store(self->context, what, where);
        if (!i)
                return NULL;

        return ssa_build_instr(self, i);
}

extern ssa_value* ssa_build_string(ssa_builder* self, tree_type* type, tree_id id)
{
        return ssa_new_string(self->context, type, id);
}

extern ssa_value* ssa_build_int_constant(ssa_builder* self, tree_type* type, suint64 val)
{
        S_ASSERT(tree_type_is_integer(type));
        uint bits = (uint)tree_get_sizeof(ssa_get_target(self->context), type) * 8;

        avalue v;
        avalue_init_int(&v, bits, tree_type_is_signed_integer(type), val);

        return ssa_new_constant(self->context, type, v);
}

extern ssa_value* ssa_build_sp_constant(ssa_builder* self, tree_type* type, float val)
{
        type = tree_desugar_type(type);
        S_ASSERT(tree_builtin_type_is(type, TBTK_FLOAT));

        avalue v;
        avalue_init_sp(&v, val);

        return ssa_new_constant(self->context, type, v);
}

extern ssa_value* ssa_build_dp_constant(ssa_builder* self, tree_type* type, double val)
{
        type = tree_desugar_type(type);
        S_ASSERT(tree_builtin_type_is(type, TBTK_DOUBLE));

        avalue v;
        avalue_init_dp(&v, val);

        return ssa_new_constant(self->context, type, v);
}

extern ssa_value* ssa_build_zero_u32(ssa_builder* self)
{
        tree_type* u32 = tree_new_builtin_type(ssa_get_tree(self->context), TBTK_UINT32);
        if (!u32)
                return NULL;

        return ssa_build_int_constant(self, u32, 0);
}

extern ssa_value* ssa_build_inc(ssa_builder* self, ssa_value* operand)
{
        tree_type* t = ssa_get_value_type(operand);
        S_ASSERT(t && tree_type_is_arithmetic(t));

        ssa_value* one = ssa_build_int_constant(self, t, 1);
        if (!one)
                return NULL;

        return ssa_build_add(self, operand, one);
}

extern ssa_value* ssa_build_dec(ssa_builder* self, ssa_value* operand)
{
        tree_type* t = ssa_get_value_type(operand);
        S_ASSERT(t && tree_type_is_arithmetic(t));

        ssa_value* one = ssa_build_int_constant(self, t, 1);
        if (!one)
                return NULL;

        return ssa_build_sub(self, operand, one);
}

extern ssa_value* ssa_build_not(ssa_builder* self, ssa_value* operand)
{
        tree_type* t = ssa_get_value_type(operand);
        S_ASSERT(t && tree_type_is_integer(t));
      
        suint64 bits = 8ULL * tree_get_sizeof(ssa_get_target(self->context), t);
        ssa_value* ones = ssa_build_int_constant(self, t, (1ULL << bits) - 1);
        if (!ones)
                return NULL;

        return ssa_build_xor(self, operand, ones);
}

extern ssa_value* ssa_build_log_not(ssa_builder* self, ssa_value* operand)
{
        ssa_value* zero = ssa_build_int_constant(self, ssa_get_value_type(operand), 0);
        if (!zero)
                return NULL;

        return ssa_build_eq(self, operand, zero);
}

extern ssa_value* ssa_build_neg(ssa_builder* self, ssa_value* operand)
{
        ssa_value* zero = ssa_build_int_constant(self, ssa_get_value_type(operand), 0);
        if (!zero)
                return NULL;

        return ssa_build_sub(self, zero, operand);
}

extern ssa_value* ssa_build_neq_zero(ssa_builder* self, ssa_value* operand)
{
        ssa_value* zero = ssa_build_int_constant(self, ssa_get_value_type(operand), 0);
        if (!zero)
                return NULL;

        return ssa_build_neq(self, operand, zero);
}

static ssa_branch* ssa_build_block_exit(ssa_builder* self, ssa_branch* br)
{
        S_ASSERT(br && !ssa_get_block_exit(self->block));
        ssa_set_block_exit(self->block, br);
        return br;
}

extern ssa_value* ssa_build_phi(ssa_builder* self, tree_type* type)
{
        ssa_instr* phi = ssa_new_phi(self->context, ssa_builder_gen_uid(self), type);
        if (!phi)
                return NULL;

        return ssa_build_instr(self, phi);
}

extern ssa_branch* ssa_build_jmp(ssa_builder* self, ssa_value* dest)
{
        ssa_branch* br = ssa_new_jump_branch(self->context, dest);
        if (!br)
                return NULL;

        return ssa_build_block_exit(self, br);
}

extern ssa_branch* ssa_build_if(
        ssa_builder* self, ssa_value* cond, ssa_value* if_true, ssa_value* if_false)
{
        ssa_branch* br = ssa_new_if_branch(self->context, cond, if_true, if_false);
        if (!br)
                return NULL;

        return ssa_build_block_exit(self, br);
}

extern ssa_branch* ssa_build_return(ssa_builder* self, ssa_value* val)
{
        ssa_branch* br = ssa_new_return_branch(self->context, val);
        if (!br)
                return NULL;

        return ssa_build_block_exit(self, br);
}