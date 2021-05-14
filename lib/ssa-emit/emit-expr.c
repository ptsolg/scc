#include "emitter.h"
#include "scc/core/num.h"
#include "scc/ssa/block.h"
#include "scc/ssa/context.h"
#include "scc/ssa/module.h"

extern ssa_value* ssa_emit_alloca(ssa_function_emitter* self, tree_type* type)
{
        ssa_value* v = ssa_build_alloca_ex(&self->builder,
                self->alloca_insertion_pos,
                type,
                tree_get_alignof(self->context->target, type),
                true);
        if (v)
                self->alloca_insertion_pos = ssa_get_var_instr(v);
        return v;
}

extern ssa_value* ssa_emit_load(ssa_function_emitter* self, ssa_value* what)
{
        return ssa_build_load(&self->builder, what);
}

extern ssa_value* ssa_emit_store(ssa_function_emitter* self, ssa_value* what, ssa_value* where)
{
        return ssa_build_store(&self->builder, what, where);
}

static ssa_value* ssa_emit_add(ssa_function_emitter* self, ssa_value* lhs, ssa_value* rhs)
{
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        return tree_type_is_pointer(lt) || tree_type_is_pointer(rt)
                ? ssa_build_ptradd(&self->builder, lhs, rhs)
                : ssa_build_add(&self->builder, lhs, rhs);
}

static ssa_value* ssa_emit_sub(ssa_function_emitter* self, ssa_value* lhs, ssa_value* rhs)
{
        tree_type* lt = ssa_get_value_type(lhs);
        tree_type* rt = ssa_get_value_type(rhs);
        ssa_value* neg;

        if (tree_type_is_pointer(lt) && tree_type_is_pointer(rt))
        {
                tree_type* pointee = tree_get_pointer_target(ssa_get_value_type(lhs));
                if (!(lhs = ssa_build_cast_to_ptrdiff_t(&self->builder, lhs)))
                        return NULL;
                if (!(rhs = ssa_build_cast_to_ptrdiff_t(&self->builder, rhs)))
                        return NULL;

                ssa_value* diff = ssa_build_sub(&self->builder, lhs, rhs);
                if (!diff)
                        return NULL;

                ssa_value* pointee_size = ssa_build_ptrdiff_t_constant(
                        &self->builder, tree_get_sizeof(self->context->target, pointee));
                if (!pointee_size)
                        return NULL;

                return ssa_build_div(&self->builder, diff, pointee_size);
        }
        else if (tree_type_is_pointer(lt))
        {
                if (!(neg = ssa_build_neg(&self->builder, rhs)))
                        return NULL;
                return ssa_build_ptradd(&self->builder, lhs, neg);
        }
        else if (tree_type_is_pointer(rt))
        {
                if (!(neg = ssa_build_neg(&self->builder, lhs)))
                        return NULL;
                return ssa_build_ptradd(&self->builder, rhs, neg);
        }

        return ssa_build_sub(&self->builder, lhs, rhs);
}

static inline ssa_value* ssa_emit_assign(ssa_function_emitter* self, ssa_value* lhs, ssa_value* rhs)
{
        return ssa_emit_store(self, rhs, lhs) ? rhs : NULL;
}

static inline ssa_value* _ssa_emit_binary_expr(ssa_function_emitter*, tree_binop_kind, ssa_value*, ssa_value*);

static inline ssa_value* ssa_emit_compound_assign(ssa_function_emitter* self,
        tree_binop_kind prefix, ssa_value* lhs, ssa_value* rhs)
{
        ssa_value* val = ssa_emit_load(self, lhs);
        if (!val)
                return NULL;

        tree_type* rhs_type = ssa_get_value_type(rhs);
        tree_type* lhs_type = tree_get_pointer_target(ssa_get_value_type(lhs));
        if (!tree_type_is_pointer(lhs_type))
                if (!(val = ssa_build_cast(&self->builder, rhs_type, val)))
                        return NULL;

        TREE_ASSERT_BINOP_KIND(prefix);
        ssa_value* result = _ssa_emit_binary_expr(self, prefix, val, rhs);
        if (!result)
                return NULL;

        if (!(result = ssa_build_cast(&self->builder, lhs_type, result)))
                return NULL;

        return ssa_emit_assign(self, lhs, result);
}

static inline ssa_value* _ssa_emit_binary_expr(
        ssa_function_emitter* self, tree_binop_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        TREE_ASSERT_BINOP_KIND(opcode);
        if (opcode >= TBK_LE && opcode <= TBK_NEQ)
        {
                tree_type* lt = ssa_get_value_type(lhs);
                tree_type* rt = ssa_get_value_type(rhs);
                if (tree_type_is_pointer(lt) && ssa_get_value_kind(rhs) == SVK_CONSTANT)
                {
                        if (num_is_zero(ssa_get_constant_value(rhs)))
                                if (!(rhs = ssa_build_zero(&self->builder, lt)))
                                        return NULL;
                }
                else if (tree_type_is_pointer(rt) && ssa_get_value_kind(lhs) == SVK_CONSTANT)
                {
                        if (num_is_zero(ssa_get_constant_value(lhs)))
                                if (!(lhs = ssa_build_zero(&self->builder, rt)))
                                        return NULL;
                }
        }

        ssa_builder* b = &self->builder;
        switch (opcode)
        {
                case TBK_MUL:        return ssa_build_mul(b, lhs, rhs);
                case TBK_DIV:        return ssa_build_div(b, lhs, rhs);
                case TBK_MOD:        return ssa_build_mod(b, lhs, rhs);
                case TBK_ADD:        return ssa_emit_add(self, lhs, rhs);
                case TBK_SUB:        return ssa_emit_sub(self, lhs, rhs);
                case TBK_SHL:        return ssa_build_shl(b, lhs, rhs);
                case TBK_SHR:        return ssa_build_shr(b, lhs, rhs);
                case TBK_OR:         return ssa_build_or(b, lhs, rhs);
                case TBK_AND:        return ssa_build_and(b, lhs, rhs);
                case TBK_XOR:        return ssa_build_xor(b, lhs, rhs);
                case TBK_LE:         return ssa_build_le(b, lhs, rhs);
                case TBK_LEQ:        return ssa_build_leq(b, lhs, rhs);
                case TBK_GR:         return ssa_build_gr(b, lhs, rhs);
                case TBK_GEQ:        return ssa_build_geq(b, lhs, rhs);
                case TBK_EQ:         return ssa_build_eq(b, lhs, rhs);
                case TBK_NEQ:        return ssa_build_neq(b, lhs, rhs);
                case TBK_ASSIGN:     return ssa_emit_assign(self, lhs, rhs);
                case TBK_ADD_ASSIGN: return ssa_emit_compound_assign(self, TBK_ADD, lhs, rhs);
                case TBK_SUB_ASSIGN: return ssa_emit_compound_assign(self, TBK_SUB, lhs, rhs);
                case TBK_MUL_ASSIGN: return ssa_emit_compound_assign(self, TBK_MUL, lhs, rhs);
                case TBK_DIV_ASSIGN: return ssa_emit_compound_assign(self, TBK_DIV, lhs, rhs);
                case TBK_MOD_ASSIGN: return ssa_emit_compound_assign(self, TBK_MOD, lhs, rhs);
                case TBK_SHL_ASSIGN: return ssa_emit_compound_assign(self, TBK_SHL, lhs, rhs);
                case TBK_SHR_ASSIGN: return ssa_emit_compound_assign(self, TBK_SHR, lhs, rhs);
                case TBK_AND_ASSIGN: return ssa_emit_compound_assign(self, TBK_AND, lhs, rhs);
                case TBK_OR_ASSIGN:  return ssa_emit_compound_assign(self, TBK_OR, lhs, rhs);
                case TBK_XOR_ASSIGN: return ssa_emit_compound_assign(self, TBK_XOR, lhs, rhs);

                case TBK_COMMA:
                        return rhs;

                case TBK_LOG_AND:
                case TBK_LOG_OR:
                        assert(0 && "Logical operations are not handled here");
                default:
                        assert(0 && "Invalid binop kind");
                        return NULL;
        }
}

typedef struct
{
        ssa_block* block;
        ssa_instr* phi;
} ssa_cond_expr_exit;

static void ssa_init_cond_expr_exit(ssa_cond_expr_exit* self, ssa_function_emitter* fe, tree_type* phi_type)
{
        self->block = ssa_new_function_block(fe);
        ssa_value* v = ssa_build_phi_ex(
                &fe->builder, ssa_get_block_instrs_end(self->block), phi_type, false);
        self->phi = ssa_get_var_instr(v);
}

static inline ssa_value* ssa_emit_log_expr_lhs(
        ssa_function_emitter* self, ssa_cond_expr_exit* exit, const tree_expr* expr)
{
        if (!tree_expr_is(expr, TEK_BINARY)
                || (!tree_binop_is(expr, TBK_LOG_AND) && !tree_binop_is(expr, TBK_LOG_OR)))
        {
                return ssa_emit_expr_as_condition(self, expr);
        }

        ssa_value* lhs_cond = ssa_emit_log_expr_lhs(self, exit, tree_get_binop_lhs(expr));
        ssa_value* lhs_label = ssa_get_block_label(self->block);
        if (!lhs_cond)
                return NULL;

        ssa_block* current = ssa_new_function_block(self);

        // finish prev block
        ssa_block* on_true = tree_binop_is(expr, TBK_LOG_OR) ? exit->block : current;
        ssa_block* on_false = tree_binop_is(expr, TBK_LOG_OR) ? current : exit->block;
        if (!ssa_emit_cond_jmp(self, lhs_cond, on_true, on_false))
                return false;

        ssa_add_phi_operand(exit->phi, self->context, lhs_cond, lhs_label);
        ssa_enter_block(self, current);
        return ssa_emit_expr_as_condition(self, tree_get_binop_rhs(expr));;
}

static ssa_value* ssa_emit_log_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_cond_expr_exit exit;
        ssa_init_cond_expr_exit(&exit, self, tree_get_expr_type(expr));

        ssa_value* last_cond = ssa_emit_log_expr_lhs(self, &exit, expr);
        ssa_value* last_label = ssa_get_block_label(self->block);
        if (!last_cond)
                return NULL;

        if (!ssa_emit_jmp(self, exit.block))
                return false;

        ssa_add_phi_operand(exit.phi, self->context, last_cond, last_label);
        ssa_enter_block(self, exit.block);
        ssa_emit_block(self, exit.block);
        return ssa_get_instr_var(exit.phi);
}

extern ssa_value* ssa_emit_binary_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_binop_kind k = tree_get_binop_kind(expr);
        if (k == TBK_LOG_AND || k == TBK_LOG_OR)
                return ssa_emit_log_expr(self, expr);

        ssa_value* lhs = ssa_emit_expr(self, tree_get_binop_lhs(expr));
        if (!lhs)
                return NULL;

        ssa_value* rhs = ssa_emit_expr(self, tree_get_binop_rhs(expr));
        if (!rhs)
                return NULL;

        return _ssa_emit_binary_expr(self, k, lhs, rhs);
}

static ssa_value* ssa_emit_inc_dec(
        ssa_function_emitter* self, ssa_value* operand, bool inc, bool prefix)
{
        ssa_value* val = ssa_emit_load(self, operand);
        if (!val)
                return NULL;

        tree_type* val_type = ssa_get_value_type(val);
        ssa_value* one = tree_type_is_pointer(val_type)
                ? ssa_build_size_t_constant(&self->builder, 1)
                : ssa_build_one(&self->builder, val_type);
        if (!one)
                return NULL;

        ssa_value* new_val = inc
                ? ssa_emit_add(self, val, one)
                : ssa_emit_sub(self, val, one);
        if (!new_val || !ssa_emit_assign(self, operand, new_val))
                return NULL;

        return prefix ? new_val : val;
}

extern ssa_value* ssa_emit_unary_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* operand = ssa_emit_expr(self, tree_get_unop_operand(expr));
        if (!operand)
                return NULL;

        tree_unop_kind k = tree_get_unop_kind(expr);
        switch (k)
        {
                case TUK_POST_INC:
                case TUK_PRE_INC:
                        return ssa_emit_inc_dec(self, operand, true, k == TUK_PRE_INC);
                case TUK_POST_DEC:
                case TUK_PRE_DEC:
                        return ssa_emit_inc_dec(self, operand, false, k == TUK_PRE_DEC);
                case TUK_MINUS:
                        return ssa_build_neg(&self->builder, operand);
                case TUK_NOT:
                        return ssa_build_not(&self->builder, operand);
                case TUK_LOG_NOT:
                        return ssa_build_log_not(&self->builder, operand);
                case TUK_DEREFERENCE:
                        return tree_expr_is_lvalue(expr) ? operand : ssa_emit_load(self, operand);
                case TUK_ADDRESS:
                case TUK_PLUS:
                        return operand;

                default:
                        assert(0 && "Invalid unary operator");
                        return NULL;
        }
}

static ssa_value* ssa_emit_intrin_call_expr(
        ssa_function_emitter* self, const tree_expr* expr, const tree_decl* intrin)
{
        ssa_value* args[32];
        size_t num_args = tree_get_call_args_size(expr);
        assert(num_args < 32 && "update args size");

        for (size_t i = 0; i < num_args; i++)
                if (!(args[i] = ssa_emit_expr(self, tree_get_call_arg(expr, i))))
                        return NULL;

        switch (tree_get_func_builtin_kind(intrin))
        {
                case TFBK_ATOMIC_CMPXCHG_32_WEAK_SEQ_CST:
                        return ssa_build_atomic_cmpxchg(&self->builder,
                                args[0], args[1], args[2], SMK_SEQ_CST, SMK_SEQ_CST);

                case TFBK_ATOMIC_ADD_FETCH_32_SEQ_CST:
                        return ssa_build_atomic_add(&self->builder, args[0], args[1], SMK_SEQ_CST);

                case TFBK_ATOMIC_XCHG_32_SEQ_CST:
                        return ssa_build_atomic_xchg(&self->builder, args[0], args[1], SMK_SEQ_CST);

                case TFBK_ATOMIC_FENCE_ST_SEQ_CST:
                        return ssa_build_fence_instr(&self->builder, SSK_SINGLE_THREAD, SMK_SEQ_CST);

                case TFBK_ATOMIC_FENCE_ST_ACQ:
                        return ssa_build_fence_instr(&self->builder, SSK_SINGLE_THREAD, SMK_ACQUIRE);

                case TFBK_ATOMIC_FENCE_ST_REL:
                        return ssa_build_fence_instr(&self->builder, SSK_SINGLE_THREAD, SMK_RELEASE);

                case TFBK_ATOMIC_FENCE_ST_ACQ_REL:
                        return ssa_build_fence_instr(&self->builder, SSK_SINGLE_THREAD, SMK_ACQ_REL);

                default:
                        assert(0 && "Unknown intrinsic instruction");
                        return NULL;
        }
}

static bool is_intrin_instr(const tree_decl* d)
{
        if (!tree_decl_is(d, TDK_FUNCTION) || !tree_func_is_builtin(d))
                return false;
        tree_function_builtin_kind k = tree_get_func_builtin_kind(d);
        return k != TFBK_VA_START && k != TFBK_FRAME_ADDRESS;
}

extern ssa_value* ssa_emit_call_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_expr* lhs = tree_get_call_lhs(expr);
        tree_expr* lhs_base = tree_desugar_expr(lhs);
        if (tree_expr_is(lhs_base, TEK_DECL))
        {
                tree_decl* func = tree_get_decl_expr_entity(lhs_base);
                if (is_intrin_instr(func))
                        return ssa_emit_intrin_call_expr(self, expr, func);
        }

        ssa_value* func = ssa_emit_expr(self, lhs);
        if (!func)
                return NULL;

        ssa_array args;
        ssa_init_array(&args);
        TREE_FOREACH_CALL_ARG(expr, it)
        {
                ssa_value* arg = ssa_emit_expr(self, *it);
                if (!arg)
                {
                        ssa_dispose_array(self->context, &args);
                        return NULL;
                }

                ssa_value** last = ssa_reserve_object(self->context, &args, sizeof(ssa_value*));
                *last = arg;
        }

        ssa_value* call = ssa_build_call_n(&self->builder, func, (ssa_value**)args.data, args.size);
        ssa_dispose_array(self->context, &args);
        return call;
}

extern ssa_value* ssa_emit_subscript_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_expr* pointer = tree_get_subscript_lhs(expr);
        tree_expr* index = tree_get_subscript_rhs(expr);

        // 0[pointer]
        if (tree_type_is_pointer(tree_get_expr_type(index)))
        {
                tree_expr* tmp = pointer;
                pointer = index;
                index = tmp;
        }

        ssa_value* ssa_pointer = ssa_emit_expr(self, pointer);
        if (!ssa_pointer)
                return NULL;

        ssa_value* ssa_index = ssa_emit_expr(self, index);
        if (!ssa_index)
                return NULL;

        ssa_value* element_ptr = ssa_build_ptradd(&self->builder, ssa_pointer, ssa_index);
        if (!element_ptr)
                return NULL;

        return tree_expr_is_lvalue(expr)
                ? element_ptr
                : ssa_emit_load(self, element_ptr);
}

static bool ssa_emit_conditional_expr_branch(
        ssa_function_emitter* self, ssa_cond_expr_exit* exit, const tree_expr* branch)
{
        ssa_value* val = ssa_emit_expr(self, branch);
        if (!val)
                return false;

        ssa_add_phi_operand(exit->phi, self->context, val, ssa_get_block_label(self->block));
        return ssa_emit_jmp(self, exit->block);
}

extern ssa_value* ssa_emit_conditional_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* cond = ssa_emit_expr_as_condition(self, tree_get_conditional_condition(expr));
        if (!cond)
                return NULL;

        ssa_cond_expr_exit exit;
        ssa_init_cond_expr_exit(&exit, self, tree_get_expr_type(tree_get_conditional_lhs(expr)));

        ssa_block* lblock = ssa_new_function_block(self);
        ssa_block* rblock = ssa_new_function_block(self);
        if (!ssa_emit_cond_jmp(self, cond, lblock, rblock))
                return false;

        ssa_enter_block(self, lblock);
        if (!ssa_emit_conditional_expr_branch(self, &exit, tree_get_conditional_lhs(expr)))
                return NULL;

        ssa_enter_block(self, rblock);
        if (!ssa_emit_conditional_expr_branch(self, &exit, tree_get_conditional_rhs(expr)))
                return NULL;

        ssa_enter_block(self, exit.block);
        return ssa_get_instr_var(exit.phi);
}

extern ssa_value* ssa_emit_integer_literal(ssa_function_emitter* self, const tree_expr* expr)
{
        return ssa_build_int_constant(&self->builder,
                tree_get_expr_type(expr), tree_get_integer_literal(expr));
}

extern ssa_value* ssa_emit_character_literal(ssa_function_emitter* self, const tree_expr* expr)
{
        return ssa_build_int_constant(&self->builder,
                tree_get_expr_type(expr), tree_get_character_literal(expr));
}

extern ssa_value* ssa_emit_floating_literal(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_type* type = tree_get_expr_type(expr);
        const struct num* value = tree_get_floating_literal_cvalue(expr);

        return tree_builtin_type_is(type, TBTK_FLOAT)
                ? ssa_build_sp_constant(&self->builder, type, num_f32(value))
                : ssa_build_dp_constant(&self->builder, type, num_f64(value));
}

extern ssa_value* ssa_emit_string_literal(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* string = ssa_build_string(&self->builder,
                tree_get_expr_type(expr), tree_get_string_literal(expr));
        if (!string)
                return NULL;

        ssa_add_module_global(self->module_emitter->module, string);
        return string;
}

static ssa_value* ssa_emit_global_decl_ptr(ssa_function_emitter* self, tree_decl* decl)
{
        ssa_value* global = ssa_get_global_decl(self->module_emitter, decl);
        if (!global)
        {
                return ssa_emit_global_decl(self->module_emitter, decl)
                        ? ssa_get_global_decl(self->module_emitter, decl)
                        : NULL;
        }
        return global;
}

extern ssa_value* ssa_emit_decl_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_decl* decl = tree_get_decl_expr_entity(expr);
        if (!decl)
                return NULL;

        if (tree_decl_is(decl, TDK_ENUMERATOR))
                return ssa_build_i32_constant(&self->builder,
                        num_i64(tree_get_enumerator_cvalue(decl)));

        ssa_value* def = tree_decl_is_global(decl)
                ? ssa_emit_global_decl_ptr(self, decl)
                : ssa_get_def(self, decl);
        if (!def)
                return NULL;

        return tree_expr_is_lvalue(expr) || tree_decl_is(decl, TDK_FUNCTION)
                ? def
                : ssa_emit_load(self, def);
}

static ssa_value* ssa_emit_member_expr_lhs(ssa_function_emitter* self, const tree_expr* lhs)
{
        ssa_value* lhs_val = ssa_emit_expr(self, lhs);
        if (!lhs_val)
                return NULL;

        tree_type* t = tree_get_expr_type(lhs);
        if (tree_expr_is_rvalue(lhs) && tree_type_is_record(t))
        {
                ssa_value* result = ssa_emit_alloca(self, t);
                return result && ssa_emit_store(self, lhs_val, result)
                        ? result : NULL;
        }
        return lhs_val;
}

extern ssa_value* ssa_emit_member_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* lhs = ssa_emit_member_expr_lhs(self, tree_get_member_expr_lhs(expr));
        if (!lhs)
                return NULL;

        ssa_value* field_addr;
        tree_decl* member = tree_get_member_expr_decl(expr);
        tree_decl* rec = tree_get_decl_type_entity(
                tree_desugar_type(tree_get_pointer_target(ssa_get_value_type(lhs))));

        if (tree_record_is_union(rec))
        {
                tree_type* ptr = tree_new_pointer_type(
                        ssa_get_tree(self->context), tree_get_decl_type(member));
                field_addr = ssa_build_cast(&self->builder, ptr, lhs);
        }
        else
                field_addr = ssa_build_getfieldaddr(&self->builder, lhs, member);

        if (!field_addr)
                return NULL;

        return tree_expr_is_lvalue(expr)
                ? field_addr
                : ssa_emit_load(self, field_addr);
}

extern ssa_value* ssa_emit_cast_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* operand = ssa_emit_expr(self, tree_get_cast_operand(expr));
        if (!operand)
                return NULL;

        tree_type* t = tree_get_expr_type(expr);
        ssa_emit_type(self->module_emitter, t);
        return ssa_build_cast(&self->builder, t, operand);
}

extern ssa_value* ssa_emit_sizeof_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        tree_type* type = tree_sizeof_contains_type(expr)
                ? tree_get_sizeof_type(expr)
                : tree_get_expr_type(tree_get_sizeof_expr(expr));

        size_t size = tree_get_sizeof(self->context->target, type);
        return ssa_build_int_constant(&self->builder, tree_get_expr_type(expr), size);
}

extern ssa_value* ssa_emit_offsetof_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        size_t offset = tree_get_offsetof(self->context->target, tree_get_offsetof_field(expr));
        return ssa_build_int_constant(&self->builder, tree_get_expr_type(expr), offset);
}

extern ssa_value* ssa_emit_expr(ssa_function_emitter* self, const tree_expr* expr)
{
        assert(expr);
        switch (tree_get_expr_kind(expr))
        {
                case TEK_UNKNOWN:           return ssa_build_undef(&self->builder, tree_get_expr_type(expr));
                case TEK_BINARY:            return ssa_emit_binary_expr(self, expr);
                case TEK_UNARY:             return ssa_emit_unary_expr(self, expr);
                case TEK_CALL:              return ssa_emit_call_expr(self, expr);
                case TEK_SUBSCRIPT:         return ssa_emit_subscript_expr(self, expr);
                case TEK_CONDITIONAL:       return ssa_emit_conditional_expr(self, expr);
                case TEK_INTEGER_LITERAL:   return ssa_emit_integer_literal(self, expr);
                case TEK_CHARACTER_LITERAL: return ssa_emit_character_literal(self, expr);
                case TEK_FLOATING_LITERAL:  return ssa_emit_floating_literal(self, expr);
                case TEK_STRING_LITERAL:    return ssa_emit_string_literal(self, expr);
                case TEK_DECL:              return ssa_emit_decl_expr(self, expr);
                case TEK_MEMBER:            return ssa_emit_member_expr(self, expr);
                case TEK_CAST:              return ssa_emit_cast_expr(self, expr);
                case TEK_SIZEOF:            return ssa_emit_sizeof_expr(self, expr);
                case TEK_OFFSETOF:          return ssa_emit_offsetof_expr(self, expr);
                case TEK_PAREN:             return ssa_emit_expr(self, tree_get_paren_expr(expr));

                case TEK_INIT_LIST:
                case TEK_IMPL_INIT:
                case TEK_DESIGNATION:
                default:
                        assert(0 && "Invalid expr");
                        return NULL;
        }
}

extern ssa_value* ssa_emit_expr_as_condition(ssa_function_emitter* self, const tree_expr* expr)
{
        ssa_value* v = ssa_emit_expr(self, expr);
        if (!v)
                return NULL;

        if (ssa_get_value_kind(v) != SVK_LOCAL_VAR)
                return ssa_build_neq_zero(&self->builder, v);

        ssa_instr* i = ssa_get_var_instr(v);
        if (ssa_get_instr_kind(i) != SIK_BINARY)
                return ssa_build_neq_zero(&self->builder, v);

        switch (ssa_get_binop_kind(i))
        {
                case SBIK_LE:
                case SBIK_GR:
                case SBIK_LEQ:
                case SBIK_GEQ:
                case SBIK_EQ:
                case SBIK_NEQ:
                        return v;

                default:
                        return ssa_build_neq_zero(&self->builder, v);
        }
}

static void remove_value_opt(ssa_value* val)
{
        if (!ssa_value_is_used(val))
                ssa_remove_instr(ssa_get_var_instr(val));
}

static bool ssa_emit_record_initializer(ssa_function_emitter* self, ssa_value* record, const tree_expr* init)
{
        if (!tree_expr_is(init, TEK_INIT_LIST))
        {
                ssa_value* val = ssa_emit_expr(self, init);
                return val && ssa_emit_store(self, val, record);
        }

        assert(tree_get_init_list_exprs_size(init));
        tree_decl* rec = tree_get_decl_type_entity(
                tree_desugar_type(tree_get_pointer_target(ssa_get_value_type(record))));
        tree_decl_scope* fields = tree_get_record_fields(rec);

        if (tree_record_is_union(rec))
        {
                assert(tree_get_init_list_exprs_size(init) == 1);
                tree_expr* des = tree_get_init_list_expr(init, 0);
                tree_designator* fd = tree_get_designation_designators_begin(des)[0];
                tree_decl* field = tree_get_designator_field(fd);
                assert(field);
                tree_type* field_addr = tree_new_pointer_type(
                        ssa_get_tree(self->context), tree_get_decl_type(field));
                ssa_value* ssa_field = ssa_build_cast(&self->builder, field_addr, record);
                if (!ssa_emit_local_var_initializer(self, ssa_field, tree_get_designation_init(des)))
                        return false;
                remove_value_opt(ssa_field);
                return true;
        }

        size_t i = 0;
        TREE_FOREACH_DECL_IN_SCOPE(fields, it)
        {
                if (!tree_decl_is(it, TDK_FIELD))
                        continue;

                assert(i < tree_get_init_list_exprs_size(init));
                const tree_expr* e = tree_get_init_list_expr(init, i++);
                ssa_value* field = ssa_build_getfieldaddr(&self->builder, record, it);
                if (!field || !ssa_emit_local_var_initializer(self, field, e))
                        return false;
                remove_value_opt(field);
        }
        return true;
}

static bool ssa_emit_array_initializer(ssa_function_emitter* self, ssa_value* array, const tree_expr* init)
{
        tree_type* et = tree_get_array_eltype(
                tree_desugar_type(tree_get_pointer_target(ssa_get_value_type(array))));
        tree_type* ptr = tree_new_pointer_type(self->context->tree, et);
        array = ssa_build_cast(&self->builder, ptr, array);
        assert(array);

        size_t index = 0;
        TREE_FOREACH_INIT_LIST_EXPR(init, it, end)
        {
                ssa_value* ssa_index = ssa_build_int_constant(&self->builder,
                        tree_get_size_type(self->context->tree), index++);
                ssa_value* elt = ssa_build_ptradd(&self->builder, array, ssa_index);
                if (!ssa_index || !elt || !ssa_emit_local_var_initializer(self, elt, *it))
                        return false;
                remove_value_opt(elt);
        }
        remove_value_opt(array);
        return true;
}

extern bool ssa_emit_local_var_initializer(ssa_function_emitter* self, ssa_value* var, const tree_expr* init)
{
        tree_type* t = ssa_get_value_type(var);
        assert(tree_type_is_pointer(t));
        t = tree_get_pointer_target(t);

        if (tree_type_is_array(t))
                return ssa_emit_array_initializer(self, var, init);
        else if (tree_type_is_record(t))
                return ssa_emit_record_initializer(self, var, init);

        if (tree_expr_is(init, TEK_INIT_LIST))
        {
                assert(tree_get_init_list_exprs_size(init) == 1);
                init = tree_get_init_list_expr(init, 0);
        }
        if (!init || tree_expr_is(init, TEK_UNKNOWN))
                return true;

        ssa_value* val = ssa_emit_expr(self, init);
        return val && ssa_emit_store(self, val, var);
}
