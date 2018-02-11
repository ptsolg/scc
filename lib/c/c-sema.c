#include "scc/c/c-sema.h"
#include "scc/c/c-info.h"
#include "scc/c/c-errors.h"
#include "scc/scl/bit-utils.h"

DSEQ_GEN(cssi, cswitch_stmt_info);

extern void csema_init(csema* self, ccontext* context, clogger* logger)
{
        self->ccontext = context;
        self->context = cget_tree(context);
        self->module = NULL;
        self->globals = NULL;
        self->target = NULL;
        self->locals = NULL;
        self->function = NULL;
        self->labels = NULL;
        self->scope = NULL;
        self->logger = logger;
        dseq_init_ex_cssi(&self->switch_stack, cget_alloc(self->ccontext));
}

extern void csema_dispose(csema* self)
{
        // todo
}

extern void csema_enter_module(csema* self, tree_module* module)
{
        self->module = module;
        self->globals = tree_get_module_globals(module);
        self->target = tree_get_module_target(module);
        self->locals = self->globals;
}

extern void csema_enter_scope(csema* self, tree_scope* scope)
{
        S_ASSERT(tree_get_scope_parent(scope) == self->scope
                && "scopes should be connected.");
        self->scope = scope;
        csema_enter_decl_scope(self, tree_get_scope_decls(scope));
}

extern void csema_exit_scope(csema* self)
{
        S_ASSERT(self->scope);
        self->scope = tree_get_scope_parent(self->scope);
        csema_exit_decl_scope(self);
}

extern void csema_enter_decl_scope(csema* self, tree_decl_scope* scope)
{
        S_ASSERT(tree_get_decl_scope_parent(scope) == self->locals
                && "scopes should be connected.");
        self->locals = scope;
}

extern void csema_exit_decl_scope(csema* self)
{
        tree_decl_scope* parent = tree_get_decl_scope_parent(self->locals);
        S_ASSERT(parent);
        self->locals = parent;
}

extern void csema_enter_function(csema* self, tree_decl* func)
{
        self->labels = tree_get_function_labels(func);
        self->function = func;
        csema_enter_decl_scope(self, tree_get_function_params(func));
}

extern void csema_exit_function(csema* self)
{
        self->labels = NULL;
        self->function = NULL;
        csema_exit_decl_scope(self);
}

extern void csema_push_scope(csema* self)
{
        csema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void csema_push_switch_stmt_info(csema* self, tree_stmt* switch_stmt)
{
        dseq_resize(&self->switch_stack, dseq_size(&self->switch_stack) + 1);
        cswitch_stmt_info* last = csema_get_switch_stmt_info(self);
        last->switch_stmt = switch_stmt;
        last->has_default_label = false;
        htab_init_ex_ptr(&last->used_values, cget_alloc(self->ccontext));
}

extern void csema_pop_switch_stmt_info(csema* self)
{
        ssize size = dseq_size(&self->switch_stack);
        S_ASSERT(size);
        htab_dispose(&csema_get_switch_stmt_info(self)->used_values);
        dseq_resize(&self->switch_stack, size - 1);
}

extern void csema_set_switch_stmt_has_default_label(csema* self)
{
        csema_get_switch_stmt_info(self)->has_default_label = true;
}

extern cswitch_stmt_info* csema_get_switch_stmt_info(const csema* self)
{
        ssize size = dseq_size(&self->switch_stack);
        S_ASSERT(size);
        return dseq_begin_cssi(&self->switch_stack) + size - 1;
}

extern bool csema_in_switch_stmt(const csema* self)
{
        return dseq_size(&self->switch_stack) != 0;
}

extern bool csema_switch_stmt_has_default_label(const csema* self)
{
        return csema_get_switch_stmt_info(self)->has_default_label;
}

extern bool csema_switch_stmt_register_case_label(const csema* self, tree_stmt* label)
{
        const int_value* val = tree_get_case_cvalue(label);
        hval h = (hval)int_get_u32(val);

        hiter res;
        htab* used = &csema_get_switch_stmt_info(self)->used_values;
        while (htab_find(used, h, &res))
        {
                const tree_stmt* other = hiter_get_ptr(&res);
                const int_value* other_val = tree_get_case_cvalue(other);
                if (int_cmp(val, other_val) == CR_EQ)
                        return false;

                h += s_mix32(h);
        }

        htab_insert_ptr(used, h, label);
        return true;
}

extern bool csema_at_file_scope(const csema* self)
{
        return self->locals == self->globals;
}

extern bool csema_at_block_scope(const csema* self)
{
        return (bool)self->function;
}