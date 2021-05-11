#include "compound-object.h"
#include "scc/core/common.h"
#include "scc/semantics/sema.h"
#include "scc/tree/context.h"
#include "scc/tree/type.h"
#include "scc/tree/target.h"

#define VEC object_stack
#define VEC_T struct object
#include "scc/core/vec.inc"

static bool maybe_init_from_parent(struct object* self, struct object* parent)
{
        if (!parent)
                return false;
        
        tree_expr* init = parent->sem_init;
        assert(init);

        if (parent->kind == OBJECT_UNION)
        {
                tree_expr* des = tree_get_init_list_expr(init, 0);
                if (!des)
                        return false;
                init = tree_get_designation_init(des);
                assert(init);
                if (tree_get_expr_type(init) == self->type)
                {
                        self->sem_init = init;
                        return true;
                }
                return false;
        }

        uint index = parent->kind & OBJECT_ARRAY
                ? parent->array.index : tree_get_field_index(parent->record.field);
        init = tree_get_init_list_expr(init, index);
        if (!init || tree_expr_is(init, TEK_UNKNOWN))
                return false;

        self->sem_init = init;
        return true;
}

static void init_object(struct object* self, struct object* parent, c_sema* sema, tree_type* t)
{
        unsigned init_size = 1;
        t = tree_desugar_type(t);
        self->type = t;

        if (tree_type_is_array(t))
        {
                self->kind = OBJECT_INCOMPLETE_ARRAY;
                self->array.index = 0;
                self->array.size = 1;
                if (tree_array_is(t, TAK_CONSTANT))
                {
                        self->kind = OBJECT_CONST_ARRAY;
                        self->array.size = tree_get_array_size(t);
                        init_size = self->array.size;
                }
        }
        else if (tree_type_is_record(t))
        {
                self->record.decl = tree_get_decl_type_entity(t);
                if (tree_record_is_union(self->record.decl))
                {
                        self->kind = OBJECT_UNION;
                        self->record.field = tree_get_first_union_field(self->record.decl);
                }
                else
                {
                        self->kind = OBJECT_STRUCT;
                        self->record.field = tree_skip_non_field_decls(
                                tree_get_record_fields_begin(self->record.decl));
                        init_size = tree_count_record_fields(self->record.decl);
                }
        }
        else
                UNREACHABLE();

        if (maybe_init_from_parent(self, parent))
                return;

        self->sem_init = tree_new_init_list_expr(sema->context, TREE_INVALID_LOC);
        tree_set_expr_type(self->sem_init, t);
        for (size_t i = 0; i < init_size; i++)
                tree_add_init_list_expr(self->sem_init, sema->context, NULL);
}

void init_compound_object(
        struct compound_object* self,
        c_sema* sema,
        tree_type* object,
        tree_storage_class object_sc)
{
        self->sema = sema;
        self->object_sc = object_sc;
        self->objects = object_stack_new();

        struct object o;
        init_object(&o, NULL, self->sema, object);
        object_stack_push(self->objects, o);
        self->object = object_stack_last_ptr(self->objects);
}

void drop_compound_object(struct compound_object* self)
{
        object_stack_del(self->objects);
}

void set_compound_object_index(struct compound_object* self, unsigned index)
{
        struct object* o = self->object;
        assert(o->kind & OBJECT_ARRAY);

        if (o->kind == OBJECT_CONST_ARRAY)
                assert(index < o->array.size);

        o->array.index = index;
        if (o->kind == OBJECT_INCOMPLETE_ARRAY && index >= o->array.size)
                o->array.size = index + 1;

        while (tree_get_init_list_exprs_size(o->sem_init) < o->array.size)
                tree_add_init_list_expr(o->sem_init, self->sema->context, NULL);
}

void set_compound_object_field(struct compound_object* self, tree_decl* field)
{
        struct object* o = self->object;
        assert(o->kind & OBJECT_RECORD);
        assert(tree_get_field_record(field) == o->record.decl);
        o->record.field = field;
}

static void _initialize_subobject_with(struct object* self, tree_context* ctx, tree_expr* expr)
{
        if (self->kind == OBJECT_UNION)
        {
                tree_location loc = tree_get_expr_loc(expr);
                tree_designator* fd = tree_new_field_designator(ctx, loc, self->record.field);
                tree_expr* des = tree_new_designation(ctx, loc, expr);
                assert(fd && des);
                tree_add_designation_designator(des, ctx, fd);
                tree_set_init_list_expr(self->sem_init, 0, des);
        }
        else
        {
                uint index = self->kind & OBJECT_ARRAY
                        ? self->array.index
                        : tree_get_field_index(self->record.field);
                tree_set_init_list_expr(self->sem_init, index, expr);
        }
}

static struct object* get_object_parent(const struct compound_object* self)
{
        assert(self->objects->size);
        return self->objects->size == 1
                ? NULL
                : self->objects->items + self->objects->size - 2;
}

void finish_current_object(struct compound_object* self)
{
        struct object* o = self->object;
        if (o->kind == OBJECT_INCOMPLETE_ARRAY)
                c_sema_set_incomplete_array_size(self->sema, o->type, o->array.size);

        if (o->kind & OBJECT_ARRAY)
        {
                tree_type* et = tree_get_array_eltype(o->type);
                TREE_FOREACH_INIT_LIST_EXPR(o->sem_init, it, end)
                        if (!*it)
                                *it = c_sema_get_default_initializer(self->sema, et, self->object_sc);
        }
        else if (o->kind == OBJECT_UNION)
        {
                tree_expr** v = tree_get_init_list_exprs_begin(o->sem_init);
                if (!*v)
                        *v = c_sema_get_default_initializer(self->sema,
                                tree_get_decl_type(o->record.field), self->object_sc);
        }
        else if (o->kind == OBJECT_STRUCT)
        {
                tree_decl* field = tree_skip_non_field_decls(tree_get_record_fields_begin(o->record.decl));
                TREE_FOREACH_INIT_LIST_EXPR(o->sem_init, it, end)
                {
                        if (!*it)
                                *it = c_sema_get_default_initializer(self->sema,
                                        tree_get_decl_type(field), self->object_sc);
                        field = tree_get_next_field(field);
                }
        }

        struct object* parent = get_object_parent(self);
        if (parent)
                _initialize_subobject_with(parent, self->sema->context, o->sem_init);
}

bool exit_subobject(struct compound_object* self)
{
        if (self->objects->size == 1)
                return false;
        finish_current_object(self);
        object_stack_pop(self->objects);
        self->object = object_stack_last_ptr(self->objects);
        return true;
}

bool goto_next_subobject(struct compound_object* self)
{
        for (int i = self->objects->size - 1; i >= 0; i--)
        {
               struct  object* o = self->objects->items + i;
                if (o->kind == OBJECT_INCOMPLETE_ARRAY)
                {
                        set_compound_object_index(self, o->array.index + 1);
                        return true;
                }
                else if (o->kind == OBJECT_CONST_ARRAY)
                {
                        if (o->array.index + 1 < o->array.size)
                        {
                                set_compound_object_index(self, o->array.index + 1);
                                return true;
                        }
                }
                else if (o->kind == OBJECT_STRUCT)
                {
                        tree_decl* next = tree_get_next_field(o->record.field);
                        tree_decl* end = tree_get_record_fields_end(o->record.decl);
                        if (next != end)
                        {
                                set_compound_object_field(self, next);
                                return true;
                        }
                }

                exit_subobject(self);
        }
        return false;
}

tree_type* get_subobject_type(const struct compound_object* self)
{
        tree_type* t = NULL;
        const struct object* o = self->object;
        if (o->kind & OBJECT_ARRAY)
                t = tree_get_array_eltype(o->type);
        else if (o->kind & OBJECT_RECORD)
                t = tree_get_decl_type(o->record.field);
        return t ?  tree_desugar_type(t) : NULL;
}

bool can_enter_subobject(const struct compound_object* self)
{
        tree_type* t = get_subobject_type(self);
        return tree_type_is_array(t) || tree_type_is_record(t);
}

void enter_subobject(struct compound_object* self)
{
        tree_type* t = get_subobject_type(self);
        assert(can_enter_subobject(self));

        struct object o;
        init_object(&o, self->object, self->sema, t);
        object_stack_push(self->objects, o);
        self->object = object_stack_last_ptr(self->objects);
}

void initialize_subobject_with(struct compound_object* self, tree_expr* expr)
{
        _initialize_subobject_with(self->object, self->sema->context, expr);
}
