#include "scc/semantics/sema.h"
#include "scc/core/hashmap.h"
#include "scc/tree/decl.h"
#include "scc/tree/stmt.h"
#include "scc/tree/context.h"
#include "scc/tree/module.h"
#include "errors.h"

extern void c_sema_init(c_sema* self, c_context* context)
{
        self->ccontext = context;
        self->context = context->tree;
        self->module = NULL;
        self->globals = NULL;
        self->target = NULL;
        self->locals = NULL;
        self->function = NULL;
        self->labels = NULL;
        self->scope = NULL;
        self->tm_info.atomic_stmt_nesting = 0;

        c_switch_stack_init(&self->switch_stack);
        vec_init(&self->tm_info.non_atomic_gotos);
        hashmap_init(&self->tm_info.atomic_labels);
}

extern void c_sema_dispose(c_sema* self)
{
        // todo
}

extern tree_module* c_sema_new_module(c_sema* self)
{
        self->module = tree_new_module(self->context);
        self->globals = tree_get_module_globals(self->module);
        self->target = tree_get_module_target(self->module);
        self->locals = self->globals;
        return self->module;
}

extern void c_sema_enter_scope(c_sema* self, tree_scope* scope)
{
        assert(tree_get_scope_parent(scope) == self->scope
                && "scopes should be connected.");
        self->scope = scope;
        c_sema_enter_decl_scope(self, tree_get_scope_decls(scope));
}

extern void c_sema_exit_scope(c_sema* self)
{
        assert(self->scope);
        self->scope = tree_get_scope_parent(self->scope);
        c_sema_exit_decl_scope(self);
}

extern void c_sema_enter_decl_scope(c_sema* self, tree_decl_scope* scope)
{
        assert(tree_get_decl_scope_parent(scope) == self->locals
                && "scopes should be connected.");
        self->locals = scope;
}

extern void c_sema_exit_decl_scope(c_sema* self)
{
        tree_decl_scope* parent = tree_get_decl_scope_parent(self->locals);
        assert(parent);
        self->locals = parent;
}

extern void c_sema_enter_function(c_sema* self, tree_decl* func)
{
        self->labels = tree_get_func_labels(func);
        self->function = func;
        c_sema_enter_decl_scope(self, tree_get_func_params(func));
}

extern void c_sema_exit_function(c_sema* self)
{
        self->labels = NULL;
        self->function = NULL;
        c_sema_exit_decl_scope(self);

        vec_resize(&self->tm_info.non_atomic_gotos, 0);
        hashmap_clear(&self->tm_info.atomic_labels);
}

extern void c_sema_push_scope(c_sema* self)
{
        c_sema_enter_decl_scope(self, tree_new_decl_scope(self->context, self->locals));
}

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt)
{
        c_switch_stmt info;
        info.has_default_label = false;
        info.in_atomic_block = false;
        info.switch_stmt = switch_stmt;
        hashmap_init(&info.labels);
        c_switch_stack_push(&self->switch_stack, info);
}

extern void c_sema_pop_switch_stmt_info(c_sema* self)
{
        hashmap_drop(&c_sema_get_switch_stmt_info(self)->labels);
        c_switch_stack_pop(&self->switch_stack);
}

extern void c_sema_set_switch_stmt_has_default_label(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->has_default_label = true;
}

extern void c_sema_set_switch_stmt_in_atomic_block(c_sema* self)
{
        c_sema_get_switch_stmt_info(self)->in_atomic_block = true;
}

extern c_switch_stmt* c_sema_get_switch_stmt_info(const c_sema* self)
{
        return c_switch_stack_last_ptr(&self->switch_stack);
}

extern bool c_sema_in_switch_stmt(const c_sema* self)
{
        return self->switch_stack.size != 0;
}

extern bool c_sema_switch_stmt_has_default_label(const c_sema* self)
{
        return c_sema_get_switch_stmt_info(self)->has_default_label;
}

extern bool c_sema_switch_stmt_in_atomic_block(const c_sema* self)
{
        return c_sema_get_switch_stmt_info(self)->in_atomic_block;
}

extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label)
{
        const struct num* val = tree_get_case_cvalue(label);
        uint32_t key = num_as_u64(val);

        struct hashmap* labels = &c_sema_get_switch_stmt_info(self)->labels;
        for (uint32_t i = 1; ; i++)
        {
                struct hashmap_entry* entry = hashmap_lookup(labels, key);
                if (entry && num_cmp(val, tree_get_case_cvalue(entry->value)) == 0)
                        return false;
                else if (hashmap_key_ok(key))
                        break;
                key += i;
        }

        hashmap_insert(labels, key, label);
        return true;
}

extern bool c_sema_at_file_scope(const c_sema* self)
{
        return self->locals == self->globals;
}

extern bool c_sema_at_block_scope(const c_sema* self)
{
        return (bool)self->function;
}

extern bool c_sema_in_atomic_block(const c_sema* self)
{
        return self->tm_info.atomic_stmt_nesting != 0;
}

extern bool c_sema_in_transaction_safe_function(const c_sema* self)
{
        return self->function
                && tree_func_type_is_transaction_safe(
                        tree_desugar_type(tree_get_decl_type(self->function)));
}

extern bool c_sema_in_transaction_safe_block(const c_sema* self)
{
        return c_sema_in_atomic_block(self) || c_sema_in_transaction_safe_function(self);
}
