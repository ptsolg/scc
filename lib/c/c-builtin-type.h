#ifndef C_BUILTIN_TYPE_H
#define C_BUILTIN_TYPE_H

#include "scc/tree/tree-type.h"

typedef struct
{
        tree_builtin_type_kind kind;
        int num_short;
        int num_long;
        int num_signed;
        int num_unsigned;
} c_builtin_type;

extern void c_builtin_type_init(c_builtin_type* self);
extern bool c_builtin_type_set_kind(c_builtin_type* self, tree_builtin_type_kind type);
extern tree_builtin_type_kind c_builtin_type_get_kind(const c_builtin_type* self);
extern bool c_builtin_type_add_signed_specifier(c_builtin_type* self);
extern bool c_builtin_type_add_unsigned_specifier(c_builtin_type* self);
extern bool c_builtin_type_add_short_specifier(c_builtin_type* self);
extern bool c_builtin_type_add_long_specifier(c_builtin_type* self);

#endif