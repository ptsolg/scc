#ifndef CBUILTIN_TYPE_H
#define CBUILTIN_TYPE_H

#include "scc/tree/tree-type.h"

typedef struct
{
        tree_builtin_type_kind kind;
        int num_short;
        int num_long;
        int num_signed;
        int num_unsigned;
} cbuiltin_type;

extern void cbuiltin_type_init(cbuiltin_type* self);
extern bool cbuiltin_type_set_kind(cbuiltin_type* self, tree_builtin_type_kind type);
extern tree_builtin_type_kind cbuiltin_type_get_kind(const cbuiltin_type* self);
extern bool cbuiltin_type_add_signed_specifier(cbuiltin_type* self);
extern bool cbuiltin_type_add_unsigned_specifier(cbuiltin_type* self);
extern bool cbuiltin_type_add_short_specifier(cbuiltin_type* self);
extern bool cbuiltin_type_add_long_specifier(cbuiltin_type* self);

#endif