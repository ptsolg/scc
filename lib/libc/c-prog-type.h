#ifndef CPROG_TYPE_H
#define CPROG_TYPE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern tree_type* cprog_build_builtin_type(
        cprog* self, tree_type_quals q, tree_builtin_type_kind k);

extern tree_type* cprog_build_decl_type(cprog* self, tree_decl* d, bool referenced);

extern tree_type* cprog_build_typedef_name(cprog* self, tree_location name_loc, tree_id name);
extern bool       cprog_typedef_name_exists(cprog* self, tree_id name);

extern tree_type* cprog_build_pointer(cprog* self, tree_type_quals quals, tree_type* target);
extern tree_type* cprog_set_pointer_target(cprog* self, tree_type* pointer, tree_type* target);

extern tree_type* cprog_build_function_type(
        cprog* self, tree_location loc, tree_type* restype);

extern tree_type* cprog_set_function_restype(
        cprog* self, tree_location loc, tree_type* func, tree_type* restype);

extern tree_type* cprog_add_function_type_param(cprog* self, tree_type* func, cparam* param);

extern tree_type* cprog_build_paren_type(cprog* self, tree_type* next);
extern tree_type* cprog_set_paren_type(cprog* self, tree_type* paren, tree_type* next);

extern tree_type* cprog_build_array_type(
        cprog*          self,
        tree_location   loc,
        tree_type_quals quals,
        tree_type*      eltype,
        tree_const_exp* size);

extern tree_type* cprog_set_array_eltype(
        cprog* self, tree_location loc, tree_type* array, tree_type* eltype);

extern tree_type* cprog_set_type_quals(cprog* self, tree_type* type, tree_type_quals quals);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_TYPE_H