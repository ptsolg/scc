#ifndef CSEMA_TYPE_H
#define CSEMA_TYPE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern tree_type* csema_new_builtin_type(
        csema* self, tree_type_quals q, tree_builtin_type_kind k);

extern tree_type* csema_new_decl_type(csema* self, tree_decl* d, bool referenced);

extern tree_type* csema_new_typedef_name(csema* self, tree_location name_loc, tree_id name);
extern bool csema_typedef_name_exists(csema* self, tree_id name);

extern tree_type* csema_new_pointer(csema* self, tree_type_quals quals, tree_type* target);
extern tree_type* csema_set_pointer_target(csema* self, tree_type* pointer, tree_type* target);

extern tree_type* csema_new_function_type(csema* self, tree_type* restype);

extern tree_type* csema_set_function_restype(
        csema* self, tree_type* func, tree_type* restype);

extern tree_type* csema_add_function_type_param(csema* self, tree_type* func, cparam* param);

extern tree_type* csema_new_paren_type(csema* self, tree_type* next);
extern tree_type* csema_set_paren_type(csema* self, tree_type* paren, tree_type* next);

extern tree_type* csema_new_array_type(
        csema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size);

extern tree_type* csema_new_constant_array_type(
        csema* self, tree_type_quals quals, tree_type* eltype, uint size);

extern tree_type* csema_set_array_eltype(
        csema* self, tree_type* array, tree_type* eltype);

extern tree_type* csema_set_type_quals(csema* self, tree_type* type, tree_type_quals quals);

extern bool csema_check_array_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_function_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_type_quals(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_pointer_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_type(const csema* self, const tree_type* t, tree_location l);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_TYPE_H
