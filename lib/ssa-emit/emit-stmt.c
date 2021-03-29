#include "emitter.h"
#include "scc/ssa/block.h"

extern bool ssa_emit_cond_jmp(
        ssa_function_emitter* self, ssa_value* cond, ssa_block* ontrue, ssa_block* onfalse)
{
        if (!ssa_build_cond_jmp(&self->builder, cond,
                ssa_get_block_label(ontrue), ssa_get_block_label(onfalse)))
        {
                return false;
        }
        ssa_emit_current_block(self);
        return true;
}

extern bool ssa_emit_jmp(ssa_function_emitter* self, ssa_block* dest)
{
        if (!ssa_build_inderect_jmp(&self->builder, ssa_get_block_label(dest)))
                return false;

        ssa_emit_current_block(self);
        return true;
}

extern bool ssa_emit_jmp_opt(ssa_function_emitter* self, ssa_block* dest)
{
        if (!self->block || ssa_current_block_is_terminated(self))
        {
                return true;
        }

        return ssa_emit_jmp(self, dest);
}

static bool ssa_emit_labeled_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        tree_decl* label_decl = tree_get_label_stmt_decl(stmt);
        ssa_block* label = ssa_get_block_for_label(self, label_decl);
        if (!ssa_emit_jmp(self, label))
                return false;

        ssa_enter_block(self, label);
        return ssa_emit_stmt(self, tree_get_label_decl_stmt(label_decl));
}

extern bool ssa_emit_default_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_instr* switch_instr = ssa_get_switch_instr(self);
        ssa_block* body = ssa_get_label_block(ssa_get_instr_operand_value(switch_instr, 1));

        if (!ssa_emit_jmp(self, body))
                return false;

        ssa_enter_block(self, body);
        return ssa_emit_stmt(self, tree_get_default_body(stmt));
}

extern bool ssa_emit_case_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_instr* switch_instr = ssa_get_switch_instr(self);
        ssa_block* body = ssa_new_function_block(self);
        ssa_value* case_val = ssa_build_int_constant(&self->builder,
                tree_get_expr_type(tree_get_case_expr(stmt)),
                num_as_u64(tree_get_case_cvalue(stmt)));

        if (!ssa_emit_jmp_opt(self, body))
                return false;

        ssa_add_switch_case(switch_instr, self->context, case_val, ssa_get_block_label(body));
        ssa_enter_block(self, body);

        return ssa_emit_stmt(self, tree_get_case_body(stmt));
}

extern bool ssa_emit_compound_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_push_scope(self);
        const tree_scope* s = tree_get_compound_cscope(stmt);
        TREE_FOREACH_STMT(s, it)
                if (!ssa_emit_stmt(self, it))
                {
                        ssa_pop_scope(self);
                        return false;
                }
        ssa_pop_scope(self);
        return true;
}

extern bool ssa_emit_expr_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        tree_expr* expr = tree_get_expr_stmt_root(stmt);
        if (!expr)
                return true;

        return (bool)ssa_emit_expr(self, expr);
}

static bool _ssa_emit_if_stmt(ssa_function_emitter*, const tree_stmt*, ssa_block*);

static bool ssa_emit_else_stmt(ssa_function_emitter* self, const tree_stmt* stmt, ssa_block* exit)
{
        if (tree_stmt_is(stmt, TSK_IF))
                return _ssa_emit_if_stmt(self, stmt, exit);

        if (!ssa_emit_stmt(self, stmt))
                return false;

        return ssa_emit_jmp_opt(self, exit);
}

static bool _ssa_emit_if_stmt(ssa_function_emitter* self, const tree_stmt* stmt, ssa_block* exit)
{
        ssa_value* cond = ssa_emit_expr_as_condition(self, tree_get_if_condition(stmt));
        if (!cond)
                return false;

        bool has_else = tree_get_if_else(stmt) != NULL;
        ssa_block* body = ssa_new_function_block(self);
        ssa_block* next = has_else ? ssa_new_function_block(self) : exit;

        if (!ssa_emit_cond_jmp(self, cond, body, next))
                return false;

        ssa_enter_block(self, body);
        if (!ssa_emit_stmt(self, tree_get_if_body(stmt)))
                return false;
        if (!ssa_emit_jmp_opt(self, exit))
                return false;

        if (has_else)
        {
                ssa_enter_block(self, next);
                if (!ssa_emit_else_stmt(self, tree_get_if_else(stmt), exit))
                        return false;
        }

        ssa_enter_block(self, exit);
        return true;
}

extern bool ssa_emit_if_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* exit = ssa_new_function_block(self);
        return _ssa_emit_if_stmt(self, stmt, exit);
}

extern bool ssa_emit_switch_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_value* cond = ssa_emit_expr(self, tree_get_switch_expr(stmt));
        if (!cond)
                return false;

        ssa_block* otherwise = ssa_new_function_block(self);
        ssa_instr* switch_instr = ssa_build_switch_instr(
                &self->builder, cond, ssa_get_block_label(otherwise));
        if (!switch_instr)
                return false;

        ssa_block* body = ssa_new_function_block(self);
        ssa_block* exit = ssa_new_function_block(self);
        ssa_emit_current_block(self);
        ssa_enter_block(self, body);

        ssa_push_break_dest(self, exit);
        ssa_push_switch_instr(self, switch_instr);
        bool result = ssa_emit_stmt(self, tree_get_switch_body(stmt));
        ssa_pop_break_dest(self);
        ssa_pop_switch_instr(self);

        if (!result || !ssa_emit_jmp_opt(self, exit))
                return false;

        ssa_enter_block(self, otherwise);
        if (!ssa_emit_jmp_opt(self, exit))
                return false;

        ssa_enter_block(self, exit);
        return true;
}

static bool ssa_emit_loop_cond(
        ssa_function_emitter* self,
        const tree_expr* cond,
        ssa_block* loop_cond,
        ssa_block* loop_body,
        ssa_block* loop_exit)
{
        ssa_enter_block(self, loop_cond);
        if (!cond)
                return ssa_emit_jmp(self, loop_body);

        ssa_value* cond_val = ssa_emit_expr_as_condition(self, cond);
        return cond_val && ssa_emit_cond_jmp(self, cond_val, loop_body, loop_exit);
}

static bool ssa_emit_loop_body(
        ssa_function_emitter* self,
        const tree_stmt* body,
        ssa_block* loop_body,
        ssa_block* loop_exit)
{
        ssa_enter_block(self, loop_body);
        if (!ssa_emit_stmt(self, body))
                return false;

        return ssa_emit_jmp_opt(self, loop_exit);
}

static bool ssa_emit_loop_step(ssa_function_emitter* self,
        const tree_expr* step, ssa_block* loop_step, ssa_block* loop_entry)
{
        ssa_enter_block(self, loop_step);
        if (step && !ssa_emit_expr(self, step))
                return false;

        return ssa_emit_jmp(self, loop_entry);
}

static bool ssa_start_loop(ssa_function_emitter* self,
        ssa_block* loop_entry, ssa_block* loop_exit, ssa_block* continue_dest)
{
        if (!ssa_emit_jmp(self, loop_entry))
                return false;

        ssa_push_continue_dest(self, continue_dest);
        ssa_push_break_dest(self, loop_exit);
        return true;
}

static void ssa_cleanup_loop(ssa_function_emitter* self)
{
        ssa_pop_continue_dest(self);
        ssa_pop_break_dest(self);
}

extern bool ssa_emit_while_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* cond = ssa_new_function_block(self);
        ssa_block* body = ssa_new_function_block(self);
        ssa_block* exit = ssa_new_function_block(self);

        if (!ssa_start_loop(self, cond, exit, cond))
                return false;

        bool succeeded = ssa_emit_loop_cond(self, tree_get_while_condition(stmt), cond, body, exit)
                && ssa_emit_loop_body(self, tree_get_while_body(stmt), body, cond);

        ssa_cleanup_loop(self);
        if (succeeded)
                ssa_enter_block(self, exit);

        return succeeded;
}

extern bool ssa_emit_do_while_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* body = ssa_new_function_block(self);
        ssa_block* cond = ssa_new_function_block(self);
        ssa_block* exit = ssa_new_function_block(self);

        if (!ssa_start_loop(self, body, exit, cond))
                return false;

        bool succeeded = ssa_emit_loop_body(self, tree_get_do_while_body(stmt), body, cond)
                && ssa_emit_loop_cond(self, tree_get_do_while_condition(stmt), cond, body, exit);

        ssa_cleanup_loop(self);
        if (succeeded)
                ssa_enter_block(self, exit);

        return succeeded;
}

static bool _ssa_emit_for_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* cond = ssa_new_function_block(self);
        ssa_block* body = ssa_new_function_block(self);
        ssa_block* step = ssa_new_function_block(self);
        ssa_block* exit = ssa_new_function_block(self);

        tree_stmt* init = tree_get_for_init(stmt);
        if (init && !ssa_emit_stmt(self, init))
                return false;

        if (!ssa_start_loop(self, cond, exit, step))
                return false;

        bool succeeded = ssa_emit_loop_cond(self, tree_get_for_condition(stmt), cond, body, exit)
                && ssa_emit_loop_body(self, tree_get_for_body(stmt), body, step)
                && ssa_emit_loop_step(self, tree_get_for_step(stmt), step, cond);

        ssa_cleanup_loop(self);
        if (succeeded)
                ssa_enter_block(self, exit);

        return succeeded;
}

extern bool ssa_emit_for_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_push_scope(self);
        bool succeeded = _ssa_emit_for_stmt(self, stmt);
        ssa_pop_scope(self);
        return succeeded;
}

extern bool ssa_emit_goto_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* label = ssa_get_block_for_label(self, tree_get_goto_label(stmt));
        return ssa_emit_jmp(self, label);
}

extern bool ssa_emit_continue_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* dest = ssa_get_continue_dest(self);
        return ssa_emit_jmp(self, dest);
}

extern bool ssa_emit_break_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* dest = ssa_get_break_dest(self);
        return ssa_emit_jmp(self, dest);
}

extern bool ssa_emit_decl_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        return ssa_emit_local_decl(self, tree_get_decl_stmt_entity(stmt));
}

extern bool ssa_emit_return_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_value* val = NULL;
        tree_expr* expr = tree_get_return_value(stmt);
        if (expr && !(val = ssa_emit_expr(self, expr)))
                return false;
        if (!ssa_build_return(&self->builder, val))
                return false;

        ssa_emit_current_block(self);
        return true;
}

extern bool ssa_emit_stmt_as_atomic(ssa_function_emitter* self, const tree_stmt* stmt)
{
        ssa_block* exit = ssa_new_function_block(self);

        self->atomic_stmt_nesting++;
        ssa_block* body = ssa_new_function_block(self);

        if (!ssa_emit_jmp_opt(self, body))
        {
                self->atomic_stmt_nesting--;
                return false;
        }

        ssa_enter_block(self, body);

        bool result = false;
        ssa_push_break_dest(self, exit);

        ssa_instr* prev_pos;
        if (self->atomic_stmt_nesting == 1)
        {
                prev_pos = self->alloca_insertion_pos;
                self->alloca_insertion_pos = ssa_get_block_instrs_begin(self->block);
        }
        if (!ssa_emit_stmt(self, stmt))
                goto cleanup;

        if (self->block && !ssa_emit_jmp_opt(self, exit))
                goto cleanup;

        assert(!self->block);
        ssa_enter_block(self, exit);
        result = true;
cleanup:
        if (self->atomic_stmt_nesting == 1)
                self->alloca_insertion_pos = prev_pos;
        self->atomic_stmt_nesting--;
        ssa_pop_break_dest(self);
        return result;
}

extern bool ssa_emit_atomic_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        return ssa_emit_stmt_as_atomic(self, tree_get_atomic_body(stmt));
}

extern bool ssa_emit_stmt(ssa_function_emitter* self, const tree_stmt* stmt)
{
        if (!self->block)
                ssa_enter_block(self, ssa_new_function_block(self));

        switch (tree_get_stmt_kind(stmt))
        {
                case TSK_LABELED:  return ssa_emit_labeled_stmt(self, stmt);
                case TSK_CASE:     return ssa_emit_case_stmt(self, stmt);
                case TSK_DEFAULT:  return ssa_emit_default_stmt(self, stmt);
                case TSK_COMPOUND: return ssa_emit_compound_stmt(self, stmt);
                case TSK_EXPR:     return ssa_emit_expr_stmt(self, stmt);
                case TSK_IF:       return ssa_emit_if_stmt(self, stmt);
                case TSK_SWITCH:   return ssa_emit_switch_stmt(self, stmt);
                case TSK_WHILE:    return ssa_emit_while_stmt(self, stmt);
                case TSK_DO_WHILE: return ssa_emit_do_while_stmt(self, stmt);
                case TSK_FOR:      return ssa_emit_for_stmt(self, stmt);
                case TSK_GOTO:     return ssa_emit_goto_stmt(self, stmt);
                case TSK_CONTINUE: return ssa_emit_continue_stmt(self, stmt);
                case TSK_BREAK:    return ssa_emit_break_stmt(self, stmt);
                case TSK_DECL:     return ssa_emit_decl_stmt(self, stmt);
                case TSK_RETURN:   return ssa_emit_return_stmt(self, stmt);
                case TSK_ATOMIC:   return ssa_emit_atomic_stmt(self, stmt);

                case TSK_UNKNOWN:
                default:
                        assert(0 && "Invalid stmt");
                        return false;
        }
}
