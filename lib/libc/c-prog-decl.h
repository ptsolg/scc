#ifndef CPROG_DECL_H
#define CPROG_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern void cprog_init_declarator(cprog* self, cdeclarator* declarator, cdeclarator_kind k);

extern bool cprog_build_direct_declarator_suffix(
        cprog* self, cdeclarator* declarator, tree_type* suffix);

extern bool cprog_build_direct_declarator_function_suffix(
        cprog* self, cdeclarator* declarator);

extern bool cprog_build_direct_declarator_array_suffix(
        cprog* self, cdeclarator* declarator, tree_type_quals quals, tree_exp* size);

extern bool cprog_add_direct_declarator_parens(cprog* self, cdeclarator* declarator);
extern bool cprog_set_declarator_name(
        cprog* self, tree_location loc, cdeclarator* declarator, tree_id name);

extern bool cprog_finish_declarator(
        cprog* self, cdeclarator* declarator, ctype_chain* pointer_chain);

extern tree_decl* cprog_export_decl(cprog* self, tree_decl_scope* scope, tree_decl* decl);

extern tree_decl* cprog_finish_decl(cprog* self, tree_decl* decl);
extern tree_decl* cprog_finish_decl_ex(cprog* self, tree_location end_loc, tree_decl* decl);

extern tree_type* cprog_build_type_name(
        cprog* self, cdeclarator* declarator, tree_type* typespec);

extern tree_decl* cprog_build_enumerator(
        cprog* self, tree_decl* enum_, tree_id id, tree_id id_loc, tree_exp* value);

extern tree_decl* cprog_build_enum_decl(
        cprog* self, tree_location kw_loc, tree_id name, bool has_body);

extern tree_decl* cprog_build_member_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* struct_declarator, tree_exp* bits);

extern tree_decl* cprog_build_record_decl(
        cprog* self, tree_location kw_loc, tree_id name, bool is_union, bool has_body);

extern tree_decl* cprog_build_external_decl(
        cprog* self, cdecl_specs* decl_specs, cdeclarator* declarator);

extern tree_decl* cprog_add_init_declarator(cprog* self, tree_decl* list, tree_decl* d);

extern bool cprog_set_var_initializer(cprog* self, tree_decl* decl, tree_exp* init);
extern bool cprog_set_function_body(cprog* self, tree_decl* decl, tree_stmt* body);

extern cparam* cprog_build_param(cprog* self);
extern void    cprog_handle_unused_param(cprog* self, cparam* p);
extern cparam* cprog_add_declarator_param(cprog* self, cdeclarator* d, cparam* p);
extern cparam* cprog_finish_param(cprog* self, cparam* p);

extern tree_decl* cprog_handle_unused_decl_specs(cprog* self, cdecl_specs* specs);

extern bool cprog_set_typespec(cprog* self, cdecl_specs* specs, tree_type* typespec);
extern bool cprog_set_typedef_specifier(cprog* self, cdecl_specs* specs);
extern bool cprog_set_inline_specifier(cprog* self, cdecl_specs* specs);
extern bool cprog_set_decl_storage_class(
        cprog* self, cdecl_specs* specs, tree_decl_storage_class class_);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_DECL_H