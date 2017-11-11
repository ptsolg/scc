#ifndef CSEMA_H
#define CSEMA_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-error.h"
#include "c-tree.h"

// The structure, used for semantic analysis and building AST
typedef struct _csema
{
        tree_context* context;
        ctree_context* ccontext;
        cident_policy* id_policy;
        cerror_manager* error_manager;
        tree_decl* function;
        tree_decl_scope* globals;
        tree_decl_scope* labels;
        tree_decl_scope* locals;
        dseq switch_stack;
        tree_scope* scope;
        tree_module* module;
        tree_target_info* target;
} csema;

extern void csema_init(
        csema* self,
        ctree_context* context,
        cident_policy* id_policy,
        tree_module* module,
        cerror_manager* error_manager);

extern void csema_dispose(csema* self);

extern void csema_enter_scope(csema* self, tree_scope* scope);
extern void csema_exit_scope(csema* self);
extern void csema_enter_decl_scope(csema* self, tree_decl_scope* scope);
extern void csema_exit_decl_scope(csema* self);
extern void csema_enter_function(csema* self, tree_decl* func);
extern void csema_exit_function(csema* self);

// creates new decl scope, and enters it
// this is used when we need to create implicit decl scope.
// e.g: to hide for-loop variables from using them outside loop scope
extern void csema_push_scope(csema* self);

extern tree_id csema_get_decl_name(const csema* self, const tree_decl* d);

extern tree_decl* csema_get_local_tag_decl(const csema* self, tree_id name, bool parent_lookup);
extern tree_decl* csema_get_local_decl(const csema* self, tree_id name);
extern tree_decl* csema_get_global_decl(const csema* self, tree_id name);
extern tree_decl* csema_get_label_decl(const csema* self, tree_id name);

extern tree_decl* csema_require_decl(
        const csema* self,
        const tree_decl_scope* scope,
        tree_location name_loc,
        tree_decl_kind kind,
        tree_id name,
        bool parent_lookup);

extern tree_decl* csema_require_local_decl(
        const csema* self, tree_location name_loc, tree_decl_kind kind, tree_id name);

extern tree_decl* csema_require_global_decl(
        const csema* self, tree_location name_loc, tree_decl_kind kind, tree_id name);

extern tree_decl* csema_require_label_decl(
        const csema* self, tree_location name_loc, tree_id name);

extern tree_decl* csema_require_member_decl(
        const csema* self, tree_location name_loc, const tree_decl* record, tree_id name);

extern bool csema_require_complete_type(
        const csema* self, tree_location loc, const tree_type* type);

extern void csema_init_dseq_ptr(csema* self, dseq* group);
extern tree_module* cparser_act_on_finish_module(csema* self);

extern const char* csema_get_id(const csema* self, tree_id id);

extern bool csema_at_file_scope(const csema* self);
extern bool csema_at_block_scope(const csema* self);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_H
