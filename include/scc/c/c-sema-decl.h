#ifndef CSEMA_DECL_H
#define CSEMA_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

typedef struct _cparam cparam;

extern tree_decl* csema_local_lookup(const csema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* csema_global_lookup(const csema* self, tree_id name, tree_lookup_kind lk);
extern tree_decl* csema_label_lookup(const csema* self, tree_id name);

extern tree_decl* csema_require_local_decl(const csema* self, tree_location name_loc, tree_id name);
extern tree_decl* csema_require_global_decl(const csema* self, tree_location name_loc, tree_id name);
extern tree_decl* csema_require_label_decl(const csema* self, tree_location name_loc, tree_id name);
extern tree_decl* csema_require_field_decl(
        const csema* self, const tree_decl* rec, tree_location name_loc, tree_id name);

typedef struct
{
        tree_type* head;
        tree_type* tail;
} ctype_chain;

extern void ctype_chain_init(ctype_chain* self);

typedef enum
{
        CDK_UNKNOWN,
        CDK_TYPE_NAME,
        CDK_PARAM,
        CDK_MEMBER,
} cdeclarator_kind;

typedef struct _dseq dseq;

typedef struct _cdeclarator
{
        cdeclarator_kind kind;
        ctype_chain type;
        tree_id name;
        tree_location name_loc;
        tree_xlocation loc;
        dseq params;
        bool params_initialized;
        ccontext* context;
} cdeclarator;

extern void cdeclarator_init(cdeclarator* self, csema* sema, cdeclarator_kind k);
extern void cdeclarator_dispose(cdeclarator* self);
extern void cdeclarator_set_name(cdeclarator* self, tree_location name_loc, tree_id name);
extern void cdeclarator_set_initialized(cdeclarator* self);
extern void cdeclarator_set_loc_begin(cdeclarator* self, tree_location begin);
extern void cdeclarator_set_loc_end(cdeclarator* self, tree_location end);
extern tree_type* cdeclarator_get_type(const cdeclarator* self);
extern tree_location cdeclarator_get_name_loc_or_begin(const cdeclarator* self);

extern bool csema_add_direct_declarator_function_suffix(csema* self, cdeclarator* d);
extern bool csema_add_direct_declarator_array_suffix(
        csema* self, cdeclarator* d, tree_type_quals q, tree_expr* size_expr);
extern bool csema_add_direct_declarator_parens(csema* self, cdeclarator* d);
extern void csema_add_declarator_param(csema* self, cdeclarator* d, cparam* p);
extern bool csema_set_declarator_has_vararg(csema* self, cdeclarator* d, tree_location ellipsis_loc);
extern bool csema_finish_declarator(csema* self, cdeclarator* d, ctype_chain* pointers);

typedef struct _cdecl_specs
{
        tree_decl_storage_class class_;
        tree_type* typespec;
        tree_function_specifier_kind funcspec;
        tree_xlocation loc;
        bool is_typedef;
} cdecl_specs;

extern void cdecl_specs_init(cdecl_specs* self);
extern void cdecl_specs_set_loc_begin(cdecl_specs* self, tree_location begin);
extern void cdecl_specs_set_loc_end(cdecl_specs* self, tree_location end);
extern tree_location cdecl_specs_get_loc_begin(const cdecl_specs* self);
extern tree_location cdecl_specs_get_loc_end(const cdecl_specs* self);

extern bool csema_set_typespec(csema* self, cdecl_specs* ds, tree_type* ts);
extern bool csema_set_typedef_spec(csema* self, cdecl_specs* ds);
extern bool csema_set_inline_spec(csema* self, cdecl_specs* ds);
extern bool csema_set_decl_storage_class(csema* self, cdecl_specs* ds, tree_decl_storage_class sc);
extern tree_decl* csema_handle_unused_decl_specs(csema* self, cdecl_specs* ds);

typedef struct _cparam
{
        cdecl_specs specs;
        cdeclarator declarator;
} cparam;

extern tree_type* cparam_get_type(const cparam* self);
extern tree_xlocation cparam_get_loc(const cparam* self);

extern cparam* csema_new_param(csema* self);
extern cparam* csema_finish_param(csema* self, cparam* p);

extern tree_type* csema_new_type_name(csema* self, cdeclarator* d, tree_type* typespec);

extern tree_decl* csema_add_decl_to_group(csema* self, tree_decl* group, tree_decl* decl);

extern tree_decl* csema_define_enumerator_decl(
        csema* self, tree_decl* enum_, tree_location name_loc, tree_id name, tree_expr* value);

extern tree_decl* csema_define_enum_decl(csema* self, tree_location kw_loc, tree_id name);
extern tree_decl* csema_declare_enum_decl(csema* self, tree_location kw_loc, tree_id name);
extern tree_decl* csema_complete_enum_decl(csema* self, tree_decl* enum_, tree_location end);

extern tree_decl* csema_define_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* csema_declare_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union);
extern tree_decl* csema_complete_record_decl(csema* self, tree_decl* rec, tree_location end);

extern tree_decl* csema_define_field_decl(csema* self, cdecl_specs* ds, cdeclarator* d, tree_expr* bit_width);

extern tree_decl* csema_define_label_decl(
        csema* self, tree_location name_loc, tree_id name, tree_location colon_loc, tree_stmt* stmt);
extern tree_decl* csema_declare_label_decl(csema* self, tree_location name_loc, tree_id name);

extern tree_decl* csema_declare_external_decl(csema* self, cdecl_specs* ds, cdeclarator* d);
extern tree_decl* csema_define_var_decl(csema* self, tree_decl* var, tree_expr* init);
extern tree_decl* csema_define_func_decl(csema* self, tree_decl* func, tree_stmt* body);
extern bool csema_check_func_def_loc(csema* self, const tree_decl* func);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_DECL_H
