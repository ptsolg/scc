#ifndef CINITIALiZER_H
#define CINITIALiZER_H

#include "scc/scl/list.h"

typedef struct _tree_expr tree_expr;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _ccontext ccontext;
typedef struct _dseq dseq;

typedef struct
{
        tree_expr** init;
        tree_expr** pos;
        tree_expr** end;
} cinitializer;

extern void cinitializer_init(cinitializer* self, tree_expr** init);

typedef enum
{
        CIOK_SCALAR,
        CIOK_ARRAY,
        CIOK_STRUCT,
        CIOK_UNION,
} cinitialized_object_kind;

typedef struct _cinitialized_object
{
        cinitialized_object_kind kind;
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
        struct _cinitialized_object* semantical_parent;
        struct _cinitialized_object* syntactical_parent;
} cinitialized_object;

extern void cinitialized_object_init(
        cinitialized_object* self, tree_type* type, bool implicit, cinitialized_object* parent);

extern cinitialized_object* cinitialized_object_new(
        ccontext* context, tree_type* type, bool implicit, cinitialized_object* parent);

extern cinitialized_object* cinitialized_object_new_rec(
        ccontext* context, tree_decl* record, tree_decl* field, bool implicit, cinitialized_object* parent);

extern void cinitialized_object_set_field(cinitialized_object* self, tree_decl* field);
extern void cinitialized_object_set_index(cinitialized_object* self, uint index);
extern void cinitialized_object_set_initialized(cinitialized_object* self);

extern bool cinitialized_object_valid(const cinitialized_object* self);
extern void cinitialized_object_next_subobject(cinitialized_object* self);
extern tree_type* cinitialized_object_get_subobject(const cinitialized_object* self);

#endif