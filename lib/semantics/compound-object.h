#ifndef COMPOUND_OBJECT_H
#define COMPOUND_OBJECT_H

#include "scc/tree/decl.h"

typedef struct _tree_type tree_type;
typedef struct _tree_expr tree_expr;
typedef struct _c_sema c_sema;

enum object_kind
{
        OBJECT_ARRAY = 1,
        OBJECT_CONST_ARRAY = OBJECT_ARRAY | 2,
        OBJECT_INCOMPLETE_ARRAY = OBJECT_ARRAY | 4,

        OBJECT_RECORD = 8,
        OBJECT_STRUCT = OBJECT_RECORD | 16,
        OBJECT_UNION = OBJECT_RECORD | 32,
};

struct object
{
        enum object_kind kind;
        tree_type* type;
        tree_expr* sem_init;
        union
        {
                struct
                {
                        unsigned index;
                        unsigned size;
                } array;

                struct
                {
                        tree_decl* decl;
                        tree_decl* field;
                } record;
        };
};

struct compound_object
{
        struct object* object;
        tree_storage_class object_sc;
        struct object_stack* objects;
        c_sema* sema;
};

void init_compound_object(
        struct compound_object* self,
        c_sema* sema,
        tree_type* object,
        tree_storage_class object_sc);

void drop_compound_object(struct compound_object* self);
void set_compound_object_index(struct compound_object* self, unsigned index);
void set_compound_object_field(struct compound_object* self, tree_decl* field);
void finish_current_object(struct compound_object* self);
bool exit_subobject(struct compound_object* self);
bool goto_next_subobject(struct compound_object* self);
tree_type* get_subobject_type(const struct compound_object* self);
bool can_enter_subobject(const struct compound_object* self);
void enter_subobject(struct compound_object* self);
void initialize_subobject_with(struct compound_object* self, tree_expr* expr);

#endif
