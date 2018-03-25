#include "c-initializer.h"
#include "scc/c/c-context.h"
#include "scc/tree/tree-expr.h"

extern void c_initializer_init(c_initializer* self, tree_expr** init)
{
        self->expr = init;
        if (tree_expr_is(*init, TEK_INIT_LIST))
        {
                self->pos = tree_get_init_list_exprs_begin(*init);
                self->end = tree_get_init_list_exprs_end(*init);
        }
        else
        {
                self->pos = init;
                self->end = init + 1;
        }
}

static void c_initialized_object_skip_non_field_decls(c_initialized_object* self)
{
        tree_decl* end = tree_get_record_fields_end(self->record.decl);
        if (self->record.field == end)
                return;

        while (self->record.field != end)
        {
                if (tree_decl_is(self->record.field, TDK_FIELD))
                        return;

                self->record.field = tree_get_next_decl(self->record.field);
        }
}

static void c_initialized_object_set_parent(c_initialized_object* self, c_initialized_object* parent)
{
        c_initialized_object* semantical_parent = NULL;
        c_initialized_object* syntactical_parent = NULL;
        if (parent)
        {
                semantical_parent = parent;
                syntactical_parent = self->implicit && parent->implicit
                        ? parent->syntactical_parent
                        : parent;
        }

        self->semantical_parent = semantical_parent;
        self->syntactical_parent = syntactical_parent;
}

extern void c_initialized_object_init(
        c_initialized_object* self, tree_type* type, bool implicit, c_initialized_object* parent)
{
        type = tree_desugar_type(type);
        self->implicit = implicit;
        self->type = type;
        c_initialized_object_set_parent(self, parent);

        if (tree_type_is_record(type))
        {
                self->record.decl = tree_get_decl_type_entity(type);
                self->kind = tree_record_is_union(self->record.decl) ? CIOK_UNION : CIOK_STRUCT;
                self->record.field = tree_get_record_fields_begin(self->record.decl);
                self->record.union_field_initialized = false;
                c_initialized_object_skip_non_field_decls(self);
        }
        else if (tree_type_is_array(type))
        {
                self->kind = CIOK_ARRAY;
                self->array.index = 0;
                self->array.size = tree_array_is(type, TAK_CONSTANT)
                        ? tree_get_constant_array_size(type) : 0;
        }
        else
        {
                self->kind = CIOK_SCALAR;
                self->scalar.initialized = false;
        }
}

extern c_initialized_object* c_initialized_object_new(
        c_context* context, tree_type* type, bool implicit, c_initialized_object* parent)
{
        c_initialized_object* obj = c_context_allocate_node(context, sizeof(c_initialized_object));
        c_initialized_object_init(obj, type, implicit, parent);
        return obj;
}

extern c_initialized_object* c_initialized_object_new_rec(
        c_context* context, tree_decl* record, tree_decl* field, bool implicit, c_initialized_object* parent)
{
        c_initialized_object* obj = c_context_allocate_node(context, sizeof(c_initialized_object));
        obj->kind = tree_record_is_union(record) ? CIOK_UNION : CIOK_STRUCT;
        obj->implicit = implicit;
        obj->record.decl = record;
        obj->record.field = field;
        obj->record.union_field_initialized = false;
        c_initialized_object_set_parent(obj, parent);
        return obj;
}

extern void c_initialized_object_set_field(c_initialized_object* self, tree_decl* field)
{
        assert(self->kind == CIOK_STRUCT || self->kind == CIOK_UNION);
        assert(tree_get_field_record(field) == self->record.decl);
        self->record.field = field;
}

extern void c_initialized_object_set_index(c_initialized_object* self, uint index)
{
        assert(self->kind == CIOK_ARRAY);
        self->array.index = index;
        if (tree_array_is(self->type, TAK_INCOMPLETE) && index > self->array.size)
                self->array.size = index;
}

extern void c_initialized_object_set_initialized(c_initialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                self->scalar.initialized = true;
        else if (self->kind == CIOK_ARRAY && tree_array_is(self->type, TAK_CONSTANT))
                self->array.index = self->array.size;
        else if (self->kind == CIOK_STRUCT || self->kind == CIOK_UNION)
        {
                self->record.field = tree_get_record_fields_end(self->record.decl);
                self->record.union_field_initialized = true;
        }
}

extern bool c_initialized_object_valid(const c_initialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                return !self->scalar.initialized;
        else if (self->kind == CIOK_ARRAY)
        {
                return tree_array_is(self->type, TAK_INCOMPLETE)
                        ? true
                        : self->array.index < self->array.size;
        }
        else if (self->kind == CIOK_STRUCT)
                return self->record.field != tree_get_record_fields_end(self->record.decl);
        else if (self->kind == CIOK_UNION)
                return !self->record.union_field_initialized;

        UNREACHABLE();
        return false;
}

extern void c_initialized_object_next_subobject(c_initialized_object* self)
{
        assert(c_initialized_object_valid(self));

        if (self->kind == CIOK_SCALAR)
                self->scalar.initialized = true;
        else if (self->kind == CIOK_ARRAY)
                c_initialized_object_set_index(self, self->array.index + 1);
        else if (self->kind == CIOK_STRUCT || self->kind == CIOK_UNION)
        {
                self->record.field = tree_get_next_decl(self->record.field);
                self->record.union_field_initialized = true;
                c_initialized_object_skip_non_field_decls(self);
        }
}

extern tree_type* c_initialized_object_get_subobject(const c_initialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                return self->type;
        else if (self->kind == CIOK_ARRAY)
                return tree_get_array_eltype(self->type);
        else if (self->kind == CIOK_STRUCT || self->kind == CIOK_UNION)
                return tree_get_decl_type(self->record.field);

        UNREACHABLE();
        return NULL;
}
