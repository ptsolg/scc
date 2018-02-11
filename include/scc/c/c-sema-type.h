#ifndef CSEMA_TYPE_H
#define CSEMA_TYPE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern bool csema_types_are_same(const csema* self, const tree_type* a, const tree_type* b);
extern bool csema_types_are_compatible(const csema* self, const tree_type* a, const tree_type* b);
extern bool csema_require_complete_type(const csema* self, tree_location loc, const tree_type* type);

// returns one of: int32_t uint32_t int64_t uint64_t
extern tree_type* csema_get_int_type(csema* self, bool signed_, bool extended);
extern tree_type* csema_get_size_t_type(csema* self);
extern tree_type* csema_get_float_type(csema* self);
extern tree_type* csema_get_double_type(csema* self);
extern tree_type* csema_get_char_type(csema* self);
extern tree_type* csema_get_logical_operation_type(csema* self);
extern tree_type* csema_get_type_for_string_literal(csema* self, tree_id id);

extern tree_type* csema_new_builtin_type(
        csema* self, tree_type_quals q, tree_builtin_type_kind k);

extern tree_type* csema_new_decl_type(csema* self, tree_decl* d, bool referenced);
extern tree_type* csema_new_typedef_name(csema* self, tree_location name_loc, tree_id name);
extern tree_type* csema_new_pointer(csema* self, tree_type_quals quals, tree_type* target);
extern tree_type* csema_new_function_type(csema* self, tree_type* restype);
extern tree_type* csema_new_paren_type(csema* self, tree_type* next);

extern tree_type* csema_new_array_type(
        csema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size);

extern tree_type* csema_new_constant_array_type(
        csema* self, tree_type_quals quals, tree_type* eltype, uint size);

extern bool csema_typedef_name_exists(csema* self, tree_id name);

extern bool csema_check_array_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_function_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_type_quals(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_pointer_type(const csema* self, const tree_type* t, tree_location l);
extern bool csema_check_type(const csema* self, const tree_type* t, tree_location l);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_TYPE_H
