#ifndef C_INITIALIZER_H
#define C_INITIALIZER_H

#include "scc/core/list.h"

typedef struct _tree_expr tree_expr;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _c_context c_context;

typedef struct
{
        tree_expr** expr;
        tree_expr** pos;
        tree_expr** end;
} c_initializer;

extern void c_initializer_init(c_initializer* self, tree_expr** init);

typedef enum
{
        CIOK_SCALAR,
        CIOK_ARRAY,
        CIOK_STRUCT,
        CIOK_UNION,
} c_initialized_object_kind;

typedef struct _c_initialized_object
{
        c_initialized_object_kind kind;
        tree_type* type;
        bool implicit;
        union
        {
                struct
                {
                        uint index;
                        uint size;
                } array;

                struct
                {
                        tree_decl* decl;
                        tree_decl* field;
                        bool union_field_initialized;
                } record;

                struct
                {
                        bool initialized;
                } scalar;
        };
        struct _c_initialized_object* semantical_parent;
        struct _c_initialized_object* syntactical_parent;
} c_initialized_object;

extern void c_initialized_object_init(
        c_initialized_object* self, tree_type* type, bool implicit, c_initialized_object* parent);

extern c_initialized_object* c_initialized_object_new(
        c_context* context, tree_type* type, bool implicit, c_initialized_object* parent);

extern c_initialized_object* c_initialized_object_new_rec(
        c_context* context, tree_decl* record, tree_decl* field, bool implicit, c_initialized_object* parent);

extern void c_initialized_object_set_field(c_initialized_object* self, tree_decl* field);
extern void c_initialized_object_set_index(c_initialized_object* self, uint index);
extern void c_initialized_object_set_initialized(c_initialized_object* self);

extern bool c_initialized_object_valid(const c_initialized_object* self);
extern void c_initialized_object_next_subobject(c_initialized_object* self);
extern tree_type* c_initialized_object_get_subobject(const c_initialized_object* self);

#endif