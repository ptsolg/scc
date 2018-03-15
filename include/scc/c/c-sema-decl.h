#ifndef C_SEMA_DECL_H
#define C_SEMA_DECL_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-type.h"

typedef struct _c_param c_param;

extern tree_decl* c_sema_local_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* c_sema_global_lookup(const c_sema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* c_sema_label_lookup(const c_sema* self, tree_id name);

extern tree_decl* c_sema_require_local_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_global_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_label_decl(const c_sema* self, tree_location name_loc, tree_id name);
extern tree_decl* c_sema_require_field_decl(
        const c_sema* self, const tree_decl* rec, tree_location name_loc, tree_id name);

typedef struct
{
        tree_type* head;
        tree_type* tail;
} c_type_chain;

extern void c_type_chain_init(c_type_chain* self);

typedef enum
{
        CDK_UNKNOWN,
        CDK_TYPE_NAME,
        CDK_PARAM,
        CDK_MEMBER,
} c_declarator_kind;

typedef struct _dseq dseq;

typedef struct _c_declarator
{
        c_declarator_kind kind;
        c_type_chain type;
        tree_id name;
        tree_location name_loc;
        tree_xlocation loc;
        dseq params;
        bool params_initialized;
        c_context* context;
} c_declarator;

extern void c_declarator_init(c_declarator* self, c_sema* sema, c_declarator_kind k);
extern void c_declarator_dispose(c_declarator* self);
extern void c_declarator_set_name(c_declarator* self, tree_location name_loc, tree_id name);
extern void c_declarator_set_initialized(c_declarator* self);
extern void c_declarator_set_loc_begin(c_declarator* self, tree_location begin);
extern void c_declarator_set_loc_end(c_declarator* self, tree_location end);
extern tree_type* c_declarator_get_type(const c_declarator* self);
extern tree_location c_declarator_get_name_loc_or_begin(const c_declarator* self);

extern bool c_sema_add_direct_declarator_function_suffix(c_sema* self, c_declarator* d);
extern bool c_sema_add_direct_declarator_array_suffix(
        c_sema* self, c_declarator* d, tree_type_quals q, tree_expr* size_expr);
extern bool c_sema_add_direct_declarator_parens(c_sema* self, c_declarator* d);
extern void c_sema_add_declarator_param(c_sema* self, c_declarator* d, c_param* p);
extern bool c_sema_set_declarator_has_vararg(c_sema* self, c_declarator* d, tree_location ellipsis_loc);
extern bool c_sema_finish_declarator(c_sema* self, c_declarator* d, c_type_chain* pointers);

typedef struct _c_decl_specs
{
        tree_decl_storage_class class_;
        tree_type* typespec;
        tree_function_specifier_kind funcspec;
        tree_xlocation loc;
        bool is_typedef;
} c_decl_specs;

extern void c_decl_specs_init(c_decl_specs* self);
extern void c_decl_specs_set_loc_begin(c_decl_specs* self, tree_location begin);
extern void c_decl_specs_set_loc_end(c_decl_specs* self, tree_location end);
extern tree_location c_decl_specs_get_loc_begin(const c_decl_specs* self);
extern tree_location c_decl_specs_get_loc_end(const c_decl_specs* self);

extern bool c_sema_set_type_specifier(c_sema* self, c_decl_specs* ds, tree_type* ts);
extern bool c_sema_set_typedef_specifier(c_sema* self, c_decl_specs* ds);
extern bool c_sema_set_inline_specifier(c_sema* self, c_decl_specs* ds);
extern bool c_sema_set_decl_storage_class(c_sema* self, c_decl_specs* ds, tree_decl_storage_class sc);
extern tree_decl* c_sema_handle_unused_decl_specs(c_sema* self, c_decl_specs* ds);

typedef struct _c_param
{
        c_decl_specs specs;
        c_declarator declarator;
} c_param;

extern tree_type* c_param_get_type(const c_param* self);
extern tree_xlocation c_param_get_loc(const c_param* self);

extern c_param* c_sema_new_param(c_sema* self);
extern c_param* c_sema_finish_param(c_sema* self, c_param* p);

extern tree_type* c_sema_new_type_name(c_sema* self, c_declarator* d, tree_type* typespec);

extern tree_decl* c_sema_add_decl_to_group(c_sema* self, tree_decl* group, tree_decl* decl);

extern tree_decl* c_sema_define_enumerator_decl(
        c_sema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* value);

extern tree_decl* c_sema_define_enum_decl(c_sema* self, tree_location kw_loc, tree_id name);
extern tree_decl* c_sema_declare_enum_decl(c_sema* self, tree_location kw_loc, tree_id name);
extern tree_decl* c_sema_complete_enum_decl(c_sema* self, tree_decl* enum_, tree_location end);

extern tree_decl* c_sema_define_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* c_sema_declare_record_decl(
        c_sema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* c_sema_complete_record_decl(c_sema* self, tree_decl* rec, tree_location end);

extern tree_decl* c_sema_define_field_decl(c_sema* self, c_decl_specs* ds, c_declarator* d, tree_expr* bit_width);

extern tree_decl* c_sema_define_label_decl(
        c_sema* self, tree_location name_loc, tree_id name, tree_location colon_loc, tree_stmt* stmt);
extern tree_decl* c_sema_declare_label_decl(c_sema* self, tree_location name_loc, tree_id name);

extern tree_decl* c_sema_declare_external_decl(c_sema* self, c_decl_specs* ds, c_declarator* d, bool has_init);
extern tree_decl* c_sema_define_var_decl(c_sema* self, tree_decl* var, tree_expr* init);
extern tree_decl* c_sema_define_func_decl(c_sema* self, tree_decl* func, tree_stmt* body);
extern bool c_sema_check_func_def_loc(c_sema* self, const tree_decl* func);

#ifdef __cplusplus
}
#endif

#endif
