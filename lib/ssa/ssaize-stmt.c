#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/tree/tree-stmt.h"

extern bool ssaize_labeled_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_default_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_case_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

static bool ssaizer_maybe_insert_return(ssaizer* self)
{
        if (ssa_get_block_exit(self->block))
                return true;

        tree_type* restype = tree_get_function_restype(
                tree_get_decl_type(ssa_get_function_entity(self->function)));

        ssa_value* val = NULL;
        if (!tree_type_is_void(restype))
        {
                ssa_value* alloca = ssa_build_alloca(&self->builder, restype);
                val = ssa_build_load(&self->builder, alloca);
        }
        
        ssa_build_return(&self->builder, val);
        ssaizer_finish_current_block(self);
        return true;
}

extern bool ssaize_compound_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssaizer_push_scope(self);
        const tree_scope* s = tree_get_compound_cscope(stmt);
        TREE_FOREACH_STMT(s, it)
                if (!ssaize_stmt(self, it))
                {
                        ssaizer_pop_scope(self);
                        return false;
                }
        ssaizer_pop_scope(self);
        return ssaizer_maybe_insert_return(self);
}

extern bool ssaize_expr_stmt(ssaizer* self, const tree_stmt* stmt)
{
        bool res = (bool)ssaize_expr(self, tree_get_expr_stmt_root(stmt));
        return res;
}

extern bool ssaize_if_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_value* cond = ssaize_expr(self, tree_get_if_condition(stmt));
        if (!cond)
                return false;

        return false;
}

extern bool ssaize_switch_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_while_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_do_while_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_for_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_goto_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_continue_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_break_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_decl_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return ssaize_decl(self, tree_get_decl_stmt_entity(stmt));
}

extern bool ssaize_return_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

static bool(*ssaize_stmt_table[TSK_SIZE])(ssaizer*, const tree_stmt*) =
{
        NULL,
        &ssaize_labeled_stmt,
        &ssaize_case_stmt,
        &ssaize_default_stmt,
        &ssaize_compound_stmt,
        &ssaize_expr_stmt,
        &ssaize_if_stmt,
        &ssaize_switch_stmt,
        &ssaize_while_stmt,
        &ssaize_do_while_stmt,
        &ssaize_for_stmt,
        &ssaize_goto_stmt,
        &ssaize_continue_stmt,
        &ssaize_break_stmt,
        &ssaize_decl_stmt,
        &ssaize_return_stmt,
};

S_STATIC_ASSERT(S_ARRAY_SIZE(ssaize_stmt_table) == TSK_SIZE,
        "ssaize_stmt_table needs an update");

extern bool ssaize_stmt(ssaizer* self, const tree_stmt* stmt)
{
        tree_stmt_kind k = tree_get_stmt_kind(stmt);
        TREE_CHECK_STMT_KIND(k);
        return ssaize_stmt_table[k](self, stmt);
}
