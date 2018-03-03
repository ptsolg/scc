#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/tree/tree-stmt.h"

extern bool ssaizer_build_jmp(ssaizer* self, ssa_block* dest)
{
        if (!ssa_build_inderect_jmp(&self->builder, ssa_get_block_label(dest)))
                return false;

        ssaizer_finish_current_block(self);
        return true;
}

extern bool ssaizer_maybe_build_jmp(ssaizer* self, ssa_block* dest)
{
        if (!self->block || ssaizer_current_block_is_terminated(self))
        {
                return true;
        }

        return ssaizer_build_jmp(self, dest);
}

extern bool ssaizer_build_if(ssaizer* self,
        ssa_value* cond, ssa_block* on_true, ssa_block* on_false)
{
        if (!ssa_build_conditional_jmp(&self->builder, cond,
                ssa_get_block_label(on_true), ssa_get_block_label(on_false)))
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
        ssa_instr* switch_instr = ssaizer_get_switch_instr(self);
        ssa_block* body = ssa_get_label_block(ssa_get_instr_operand_value(switch_instr, 1));

        if (!ssaizer_build_jmp(self, body))
                return false;

        ssaizer_enter_block(self, body);
        return ssaize_stmt(self, tree_get_default_body(stmt));
}

extern bool ssaize_case_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_instr* switch_instr = ssaizer_get_switch_instr(self);
        ssa_block* body = ssaizer_new_block(self);
        ssa_value* case_val = ssa_build_int_constant(&self->builder,
                tree_get_expr_type(tree_get_case_expr(stmt)),
                int_get_u64(tree_get_case_cvalue(stmt)));

        if (!ssaizer_maybe_build_jmp(self, body))
                return false;

        ssa_add_switch_case(switch_instr, self->context, case_val, ssa_get_block_label(body));
        ssaizer_enter_block(self, body);

        return ssaize_stmt(self, tree_get_case_body(stmt));
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

        return ssaizer_maybe_build_jmp(self, exit);
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
        ssa_value* cond = ssaize_expr(self, tree_get_switch_expr(stmt));
        if (!cond)
                return false;

        ssa_block* otherwise = ssaizer_new_block(self);
        ssa_instr* switch_instr = ssa_build_switch_instr(
                &self->builder, cond, ssa_get_block_label(otherwise));
        if (!switch_instr)
                return false;

        ssa_block* body = ssaizer_new_block(self);
        ssa_block* exit = ssaizer_new_block(self);
        ssaizer_finish_current_block(self);
        ssaizer_enter_block(self, body);

        ssaizer_push_break_dest(self, exit);
        ssaizer_push_switch_instr(self, switch_instr);
        bool correct = ssaize_stmt(self, tree_get_switch_body(stmt));
        ssaizer_pop_break_dest(self);
        ssaizer_pop_switch_instr(self);
        
        if (!correct || !ssaizer_maybe_build_jmp(self, exit))
                return false;

        ssaizer_enter_block(self, otherwise);
        if (!ssaizer_maybe_build_jmp(self, exit))
                return false;

        ssaizer_enter_block(self, exit);
        return true;
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

        bool succeeded = ssaize_loop_cond(self, tree_get_while_condition(stmt), cond, body, exit)
                && ssaize_loop_body(self, tree_get_while_body(stmt), body, cond);

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

        if (!ssaizer_start_loop(self, body, exit, cond))
                return false;

        bool succeeded = ssaize_loop_body(self, tree_get_do_while_body(stmt), body, cond)
                && ssaize_loop_cond(self, tree_get_do_while_condition(stmt), cond, body, exit);

        ssaizer_cleanup_loop(self);
        if (succeeded)
                ssaizer_enter_block(self, exit);

        return succeeded;
}

static bool _ssaize_for_stmt(ssaizer* self, const tree_stmt* stmt)
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

        bool succeeded = ssaize_loop_cond(self, tree_get_for_condition(stmt), cond, body, exit)
                && ssaize_loop_body(self, tree_get_for_body(stmt), body, step)
                && ssaize_loop_step(self, tree_get_for_step(stmt), step, cond);

        ssaizer_cleanup_loop(self);
        if (succeeded)
                ssaizer_enter_block(self, exit);

        return succeeded;
}

extern bool ssaize_for_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssaizer_push_scope(self);
        bool succeeded = _ssaize_for_stmt(self, stmt);
        ssaizer_pop_scope(self);
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

        if (!ssa_build_return(&self->builder, val))
                return false;
        
        ssaizer_finish_current_block(self);
        return true;
}

extern bool ssaize_stmt(ssaizer* self, const tree_stmt* stmt)
{
        if (!self->block)
                ssaizer_enter_block(self, ssaizer_new_block(self));

        switch (tree_get_stmt_kind(stmt))
        {
                case TSK_LABELED:  return ssaize_labeled_stmt(self, stmt);
                case TSK_CASE:     return ssaize_case_stmt(self, stmt);
                case TSK_DEFAULT:  return ssaize_default_stmt(self, stmt);
                case TSK_COMPOUND: return ssaize_compound_stmt(self, stmt);
                case TSK_EXPR:     return ssaize_expr_stmt(self, stmt);
                case TSK_IF:       return ssaize_if_stmt(self, stmt);
                case TSK_SWITCH:   return ssaize_switch_stmt(self, stmt);
                case TSK_WHILE:    return ssaize_while_stmt(self, stmt);
                case TSK_DO_WHILE: return ssaize_do_while_stmt(self, stmt);
                case TSK_FOR:      return ssaize_for_stmt(self, stmt);
                case TSK_GOTO:     return ssaize_goto_stmt(self, stmt);
                case TSK_CONTINUE: return ssaize_continue_stmt(self, stmt);
                case TSK_BREAK:    return ssaize_break_stmt(self, stmt);
                case TSK_DECL:     return ssaize_decl_stmt(self, stmt);
                case TSK_RETURN:   return ssaize_return_stmt(self, stmt);

                case TSK_UNKNOWN:
                default:
                        assert(0 && "Invalid stmt");
                        return false;
        }
}
