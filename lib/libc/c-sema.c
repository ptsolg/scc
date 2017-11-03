#include "c-sema.h"
#include "c-info.h"
#include <libscl/bit-utils.h>

extern void csema_init(
        csema* self,
        ctree_context* context,
        cident_info* id_info,
        tree_module* module,
        cerror_manager* error_manager)
{
        self->ccontext = context;
        self->context = ctree_context_base(context);
        self->id_info = id_info;
        self->module = module;
        self->labels = NULL;
        self->globals = tree_get_module_globals(module);
        self->target = tree_get_module_target(module);
        self->locals = self->globals;
        self->scope = NULL;
        self->function = NULL;
        self->error_manager = error_manager;
}

extern void csema_dispose(csema* self)
{
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

extern tree_id csema_get_decl_name(const csema* self, const tree_decl* d)
{
        return cident_info_get_orig_decl_name(self->id_info, d);
}

extern void csema_init_objgroup(csema* self, objgroup* args)
{
        objgroup_init_ex(args, tree_get_context_allocator(self->context));
}

extern tree_decl* csema_get_local_tag_decl(const csema* self, tree_id name, bool parent_lookup)
{
        if (tree_id_is_empty(name))
                return NULL;

        return tree_decl_scope_find(self->locals,
                cident_info_to_tag(self->id_info, name), parent_lookup);
}

extern tree_decl* csema_get_local_decl(const csema* self, tree_id name)
{
        return tree_decl_scope_find(self->locals, name, true);
}

extern tree_decl* csema_get_global_decl(const csema* self, tree_id name)
{
        return tree_decl_scope_find(self->globals, name, false);
}

extern tree_decl* csema_get_label_decl(const csema* self, tree_id name)
{
        return tree_decl_scope_find(self->labels, name, false);
}

extern tree_decl* csema_require_decl(
        const csema* self,
        const tree_decl_scope* scope,
        tree_location name_loc,
        tree_decl_kind kind,
        tree_id name,
        bool parent_lookup)
{
        tree_decl* d = tree_symtab_get(tree_get_decl_scope_csymtab(scope), name, parent_lookup);
        if (!d)
        {
                cerror(self->error_manager, CES_ERROR, name_loc,
                        "undeclared identifier '%s'",
                        tree_context_get_id(self->context, name));
                return NULL;
        }

        if (kind != TDK_UNKNOWN && tree_get_decl_kind(d) != kind)
                return NULL;

        return d;
}

extern tree_decl* csema_require_local_decl(
        const csema* self, tree_location name_loc, tree_decl_kind kind, tree_id name)
{
        return csema_require_decl(self, self->locals, name_loc, kind, name, true);
}

extern tree_decl* csema_require_global_decl(
        const csema* self, tree_location name_loc, tree_decl_kind kind, tree_id name)
{
        return csema_require_decl(self, self->locals, name_loc, kind, name, false);
}

extern tree_decl* csema_require_label_decl(
        const csema* self, tree_location name_loc, tree_id name)
{
        return csema_require_decl(self, self->locals, name_loc, TDK_LABEL, name, false);
}

extern tree_decl* csema_require_member_decl(
        const csema* self, tree_location name_loc, const tree_decl* record, tree_id name)
{
        return csema_require_decl(self,
                tree_get_record_cscope(record), name_loc, TDK_MEMBER, name, false);
}

extern bool csema_require_complete_type(
        const csema* self, tree_location loc, const tree_type* type)
{
        if (tree_type_is_incomplete(type))
        {
                cerror(self->error_manager, CES_ERROR, loc,
                        "incomplete type is not allowed");
                return false;
        }
        return true;
}

extern tree_module* cparser_act_on_finish_module(csema* self)
{
        return self->module;
}

extern const char* csema_get_id(const csema* self, tree_id id)
{
        return tree_context_get_id(self->context, id);
}

extern bool csema_at_file_scope(const csema* self)
{
        return self->locals == self->globals;
}

extern bool csema_at_block_scope(const csema* self)
{
        return (bool)self->function;
}