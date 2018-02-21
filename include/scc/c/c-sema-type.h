#ifndef C_SEMA_TYPE_H
#define C_SEMA_TYPE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern bool c_sema_types_are_same(const c_sema* self, const tree_type* a, const tree_type* b);
extern bool c_sema_types_are_compatible(const c_sema* self, const tree_type* a, const tree_type* b);
extern bool c_sema_require_complete_type(const c_sema* self, tree_location loc, const tree_type* type);

// returns one of: int32_t uint32_t int64_t uint64_t
extern tree_type* c_sema_get_int_type(c_sema* self, bool signed_, bool extended);
extern tree_type* c_sema_get_size_t_type(c_sema* self);
extern tree_type* c_sema_get_float_type(c_sema* self);
extern tree_type* c_sema_get_double_type(c_sema* self);
extern tree_type* c_sema_get_char_type(c_sema* self);
extern tree_type* c_sema_get_logical_operation_type(c_sema* self);
extern tree_type* c_sema_get_type_for_string_literal(c_sema* self, tree_id id);

extern tree_type* c_sema_get_builtin_type(
        c_sema* self, tree_type_quals q, tree_builtin_type_kind k);

extern tree_type* c_sema_new_decl_type(c_sema* self, tree_decl* d, bool referenced);
extern tree_type* c_sema_new_typedef_name(c_sema* self, tree_location name_loc, tree_id name);
extern tree_type* c_sema_new_pointer_type(c_sema* self, tree_type_quals quals, tree_type* target);
extern tree_type* c_sema_new_function_type(c_sema* self, tree_type* restype);
extern tree_type* c_sema_new_paren_type(c_sema* self, tree_type* next);

extern tree_type* c_sema_new_array_type(
        c_sema* self,
        tree_type_quals quals,
        tree_type* eltype,
        tree_expr* size);

extern tree_type* c_sema_new_constant_array_type(
        c_sema* self, tree_type_quals quals, tree_type* eltype, uint size);

extern bool c_sema_typedef_name_exists(c_sema* self, tree_id name);

extern bool c_sema_check_array_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_function_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_type_quals(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_pointer_type(const c_sema* self, const tree_type* t, tree_location l);
extern bool c_sema_check_type(const c_sema* self, const tree_type* t, tree_location l);

#ifdef __cplusplus
}
#endif

#endif
