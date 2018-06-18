#include "scc/ssa/ssa-builder.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-context.h"

extern void ssa_init_builder(ssa_builder* self, ssa_context* context, ssa_instr* pos)
{
        self->context = context;
        self->pos = pos;
}

extern void ssa_dispose_builder(ssa_builder* self)
{
}

static ssa_value* ssa_insert_instr(ssa_instr* pos, ssa_instr* instr, bool insert_after)
{
        assert(instr && pos);
        if (insert_after)
                ssa_add_instr_after(instr, pos);
        else
                ssa_add_instr_before(instr, pos);
        return ssa_get_instr_var(instr);
}

static ssa_value* ssa_build_binop(ssa_builder* self,
        ssa_binop_kind opcode, tree_type* restype, ssa_value* lhs, ssa_value* rhs)
{
        ssa_instr* i = ssa_new_binop(self->context, restype, opcode, lhs, rhs);
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}

static inline ssa_value* ssa_build_arith_binop(
        ssa_builder* self, ssa_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        assert(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        assert(lt && rt);
        assert(tree_type_is_arithmetic(lt) && tree_type_is_arithmetic(rt));

        return ssa_build_binop(self, opcode, lt, lhs, rhs);
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

extern ssa_value* ssa_build_ptradd(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        ssa_value* pointer = lhs;
        ssa_value* offset = rhs;
        if (tree_type_is_pointer(ssa_get_value_type(rhs)))
        {
                pointer = rhs;
                offset = lhs;
        }

        assert(tree_type_is_pointer(ssa_get_value_type(pointer))
                && tree_type_is_integer(ssa_get_value_type(offset)));
        return ssa_build_binop(self, SBIK_PTRADD, ssa_get_value_type(pointer), pointer, offset);
}

extern ssa_value* ssa_build_sub(ssa_builder* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_build_arith_binop(self, SBIK_SUB, lhs, rhs);
}

static inline ssa_value* ssa_build_bitwise_binop(
        ssa_builder* self, ssa_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        assert(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        assert(lt && rt);
        assert(tree_type_is_integer(lt) && tree_type_is_integer(rt));

        return ssa_build_binop(self, opcode, lt, lhs, rhs);
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
        ssa_builder* self, ssa_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        assert(lhs && rhs);
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        assert(lt && rt);
        assert(tree_type_is_scalar(lt) && tree_type_is_scalar(rt));

        return ssa_build_binop(self, opcode,
                tree_new_builtin_type(ssa_get_tree(self->context), TBTK_INT32), lhs, rhs);
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

extern ssa_value* ssa_build_cast(ssa_builder* self, tree_type* to, ssa_value* operand)
{
        return ssa_build_cast_ex(self, self->pos, to, operand, false);
}

extern ssa_value* ssa_build_cast_ex(ssa_builder* self, ssa_instr* pos, tree_type* to, ssa_value* operand, bool insert_after)
{
        assert(to && tree_type_is_scalar(to));
        to = tree_desugar_type(to);
        tree_type* from = tree_desugar_type(ssa_get_value_type(operand));

        if (tree_compare_types(to, from) == TTEK_EQ)
                return operand;

        if (tree_type_is(from, TTK_FUNCTION) && tree_type_is_function_pointer(to))
        {
                ssa_set_value_type(operand, to);
                return operand;
        }

        ssa_instr* c = ssa_new_cast(self->context, to, operand);
        if (!c)
                return NULL;

        return ssa_insert_instr(pos, c, insert_after);
}

extern ssa_value* ssa_build_cast_to_pvoid(ssa_builder* self, ssa_value* operand)
{
        tree_type* pvoid = tree_new_pointer_type(self->context->tree,
                tree_new_builtin_type(self->context->tree, TBTK_VOID));
        return ssa_build_cast(self, pvoid, operand);
}

extern ssa_value* ssa_build_call_0(ssa_builder* self, ssa_value* func)
{
        return ssa_build_call_n(self, func, NULL, 0);
}

extern ssa_value* ssa_build_call_1(ssa_builder* self, ssa_value* func, ssa_value* a1)
{
        ssa_value* args[1] = { a1 };
        return ssa_build_call_n(self, func, args, 1);
}

extern ssa_value* ssa_build_call_2(ssa_builder* self, ssa_value* func, ssa_value* a1, ssa_value* a2)
{
        ssa_value* args[2] = { a1, a2 };
        return ssa_build_call_n(self, func, args, 2);
}

extern ssa_value* ssa_build_call_3(ssa_builder* self, ssa_value* func, ssa_value* a1, ssa_value* a2, ssa_value* a3)
{
        ssa_value* args[3] = { a1, a2, a3 };
        return ssa_build_call_n(self, func, args, 3);
}

extern ssa_value* ssa_build_call_n(ssa_builder* self, ssa_value* func, ssa_value** args, size_t n)
{
        return ssa_build_call_n_ex(self, self->pos, func, args, n, false);
}

extern ssa_value* ssa_build_call_n_ex(
        ssa_builder* self, ssa_instr* pos, ssa_value* func, ssa_value** args, size_t n, bool insert_after)
{
        assert(func);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));
        assert(tree_type_is(func_type, TTK_FUNCTION));

        ssa_instr* call = ssa_new_call(self->context, tree_get_func_type_result(func_type), func);
        if (!call)
                return NULL;

        for (size_t i = 0; i < n; i++)
                ssa_add_instr_operand(call, self->context, args[i]);

        return ssa_insert_instr(pos, call, insert_after);
}

extern ssa_value* ssa_build_alloca(ssa_builder* self, tree_type* type)
{
        return ssa_build_alloca_ex(self, self->pos, type, false);
}

extern ssa_value* ssa_build_alloca_ex(
        ssa_builder* self, ssa_instr* pos, tree_type* type, bool insert_after)
{
        tree_type* p = tree_new_pointer_type(ssa_get_tree(self->context), type);
        if (!p)
                return NULL;

        ssa_instr* i = ssa_new_alloca(self->context, p);
        if (!i)
                return NULL;

        return ssa_insert_instr(pos, i, insert_after);
}

extern ssa_value* ssa_build_load(ssa_builder* self, ssa_value* what)
{
        tree_type* what_type = ssa_get_value_type(what);
        assert(what_type && tree_type_is_object_pointer(what_type));

        tree_type* pointee = tree_get_pointer_target(what_type);
        ssa_instr* i = ssa_new_load(self->context, pointee, what);
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}

extern ssa_value* ssa_build_store(ssa_builder* self, ssa_value* what, ssa_value* where)
{
        tree_type* what_type = ssa_get_value_type(what);
        tree_type* where_type = ssa_get_value_type(where);
        assert(what_type && where_type);
        assert(tree_type_is_object_pointer(where_type));

        ssa_instr* i = ssa_new_store(self->context, what, where);
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}

extern ssa_value* ssa_build_getfieldaddr(
        ssa_builder* self, ssa_value* record, tree_decl* field)
{
        assert(tree_type_is(ssa_get_value_type(record), TTK_POINTER));
        assert(tree_get_field_record(field) == 
                tree_get_decl_type_entity(tree_desugar_type(
                        tree_get_pointer_target(ssa_get_value_type(record)))));
  
        tree_type* field_ptr = tree_new_pointer_type(
                ssa_get_tree(self->context), tree_get_decl_type(field));

        ssa_instr* i = ssa_new_getfieldaddr(
                self->context,
                field_ptr,
                record,
                tree_get_field_index(field));
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}

extern ssa_value* ssa_build_undef(ssa_builder* self, tree_type* type)
{
        return ssa_new_undef(self->context, type);
}

extern ssa_value* ssa_build_string(ssa_builder* self, tree_type* type, tree_id id)
{
        return ssa_new_string(self->context, type, id);
}

extern ssa_value* ssa_build_int_constant(ssa_builder* self, tree_type* type, uint64_t val)
{
        assert(tree_type_is_integer(type) || tree_type_is_pointer(type));
        uint bits = (uint)tree_get_sizeof(ssa_get_target(self->context), type) * 8;

        avalue v;
        avalue_init_int(&v, bits, tree_type_is_signed_integer(type), val);

        return ssa_new_constant(self->context, type, &v);
}

extern ssa_value* ssa_build_i32_constant(ssa_builder* self, int val)
{
        tree_type* i32 = tree_new_builtin_type(ssa_get_tree(self->context), TBTK_INT32);
        if (!i32)
                return NULL;

        return ssa_build_int_constant(self, i32, val);
}

extern ssa_value* ssa_build_u32_constant(ssa_builder* self, unsigned val)
{
        tree_type* u32 = tree_new_builtin_type(ssa_get_tree(self->context), TBTK_UINT32);
        if (!u32)
                return NULL;

        return ssa_build_int_constant(self, u32, val);
}

extern ssa_value* ssa_build_size_t_constant(ssa_builder* self, size_t val)
{
        tree_type* size_type = tree_new_size_type(ssa_get_tree(self->context));
        if (!size_type)
                return NULL;

        return ssa_build_int_constant(self, size_type, val);
}

extern ssa_value* ssa_build_sp_constant(ssa_builder* self, tree_type* type, float val)
{
        type = tree_desugar_type(type);
        assert(tree_builtin_type_is(type, TBTK_FLOAT));

        avalue v;
        avalue_init_sp(&v, val);

        return ssa_new_constant(self->context, type, &v);
}

extern ssa_value* ssa_build_dp_constant(ssa_builder* self, tree_type* type, double val)
{
        type = tree_desugar_type(type);
        assert(tree_builtin_type_is(type, TBTK_DOUBLE));

        avalue v;
        avalue_init_dp(&v, val);

        return ssa_new_constant(self->context, type, &v);
}

extern ssa_value* ssa_build_zero(ssa_builder* self, tree_type* type)
{
        assert(tree_type_is_scalar(type));
        if (tree_builtin_type_is(type, TBTK_FLOAT))
                return ssa_build_sp_constant(self, type, 0.0f);
        else if (tree_builtin_type_is(type, TBTK_DOUBLE))
                return ssa_build_dp_constant(self, type, 0.0);
        return ssa_build_int_constant(self, type, 0);
}

extern ssa_value* ssa_build_one(ssa_builder* self, tree_type* type)
{
        assert(tree_type_is_arithmetic(type));
        if (tree_builtin_type_is(type, TBTK_FLOAT))
                return ssa_build_sp_constant(self, type, 1.0f);
        else if (tree_builtin_type_is(type, TBTK_DOUBLE))
                return ssa_build_sp_constant(self, type, 1.0);
        return ssa_build_int_constant(self, type, 1);
}

extern ssa_value* ssa_build_not(ssa_builder* self, ssa_value* operand)
{
        tree_type* t = ssa_get_value_type(operand);
        assert(t && tree_type_is_integer(t));
      
        uint64_t bits = 8ULL * tree_get_sizeof(ssa_get_target(self->context), t);
        ssa_value* ones = ssa_build_int_constant(self, t, (1ULL << bits) - 1);
        if (!ones)
                return NULL;

        return ssa_build_xor(self, operand, ones);
}

extern ssa_value* ssa_build_log_not(ssa_builder* self, ssa_value* operand)
{
        ssa_value* zero = ssa_build_zero(self, ssa_get_value_type(operand));
        if (!zero)
                return NULL;

        return ssa_build_eq(self, operand, zero);
}

extern ssa_value* ssa_build_neg(ssa_builder* self, ssa_value* operand)
{
        ssa_value* zero = ssa_build_zero(self, ssa_get_value_type(operand));
        if (!zero)
                return NULL;

        return ssa_build_sub(self, zero, operand);
}

extern ssa_value* ssa_build_neq_zero(ssa_builder* self, ssa_value* operand)
{
        tree_type* type = ssa_get_value_type(operand);
        assert(tree_type_is_scalar(type));

        ssa_value* zero = ssa_build_zero(self, type);
        if (!zero)
                return NULL;

        return ssa_build_neq(self, operand, zero);
}

extern ssa_value* ssa_build_phi(ssa_builder* self, tree_type* type)
{
        return ssa_build_phi_ex(self, self->pos, type, false);
}

extern ssa_value* ssa_build_phi_ex(ssa_builder* self, ssa_instr* pos, tree_type* type, bool insert_after)
{
        ssa_instr* phi = ssa_new_phi(self->context, type);
        if (!phi)
                return NULL;

        return ssa_insert_instr(pos, phi, insert_after);
}

static ssa_instr* ssa_insert_block_terminator(ssa_instr* pos, ssa_instr* terminator, bool insert_after)
{
        if (insert_after)
                ssa_add_instr_after(terminator, pos);
        else
                ssa_add_instr_before(terminator, pos);

        return terminator;
}

extern ssa_instr* ssa_build_inderect_jmp(ssa_builder* self, ssa_value* dest)
{
        SSA_ASSERT_VALUE(dest, SVK_LABEL);

        ssa_instr* jmp = ssa_new_inderect_jump(self->context, dest);
        if (!jmp)
                return NULL;

        return ssa_insert_block_terminator(self->pos, jmp, false);
}

extern ssa_instr* ssa_build_cond_jmp(
        ssa_builder* self, ssa_value* cond, ssa_value* if_true, ssa_value* if_false)
{
        SSA_ASSERT_VALUE(if_true, SVK_LABEL);
        SSA_ASSERT_VALUE(if_false, SVK_LABEL);

        ssa_instr* jmp = ssa_new_conditional_jump(self->context, cond, if_true, if_false);
        if (!jmp)
                return NULL;

        return ssa_insert_block_terminator(self->pos, jmp, false);
}

extern ssa_instr* ssa_build_switch_instr(ssa_builder* self, ssa_value* cond, ssa_value* otherwise)
{
        ssa_instr* switch_instr = ssa_new_switch_instr(self->context, cond);
        if (!switch_instr)
                return NULL;

        ssa_add_instr_operand(switch_instr, self->context, otherwise);
        return ssa_insert_block_terminator(self->pos, switch_instr, false);
}

extern ssa_instr* ssa_build_return(ssa_builder* self, ssa_value* val)
{
        ssa_instr* ret = val 
                ? ssa_new_ret(self->context, val)
                : ssa_new_ret_void(self->context);
        if (!ret)
                return NULL;

        return ssa_insert_block_terminator(self->pos, ret, false);
}

extern ssa_value* ssa_build_function_param(ssa_builder* self, tree_type* type)
{
        return ssa_new_param(self->context, type);
}

static ssa_value* ssa_build_atomic_rmw_instr(
        ssa_builder* self,
        ssa_atomic_rmw_instr_kind kind,
        ssa_value* ptr,
        ssa_value* val,
        ssa_memorder_kind ordering)
{
        tree_type* ptr_type = ssa_get_value_type(ptr);
        tree_type* val_type = ssa_get_value_type(val);
        assert(tree_type_is_integer(val_type));
        assert(tree_compare_types(val_type, tree_get_pointer_target(ptr_type)) != TTEK_NEQ);

        ssa_instr* i = ssa_new_atomic_rmw_instr(
                self->context, val_type, kind, ptr, val, ordering);
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}

extern ssa_value* ssa_build_atomic_add(
        ssa_builder* self, ssa_value* ptr, ssa_value* val, ssa_memorder_kind ordering)
{
        return ssa_build_atomic_rmw_instr(self, SARIK_ADD, ptr, val, ordering);
}

extern ssa_value* ssa_build_atomic_xchg(
        ssa_builder* self, ssa_value* ptr, ssa_value* val, ssa_memorder_kind ordering)
{
        return ssa_build_atomic_rmw_instr(self, SARIK_XCHG, ptr, val, ordering);
}

extern ssa_value* ssa_build_fence_instr(
        ssa_builder* self, ssa_syncscope_kind syncscope, ssa_memorder_kind ordering)
{
        ssa_instr* fence = ssa_new_fence_instr(self->context, syncscope, ordering);
        return fence ? ssa_insert_instr(self->pos, fence, false) : NULL;
}

extern ssa_value* ssa_build_atomic_cmpxchg(
        ssa_builder* self,
        ssa_value* ptr,
        ssa_value* expected,
        ssa_value* desired,
        ssa_memorder_kind success_ordering,
        ssa_memorder_kind failure_ordering)
{
        assert(tree_compare_types(
                ssa_get_value_type(expected), ssa_get_value_type(desired)) != TTEK_NEQ);
        assert(tree_compare_types(
                tree_get_pointer_target(ssa_get_value_type(ptr)),  ssa_get_value_type(desired)) != TTEK_NEQ);

        ssa_instr* i = ssa_new_atomic_cmpxchg_instr(self->context,
                tree_get_builtin_type(self->context->tree, TBTK_INT32),
                ptr, expected, desired,
                success_ordering, failure_ordering);
        if (!i)
                return NULL;

        return ssa_insert_instr(self->pos, i, false);
}