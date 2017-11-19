#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/tree/tree-stmt.h"

extern bool ssaizer_build_jmp(ssaizer* self, ssa_block* dest)
{
        if (!ssa_build_jmp(&self->builder, ssa_get_block_value(dest)))
                return false;

        ssaizer_finish_current_block(self);
        return true;
}

extern bool ssaizer_maybe_build_jmp(ssaizer* self, ssa_block* dest)
{
        if (!self->block || ssa_get_block_exit(self->block))
                return true;

        return ssaizer_build_jmp(self, dest);
}

extern bool ssaizer_build_if(ssaizer* self,
        ssa_value* cond, ssa_block* on_true, ssa_block* on_false)
{
        if (!ssa_build_if(&self->builder, cond,
                ssa_get_block_value(on_true), ssa_get_block_value(on_false)))
        {
                return false;
        }
        ssaizer_finish_current_block(self);
        return true;
}

extern bool ssaize_labeled_stmt(ssaizer* self, const tree_stmt* stmt)
{
        tree_decl* label_decl = tree_get_label_stmt_decl(stmt);
        ssa_block* label = ssaizer_get_label_block(self, label_decl);
        if (!ssaizer_build_jmp(self, label))
                return false;

        ssaizer_enter_block(self, label);
        return ssaize_stmt(self, tree_get_label_decl_stmt(label_decl));
}

extern bool ssaize_default_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

extern bool ssaize_case_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
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
        return true;
}

extern bool ssaize_expr_stmt(ssaizer* self, const tree_stmt* stmt)
{
        tree_expr* expr = tree_get_expr_stmt_root(stmt);
        if (!expr)
                return true;

        return (bool)ssaize_expr(self, expr);
}

static bool _ssaize_if_stmt(ssaizer*, const tree_stmt*, ssa_block*);

static bool ssaize_else_stmt(ssaizer* self, const tree_stmt* stmt, ssa_block* exit)
{
        if (tree_stmt_is(stmt, TSK_IF))
                return _ssaize_if_stmt(self, stmt, exit);

        if (!ssaize_stmt(self, stmt))
                return false;

        return ssaizer_build_jmp(self, exit);
}

static bool _ssaize_if_stmt(ssaizer* self, const tree_stmt* stmt, ssa_block* exit)
{
        ssa_value* cond = ssaize_expr_as_condition(self, tree_get_if_condition(stmt));
        if (!cond)
                return false;

        bool has_else = tree_get_if_else(stmt) != NULL;
        ssa_block* body = ssaizer_new_block(self);
        ssa_block* next = has_else ? ssaizer_new_block(self) : exit;

        if (!ssaizer_build_if(self, cond, body, next))
                return false;

        ssaizer_enter_block(self, body);
        if (!ssaize_stmt(self, tree_get_if_body(stmt)))
                return false;
        if (!ssaizer_maybe_build_jmp(self, exit))
                return false;

        if (has_else)
        {
                ssaizer_enter_block(self, next);
                if (!ssaize_else_stmt(self, tree_get_if_else(stmt), exit))
                        return false;
        }

        ssaizer_enter_block(self, exit);
        return true;
}

extern bool ssaize_if_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* exit = ssaizer_new_block(self);
        return _ssaize_if_stmt(self, stmt, exit);
}

extern bool ssaize_switch_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return false;
}

static bool ssaize_loop_cond(
        ssaizer* self,
        const tree_expr* cond,
        ssa_block* loop_cond,
        ssa_block* loop_body,
        ssa_block* loop_exit)
{
        ssaizer_enter_block(self, loop_cond);
        if (!cond)
                return ssaizer_build_jmp(self, loop_body);

        ssa_value* cond_val = ssaize_expr_as_condition(self, cond);
        return cond_val && ssaizer_build_if(self, cond_val, loop_body, loop_exit);
}

static bool ssaize_loop_body(
        ssaizer* self,
        const tree_stmt* body,
        ssa_block* loop_body,
        ssa_block* loop_exit)
{
        ssaizer_enter_block(self, loop_body);
        if (!ssaize_stmt(self, body))
                return false;

        return ssaizer_maybe_build_jmp(self, loop_exit);
}

static bool ssaize_loop_step(ssaizer* self,
        const tree_expr* step, ssa_block* loop_step, ssa_block* loop_entry)
{
        ssaizer_enter_block(self, loop_step);
        if (step && !ssaize_expr(self, step))
                return false;

        return ssaizer_build_jmp(self, loop_entry);
}

static bool ssaizer_start_loop(ssaizer* self,
        ssa_block* loop_entry, ssa_block* loop_exit, ssa_block* continue_dest)
{
        if (!ssaizer_build_jmp(self, loop_entry))
                return false;

        ssaizer_push_continue_dest(self, continue_dest);
        ssaizer_push_break_dest(self, loop_exit);
        return true;
}

static void ssaizer_cleanup_loop(ssaizer* self)
{
        ssaizer_pop_continue_dest(self);
        ssaizer_pop_break_dest(self);
}

extern bool ssaize_while_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* cond = ssaizer_new_block(self);
        ssa_block* body = ssaizer_new_block(self);
        ssa_block* exit = ssaizer_new_block(self);

        if (!ssaizer_start_loop(self, cond, exit, cond))
                return false;

        bool succeeded = true;
        if (!ssaize_loop_cond(self, tree_get_while_condition(stmt), cond, body, exit)
         || !ssaize_loop_body(self, tree_get_while_body(stmt), body, cond))
        {
                succeeded = false;
        }

        ssaizer_cleanup_loop(self);
        if (succeeded)
                ssaizer_enter_block(self, exit);
        return succeeded;
}

extern bool ssaize_do_while_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* body = ssaizer_new_block(self);
        ssa_block* cond = ssaizer_new_block(self);
        ssa_block* exit = ssaizer_new_block(self);

        if (!ssaizer_start_loop(self, cond, exit, cond))
                return false;

        bool succeeded = true;
        if (!ssaize_loop_body(self, tree_get_do_while_body(stmt), body, cond)
         || !ssaize_loop_cond(self, tree_get_do_while_condition(stmt), cond, body, exit))
        {
                succeeded = false;
        }

        ssaizer_cleanup_loop(self);
        if (succeeded)
                ssaizer_enter_block(self, exit);
        return succeeded;
}

extern bool ssaize_for_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* cond = ssaizer_new_block(self);
        ssa_block* body = ssaizer_new_block(self);
        ssa_block* step = ssaizer_new_block(self);
        ssa_block* exit = ssaizer_new_block(self);

        tree_stmt* init = tree_get_for_init(stmt);
        if (init && !ssaize_stmt(self, init))
                return false;

        if (!ssaizer_start_loop(self, cond, exit, step))
                return false;

        bool succeeded = true;
        ssaizer_push_scope(self);
        if (!ssaize_loop_cond(self, tree_get_for_condition(stmt), cond, body, exit)
         || !ssaize_loop_body(self, tree_get_for_body(stmt), body, step)
         || !ssaize_loop_step(self, tree_get_for_step(stmt), step, cond))
        {
                succeeded = false;
        }
      
        ssaizer_pop_scope(self);
        ssaizer_cleanup_loop(self);
        if (succeeded)
                ssaizer_enter_block(self, exit);
        return succeeded;
}

extern bool ssaize_goto_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* label = ssaizer_get_label_block(self, tree_get_goto_label(stmt));
        return ssaizer_build_jmp(self, label);
}

extern bool ssaize_continue_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* dest = ssaizer_get_continue_dest(self);
        return ssaizer_build_jmp(self, dest);
}

extern bool ssaize_break_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* dest = ssaizer_get_break_dest(self);
        return ssaizer_build_jmp(self, dest);
}

extern bool ssaize_decl_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return ssaize_decl(self, tree_get_decl_stmt_entity(stmt));
}

extern bool ssaize_return_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_value* val = NULL;
        tree_expr* expr = tree_get_return_value(stmt);
        if (expr && !(val = ssaize_expr(self, expr)))
                return false;

        return (bool)ssa_build_return(&self->builder, val);
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
        if (!self->block)
        {
                self->block = ssaizer_new_block(self);
                ssaizer_enter_block(self, self->block);
        }

        tree_stmt_kind k = tree_get_stmt_kind(stmt);
        TREE_CHECK_STMT_KIND(k);
        return ssaize_stmt_table[k](self, stmt);
}
