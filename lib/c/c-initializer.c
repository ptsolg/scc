#include "c-initializer.h"
#include "scc/c/c-context.h"
#include "scc/tree/tree-expr.h"

extern void cinitializer_init(cinitializer* self, tree_expr** init)
{
        self->init = init;
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

static void cinitialized_object_skip_non_field_decls(cinitialized_object* self)
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

static void cinitialized_object_set_parent(cinitialized_object* self, cinitialized_object* parent)
{
        cinitialized_object* semantical_parent = NULL;
        cinitialized_object* syntactical_parent = NULL;
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

extern void cinitialized_object_init(
        cinitialized_object* self, tree_type* type, bool implicit, cinitialized_object* parent)
{
        type = tree_desugar_type(type);
        self->implicit = implicit;
        self->type = type;
        cinitialized_object_set_parent(self, parent);

        if (tree_type_is_record(type))
        {
                self->record.decl = tree_get_decl_type_entity(type);
                self->kind = tree_record_is_union(self->record.decl) ? CIOK_UNION : CIOK_STRUCT;
                self->record.field = tree_get_record_fields_begin(self->record.decl);
                self->record.union_field_initialized = false;
                cinitialized_object_skip_non_field_decls(self);
        }
        else if (tree_type_is_array(type))
        {
                self->kind = CIOK_ARRAY;
                self->array.index = 0;
                self->array.size = tree_array_is(type, TAK_CONSTANT)
                        ? int_get_u32(tree_get_constant_array_size_cvalue(type))
                        : 0;
        }
        else
        {
                self->kind = CIOK_SCALAR;
                self->scalar.initialized = false;
        }
}

extern cinitialized_object* cinitialized_object_new(
        ccontext* context, tree_type* type, bool implicit, cinitialized_object* parent)
{
        cinitialized_object* obj = callocate(context, sizeof(cinitialized_object));
        cinitialized_object_init(obj, type, implicit, parent);
        return obj;
}

extern cinitialized_object* cinitialized_object_new_rec(
        ccontext* context, tree_decl* record, tree_decl* field, bool implicit, cinitialized_object* parent)
{
        cinitialized_object* obj = callocate(context, sizeof(cinitialized_object));
        obj->kind = tree_record_is_union(record) ? CIOK_UNION : CIOK_STRUCT;
        obj->implicit = implicit;
        obj->record.decl = record;
        obj->record.field = field;
        obj->record.union_field_initialized = false;
        cinitialized_object_set_parent(obj, parent);
        return obj;
}

extern void cinitialized_object_set_field(cinitialized_object* self, tree_decl* field)
{
        S_ASSERT(self->kind == CIOK_STRUCT || self->kind == CIOK_UNION);
        S_ASSERT(tree_get_field_record(field) == self->record.decl);
        self->record.field = field;
}

extern void cinitialized_object_set_index(cinitialized_object* self, uint index)
{
        S_ASSERT(self->kind == CIOK_ARRAY);
        self->array.index = index;
}

extern void cinitialized_object_set_initialized(cinitialized_object* self)
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

extern bool cinitialized_object_valid(const cinitialized_object* self)
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

        S_UNREACHABLE();
        return false;
}

extern void cinitialized_object_next_subobject(cinitialized_object* self)
{
        S_ASSERT(cinitialized_object_valid(self));

        if (self->kind == CIOK_SCALAR)
                self->scalar.initialized = true;
        else if (self->kind == CIOK_ARRAY)
                self->array.index++;
        else if (self->kind == CIOK_STRUCT || self->kind == CIOK_UNION)
        {
                self->record.field = tree_get_next_decl(self->record.field);
                self->record.union_field_initialized = true;
                cinitialized_object_skip_non_field_decls(self);
        }
}

extern tree_type* cinitialized_object_get_subobject(const cinitialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                return self->type;
        else if (self->kind == CIOK_ARRAY)
                return tree_get_array_eltype(self->type);
        else if (self->kind == CIOK_STRUCT || self->kind == CIOK_UNION)
                return tree_get_decl_type(self->record.field);

        S_UNREACHABLE();
        return NULL;
}
