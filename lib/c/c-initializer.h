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
        CIOK_RECORD,
} cinitialized_object_kind;

typedef struct _cinitialized_subobject
{
        cinitialized_object_kind kind;
        union
        {
                struct
                {
                        tree_type* type;
                        uint index;
                } array;

                struct
                {
                        tree_decl* decl;
                        tree_decl* field;
                } record;

                struct
                {
                        tree_type* type;
                        bool initialized;
                } scalar;
        };
} cinitialized_object;

extern void cinitialized_object_init(cinitialized_object* self, tree_type* type);
extern bool cinitialized_object_valid(const cinitialized_object* self);
extern void cinitialized_object_next_subobject(cinitialized_object* self);
extern tree_type* cinitialized_object_get_subobject(const cinitialized_object* self);

#endif