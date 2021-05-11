#include "emitter.h"
#include "scc/core/common.h"
#include "scc/core/num.h"
#include "scc/ssa/const.h"
#include "scc/ssa/module.h"
#include "scc/ssa/value.h"
#include "scc/tree/decl.h"
#include "scc/tree/eval.h"
#include "scc/tree/expr.h"
#include "scc/tree/type.h"
#include "scc/ssa/context.h"

static ssa_const* emit_ptradd(ssa_module_emitter* self,
        int is_add, tree_type* t, const tree_expr* lhs, const tree_expr* rhs)
{
        ssa_const* ptr = ssa_emit_const_expr(self, lhs);
        ssa_const* off = ssa_emit_const_expr(self, rhs);
        if (!ptr || !off)
                return NULL;
        if (tree_type_is_pointer(ssa_get_const_type(off)))
        {
                ssa_const* tmp = ptr;
                ptr = off;
                off = tmp;
        }

        assert(tree_type_is_pointer(ssa_get_const_type(ptr)));
        assert(ssa_const_is(off, SCK_LITERAL));
        if (!is_add)
        {
                struct num* v = ssa_get_const_literal(off);
                num_to_int(v, v->num_bits);
                num_neg(v);
        }
        return ssa_new_const_ptradd(self->context, t, ptr, off);
}

static ssa_const* emit_addr_binop(ssa_module_emitter* self, const tree_expr* expr)
{
        assert(tree_binop_is(expr, TBK_ADD) || tree_binop_is(expr, TBK_SUB));
        return emit_ptradd(self,
                tree_binop_is(expr, TBK_ADD),
                tree_get_expr_type(expr),
                tree_get_binop_lhs(expr),
                tree_get_binop_rhs(expr));
}

static ssa_const* emit_addr_unop(ssa_module_emitter* self, const tree_expr* expr)
{
        ssa_const* operand = ssa_emit_const_expr(self, tree_get_unop_operand(expr));
        if (!operand)
                return NULL;
        assert(tree_unop_is(expr, TUK_ADDRESS));
        return operand;
}

static ssa_const* emit_addr_subscript(ssa_module_emitter* self, const tree_expr* expr)
{
        return emit_ptradd(self, 1,
                tree_get_expr_type(expr),
                tree_get_subscript_lhs(expr),
                tree_get_subscript_rhs(expr));
}

static ssa_const* emit_decl_addr(ssa_module_emitter* self, tree_decl* decl)
{
        ssa_value* global = ssa_get_global_decl(self, decl);
        if (!global && !ssa_emit_global_decl(self, decl))
                return NULL;
        tree_type* ptr = tree_new_pointer_type(self->context->tree, tree_get_decl_type(decl));
        assert(ptr);
        return ssa_new_const_addr(self->context, 
                ptr, ssa_get_global_decl(self, decl));
}

static ssa_const* emit_enumerator(ssa_module_emitter* self, tree_decl* e)
{
        return ssa_new_const_literal(self->context,
                tree_get_decl_type(e), *tree_get_enumerator_cvalue(e));
}

static ssa_const* emit_member_addr(ssa_module_emitter* self, const tree_expr* expr)
{
        ssa_const* var = ssa_emit_const_expr(self, tree_get_member_expr_lhs(expr));
        if (!var)
                return NULL;

        tree_decl* member = tree_get_member_expr_decl(expr);
        tree_type* member_ptr = tree_new_pointer_type(
                ssa_get_tree(self->context), tree_get_decl_type(member));
        tree_decl* rec = tree_get_decl_type_entity(
                tree_desugar_type(tree_get_pointer_target(ssa_get_const_type(var))));

        return tree_record_is_union(rec)
                ? ssa_new_const_cast(self->context, member_ptr, var)
                : ssa_new_const_field_addr(self->context, member_ptr, var, tree_get_field_index(member));
}

static ssa_const* emit_cast(ssa_module_emitter* self, const tree_expr* expr)
{
        ssa_const* operand = ssa_emit_const_expr(self, tree_get_cast_operand(expr));
        return operand 
                ? ssa_new_const_cast(self->context, tree_get_expr_type(expr), operand)
                : NULL;
}

static ssa_const* emit_addr_constant(ssa_module_emitter* self, const tree_expr* expr)
{
        switch (tree_get_expr_kind(expr))
        {
                case TEK_BINARY:    return emit_addr_binop(self, expr);
                case TEK_UNARY:     return emit_addr_unop(self, expr);
                case TEK_CAST:      return emit_cast(self, expr);
                case TEK_PAREN:     return emit_addr_constant(self, tree_get_paren_expr(expr));
                default:
                        assert(0 && "Invalid address constant");
                        return 0;
        }
}

extern ssa_const* emit_int_literal(ssa_context* context, tree_type* type, uint64_t val)
{
        struct num v;
        uint bits = (uint)tree_get_sizeof(ssa_get_target(context), type) * 8;
        if (tree_type_is_signed_integer(type))
                init_int(&v, val, bits);
        else
                init_uint(&v, val, bits);
        return ssa_new_const_literal(context, type, v);
}

static ssa_const* emit_literal(ssa_module_emitter* self, const tree_expr* expr)
{
        tree_type* t = tree_desugar_type(tree_get_expr_type(expr));
        if (tree_expr_is(expr, TEK_STRING_LITERAL))
        {
                ssa_value* str = ssa_new_string(self->context, t, tree_get_string_literal(expr));
                if (!str)
                        return NULL;
                ssa_add_module_global(self->module, str);
                return ssa_new_const_addr(self->context, t, str);
        }

        if (tree_expr_is(expr, TEK_INTEGER_LITERAL))
                return emit_int_literal(self->context, t, tree_get_integer_literal(expr));
        else if (tree_expr_is(expr, TEK_FLOATING_LITERAL))
        {
                struct num v = *tree_get_floating_literal_cvalue(expr);
                return ssa_new_const_literal(self->context, t, v);
        }
        else if (tree_expr_is(expr, TEK_CHARACTER_LITERAL))
                return emit_int_literal(self->context, t, tree_get_character_literal(expr));

        UNREACHABLE();
        return 0;
}

extern ssa_const* ssa_emit_const_expr(ssa_module_emitter* self, const tree_expr* expr)
{
        assert(expr);
        tree_type* t = tree_get_expr_type(expr);
        if (tree_type_is_record(t) && tree_record_is_union(tree_get_decl_type_entity(t)))
        {
                assert(0 && "Todo: global union initializers");
                return 0;
        }

        if (tree_expr_is_literal(expr))
                return emit_literal(self, expr);
        else if (tree_expr_is(expr, TEK_DECL))
        {
                tree_decl* entity = tree_get_decl_expr_entity(expr);
                return tree_decl_is(entity, TDK_ENUMERATOR)
                        ? emit_enumerator(self, entity)
                        : emit_decl_addr(self, entity);
        }
        else if (tree_expr_is(expr, TEK_MEMBER))
                return emit_member_addr(self, expr);
        else if (tree_expr_is(expr, TEK_SUBSCRIPT))
                return emit_addr_subscript(self, expr);
        else if (tree_expr_is(expr, TEK_INIT_LIST))
        {
                ssa_const* list = ssa_new_const_list(self->context, tree_get_expr_type(expr));
                TREE_FOREACH_INIT_LIST_EXPR(expr, it, end)
                {
                        ssa_const* v = ssa_emit_const_expr(self, *it);
                        if (!v)
                                return NULL;
                        vec_push(ssa_get_const_list(list), v);
                }
                return list;
        }

        if (tree_type_is_pointer(t))
                return emit_addr_constant(self, expr);

        tree_eval_result er;
        bool result = tree_eval_expr(self->context->tree, expr, &er);
        assert(result && er.kind != TERK_ADDRESS_CONSTANT);
        return ssa_new_const_literal(self->context, t, er.value);
}
