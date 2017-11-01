#ifndef CPROG_H
#define CPROG_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-error.h"
#include "c-tree.h"

// The structure, used for semantic analysis and building AST
typedef struct _cprog
{
        tree_context*     context;
        ctree_context*    ccontext;
        cerror_manager*   error_manager;
        tree_decl*        function;
        tree_decl_scope*  globals;
        tree_decl_scope*  labels;
        tree_decl_scope*  locals;
        tree_scope*       scope;
        tree_module*      module;
        tree_target_info* target;
} cprog;

extern void cprog_init(
        cprog*          self,
        ctree_context*  context,
        tree_module*    module,
        cerror_manager* error_manager);

extern void cprog_dispose(cprog* self);

extern void cprog_enter_scope(cprog* self, tree_scope* scope);
extern void cprog_exit_scope(cprog* self);
extern void cprog_enter_decl_scope(cprog* self, tree_decl_scope* scope);
extern void cprog_exit_decl_scope(cprog* self);
extern void cprog_enter_function(cprog* self, tree_decl* func);
extern void cprog_exit_function(cprog* self);

// creates new decl scope, and enters it
// this is used when we need to create implicit decl scope.
// e.g: to hide for-loop variables from using them outside loop scope
extern void cprog_push_scope(cprog* self);

extern tree_decl* cprog_get_local_decl(const cprog* self, tree_id name);
extern tree_decl* cprog_get_global_decl(const cprog* self, tree_id name);
extern tree_decl* cprog_get_label_decl(const cprog* self, tree_id name);

extern tree_decl* cprog_require_decl(
        const cprog*           self,
        const tree_decl_scope* scope,
        tree_location          name_loc,
        tree_decl_kind         kind,
        tree_id                name,
        bool                   lookup);

extern tree_decl* cprog_require_local_decl(
        const cprog* self, tree_location name_loc, tree_decl_kind kind, tree_id name);

extern tree_decl* cprog_require_global_decl(
        const cprog* self, tree_location name_loc, tree_decl_kind kind, tree_id name);

extern tree_decl* cprog_require_label_decl(
        const cprog* self, tree_location name_loc, tree_id name);

extern tree_decl* cprog_require_member_decl(
        const cprog* self, tree_location name_loc, const tree_decl* record, tree_id name);

extern bool cprog_require_complete_type(
        const cprog* self, tree_location loc, const tree_type* type);

extern void cprog_init_objgroup(cprog* self, objgroup* group);
extern tree_module* cparser_act_on_finish_module(cprog* self);

extern const char* cprog_get_id(const cprog* self, tree_id id);

extern bool cprog_at_file_scope(const cprog* self);
extern bool cprog_at_block_scope(const cprog* self);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_H