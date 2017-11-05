#ifndef CSEMA_DECL_H
#define CSEMA_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern void csema_init_declarator(csema* self, cdeclarator* declarator, cdeclarator_kind k);

extern bool csema_new_direct_declarator_suffix(
        csema* self, cdeclarator* declarator, tree_type* suffix);

extern bool csema_new_direct_declarator_function_suffix(
        csema* self, cdeclarator* declarator);

extern bool csema_new_direct_declarator_array_suffix(
        csema* self, cdeclarator* declarator, tree_type_quals quals, tree_expr* size);

extern bool csema_add_direct_declarator_parens(csema* self, cdeclarator* declarator);
extern bool csema_set_declarator_name(
        csema* self, tree_location loc, cdeclarator* declarator, tree_id name);

extern bool csema_finish_declarator(
        csema* self, cdeclarator* declarator, ctype_chain* pointer_chain);

extern tree_decl* csema_export_decl(csema* self, tree_decl_scope* scope, tree_decl* decl);

extern tree_decl* csema_finish_decl(csema* self, tree_decl* decl);
extern tree_decl* csema_finish_decl_ex(csema* self, tree_location end_loc, tree_decl* decl);

extern tree_type* csema_new_type_name(
        csema* self, cdeclarator* declarator, tree_type* typespec);

extern tree_decl* csema_new_enumerator(
        csema* self, tree_decl* enum_, tree_id id, tree_id id_loc, tree_expr* value);

extern tree_decl* csema_new_enum_decl(
        csema* self, tree_location kw_loc, tree_id name, bool has_body);

extern tree_decl* csema_new_member_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_expr* bits);

extern tree_decl* csema_new_record_decl(
        csema* self, tree_location kw_loc, tree_id name, bool is_union, bool has_body);

extern tree_decl* csema_new_external_decl(
        csema* self, cdecl_specs* decl_specs, cdeclarator* declarator);

extern tree_decl* csema_add_init_declarator(csema* self, tree_decl* list, tree_decl* d);

extern bool csema_set_var_initializer(csema* self, tree_decl* decl, tree_expr* init);
extern bool csema_check_function_definition_location(csema* self, tree_decl* decl);
extern bool csema_set_function_body(csema* self, tree_decl* decl, tree_stmt* body);

extern cparam* csema_new_param(csema* self);
extern void csema_handle_unused_param(csema* self, cparam* p);
extern cparam* csema_add_declarator_param(csema* self, cdeclarator* d, cparam* p);
extern cparam* csema_finish_param(csema* self, cparam* p);

extern tree_decl* csema_handle_unused_decl_specs(csema* self, cdecl_specs* specs);

extern bool csema_set_typespec(csema* self, cdecl_specs* specs, tree_type* typespec);
extern bool csema_set_typedef_specifier(csema* self, cdecl_specs* specs);
extern bool csema_set_inline_specifier(csema* self, cdecl_specs* specs);
extern bool csema_set_decl_storage_class(
        csema* self, cdecl_specs* specs, tree_decl_storage_class class_);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_DECL_H
