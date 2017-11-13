#include "scc/ssa/ssa-ir.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-value.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-branch.h"
#include "scc/tree/tree-decl.h"

DSEQ_GEN(htab, htab);

extern void ssa_init_ir_gen(ssa_ir_gen* self, ssa_context* context)
{
        self->context = context;
        self->label_uid = 0;
        ssa_ir_gen_enter_block(self, NULL);
        dseq_init_ex_htab(&self->defs, ssa_get_context_alloc(context));
}

extern void ssa_dispose_ir_gen(ssa_ir_gen* self)
{
        // todo
}

extern void ssa_ir_gen_enter_block(ssa_ir_gen* self, ssa_block* block)
{
        self->block = block;
        ssa_init_builder(&self->builder, self->context, block);
}

extern void ssa_ir_gen_push_scope(ssa_ir_gen* self)
{
        dseq_resize(&self->defs, dseq_size(&self->defs) + 1);
        htab* last = dseq_end_htab(&self->defs) - 1;
        htab_init_ex_ptr(last, ssa_get_context_alloc(self->context));
}

extern void ssa_ir_gen_pop_scope(ssa_ir_gen* self)
{
        ssize size = dseq_size(&self->defs);
        S_ASSERT(size);
        htab* last = dseq_end_htab(&self->defs) - 1;
        htab_dispose(last);
        dseq_resize(&self->defs, size - 1);
}

extern void ssa_ir_gen_set_def(ssa_ir_gen* self, const tree_decl* var, ssa_value* def)
{
        S_ASSERT(dseq_size(&self->defs));
        htab* last = dseq_end_htab(&self->defs) - 1;
        htab_insert_ptr(last, tree_get_decl_name(var), def);
}

extern ssa_value* ssa_ir_gen_get_def(ssa_ir_gen* self, const tree_decl* var)
{
        S_ASSERT(dseq_size(&self->defs));
        htab* last = dseq_end_htab(&self->defs) - 1;
        hiter res;
        return htab_find(last, tree_get_decl_name(var), &res)
                ? hiter_get_ptr(&res)
                : NULL;
}

static ssa_value* ssa_gen_mul_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_div_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_mod_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_add_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_sub_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_shl_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_shr_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_le_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_gr_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_leq_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_geq_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_eq_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_neq_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_and_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_or_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_add_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_mul_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_div_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_mod_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_shl_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_shr_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_and_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_xor_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_or_assign_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value* ssa_gen_comma_ir(ssa_ir_gen* self, ssa_value* lhs, ssa_value* rhs)
{
        return NULL;
}

static ssa_value*(*ssa_gen_binary_expr_table[TBK_SIZE])(ssa_ir_gen*, ssa_value*, ssa_value*) =
{
        NULL,
        &ssa_gen_mul_ir,
        &ssa_gen_div_ir,
        &ssa_gen_mod_ir,
        &ssa_gen_add_ir,
        &ssa_gen_sub_ir,
        &ssa_gen_shl_ir,
        &ssa_gen_shr_ir,
        &ssa_gen_le_ir,
        &ssa_gen_gr_ir,
        &ssa_gen_leq_ir,
        &ssa_gen_geq_ir,
        &ssa_gen_eq_ir,
        &ssa_gen_neq_ir,
        &ssa_gen_and_ir,
        &ssa_gen_or_ir,
        &ssa_gen_assign_ir,
        &ssa_gen_add_assign_ir,
        &ssa_gen_mul_assign_ir,
        &ssa_gen_div_assign_ir,
        &ssa_gen_mod_assign_ir,
        &ssa_gen_shl_assign_ir,
        &ssa_gen_shr_assign_ir,
        &ssa_gen_and_assign_ir,
        &ssa_gen_xor_assign_ir,
        &ssa_gen_or_assign_ir,
        &ssa_gen_comma_ir,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_gen_binary_expr_table) == TBK_SIZE,
        "ssa_gen_binary_expr_table needs an update");

extern ssa_value* ssa_gen_binary_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        tree_binop_kind k = tree_get_binop_kind(expr);
        TREE_CHECK_BINOP_KIND(k);

        ssa_value* lhs = ssa_gen_expr_ir(self, tree_get_binop_lhs(expr));
        if (!lhs)
                return NULL;

        ssa_value* rhs = ssa_gen_expr_ir(self, tree_get_binop_rhs(expr));
        if (!rhs)
                return NULL;

        return ssa_gen_binary_expr_table[k](self, lhs, rhs);
}

static ssa_value* ssa_gen_post_inc_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_post_dec_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_pre_inc_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_pre_dec_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_plus_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_minus_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_not_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_log_not_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_dereference_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value* ssa_gen_address_ir(ssa_ir_gen* self, ssa_value* operand)
{
        return NULL;
}

static ssa_value*(*ssa_gen_unary_expr_ir_table[TUK_SIZE])(ssa_ir_gen*, ssa_value*) =
{
        NULL,
        &ssa_gen_post_inc_ir,
        &ssa_gen_post_dec_ir,
        &ssa_gen_pre_inc_ir,
        &ssa_gen_pre_dec_ir,
        &ssa_gen_plus_ir,
        &ssa_gen_minus_ir,
        &ssa_gen_not_ir,
        &ssa_gen_log_not_ir,
        &ssa_gen_dereference_ir,
        &ssa_gen_address_ir,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_gen_unary_expr_ir_table) == TUK_SIZE,
        "ssa_gen_unary_expr_ir_table needs an update");

extern ssa_value* ssa_gen_unary_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        TREE_CHECK_UNOP_KIND(k);

        ssa_value* operand = ssa_gen_expr_ir(self, tree_get_unop_expr(expr));
        if (!operand)
                return NULL;

        return ssa_gen_unary_expr_ir_table[k](self, operand);
}

extern ssa_value* ssa_gen_call_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_subscript_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_conditional_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_integer_literal_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_character_literal_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_floating_literal_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_string_literal_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_decl_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_member_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_cast_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_sizeof_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_paren_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_init_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

extern ssa_value* ssa_gen_impl_init_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        return NULL;
}

static ssa_value* (*ssa_gen_expr_ir_table[TEK_SIZE])(ssa_ir_gen*, const tree_expr*) =
{
        NULL, // TEK_UNKNOWN
        &ssa_gen_binary_expr_ir, // TEK_BINARY
        &ssa_gen_unary_expr_ir, // TEK_UNARY
        &ssa_gen_call_expr_ir, // TEK_CALL
        &ssa_gen_subscript_expr_ir, // TEK_SUBSCRIPT
        &ssa_gen_conditional_expr_ir, // TEK_CONDITIONAL
        &ssa_gen_integer_literal_ir, // TEK_INTEGER_LITERAL
        &ssa_gen_character_literal_ir, // TEK_CHARACTER_LITERAL
        &ssa_gen_floating_literal_ir, // TEK_FLOATING_LITERAL
        &ssa_gen_string_literal_ir, // TEK_STRING_LITERAL
        &ssa_gen_decl_expr_ir, // TEK_DECL
        &ssa_gen_member_expr_ir, // TEK_MEMBER
        &ssa_gen_cast_expr_ir, // TEK_EXPLICIT_CAST
        &ssa_gen_cast_expr_ir, // TEK_IMPLICIT_CAST
        &ssa_gen_sizeof_expr_ir, // TEK_SIZEOF
        &ssa_gen_paren_expr_ir, // TEK_PAREN
        &ssa_gen_init_expr_ir, // TEK_INIT
        &ssa_gen_impl_init_expr_ir, // TEK_IMPL_INIT
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_gen_expr_ir_table) == TEK_SIZE,
        "ssa_gen_expr_ir_table needs an update");

extern ssa_value* ssa_gen_expr_ir(ssa_ir_gen* self, const tree_expr* expr)
{
        S_ASSERT(expr);
        tree_expr_kind k = tree_get_expr_kind(expr);
        TREE_CHECK_EXPR_KIND(k);
        return ssa_gen_expr_ir_table[k](self, expr);
}

extern bool ssa_gen_var_decl_ir(ssa_ir_gen* self, const tree_decl* decl)
{
        tree_expr* init = tree_get_var_init(decl);
        ssa_value* val;
        if (init)
        {
                ssa_value* init_val = ssa_gen_expr_ir(self, init);
                if (!init_val)
                        return false;

                val = ssa_build_init(&self->builder, init_val);
        }
        else
                val = ssa_build_empty_init(&self->builder, tree_get_decl_type(decl));

        
        return val != NULL;
}

extern bool ssa_gen_decl_group_ir(ssa_ir_gen* self, const tree_decl* decl)
{
        return false;
}

extern bool ssa_gen_decl_ir(ssa_ir_gen* self, const tree_decl* decl)
{
        if (tree_decl_is(decl, TDK_VAR))
                return ssa_gen_var_decl_ir(self, decl);
        else if (tree_decl_is(decl, TDK_GROUP))
                return ssa_gen_decl_group_ir(self, decl);

        // just ignore unknown decl
        return true;
}

extern bool ssa_gen_labeled_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_default_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_case_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_compound_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        ssa_ir_gen_push_scope(self);
        const tree_scope* s = tree_get_compound_cscope(stmt);
        TREE_FOREACH_STMT(s, it)
                if (!ssa_gen_stmt_ir(self, it))
                {
                        ssa_ir_gen_pop_scope(self);
                        return false;
                }
        ssa_ir_gen_pop_scope(self);
        return true;
}

extern bool ssa_gen_expr_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return ssa_gen_expr_ir(self, tree_get_expr_stmt_root(stmt));
}

extern bool ssa_gen_if_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_switch_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_while_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_do_while_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_for_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_goto_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_continue_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_break_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssa_gen_decl_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return ssa_gen_decl_ir(self, tree_get_decl_stmt_entity(stmt));
}

extern bool ssa_gen_return_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        return false;
}

static bool(*ssa_gen_stmt_ir_table[TSK_SIZE])(ssa_ir_gen*, const tree_stmt*) = 
{
        NULL,
        &ssa_gen_labeled_stmt_ir,
        &ssa_gen_case_stmt_ir,
        &ssa_gen_default_stmt_ir,
        &ssa_gen_compound_stmt_ir,
        &ssa_gen_expr_stmt_ir,
        &ssa_gen_if_stmt_ir,
        &ssa_gen_switch_stmt_ir,
        &ssa_gen_while_stmt_ir,
        &ssa_gen_do_while_stmt_ir,
        &ssa_gen_for_stmt_ir,
        &ssa_gen_goto_stmt_ir,
        &ssa_gen_continue_stmt_ir,
        &ssa_gen_break_stmt_ir,
        &ssa_gen_decl_stmt_ir,
        &ssa_gen_return_stmt_ir,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssa_gen_stmt_ir_table) == TSK_SIZE,
        "ssa_gen_stmt_ir_table needs an update");

extern bool ssa_gen_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt)
{
        tree_stmt_kind k = tree_get_stmt_kind(stmt);
        TREE_CHECK_STMT_KIND(k);
        return ssa_gen_stmt_ir_table[k](self, stmt);
}

extern ssa_module* ssa_gen_module_ir(ssa_ir_gen* self, const tree_module* module)
{
        ssa_module* m = ssa_new_module(self->context);
        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_DECL_SCOPE_FOREACH(globals, func)
        {
                if (!tree_decl_is(func, TDK_FUNCTION))
                        continue;

                tree_stmt* body = tree_get_function_body(func);
                if (!body)
                        continue;

                self->block = ssa_new_block(self->context, 0, NULL);
                ssa_block* def = self->block;

                ssa_ir_gen_enter_block(self, def);
                if (!ssa_gen_compound_stmt_ir(self, body))
                        continue;

                ssa_module_add_func_def(m, func, def);
        }

        return m;
}