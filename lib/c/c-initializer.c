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


extern void cinitialized_object_init(cinitialized_object* self, tree_type* type)
{
        type = tree_desugar_type(type);
        if (tree_type_is_record(type))
        {
                self->kind = CIOK_RECORD;
                self->record.decl = tree_get_decl_type_entity(type);
                self->record.field = tree_get_record_fields_begin(self->record.decl);
                cinitialized_object_skip_non_field_decls(self);
        }
        else if (tree_type_is_array(type))
        {
                self->kind = CIOK_ARRAY;
                self->array.type = type;
                self->array.index = 0;
        }
        else
        {
                self->kind = CIOK_SCALAR;
                self->scalar.initialized = false;
                self->scalar.type = type;
        }
}

extern bool cinitialized_object_valid(const cinitialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                return !self->scalar.initialized;
        else if (self->kind == CIOK_ARRAY)
        {
                if (tree_array_is(self->array.type, TAK_INCOMPLETE))
                        return true;

                uint array_size = int_get_u32(tree_get_constant_array_size_cvalue(self->array.type));
                return self->array.index < array_size;
        }
        else if (self->kind == CIOK_RECORD)
                return self->record.field != tree_get_record_fields_end(self->record.decl);

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
        else if (self->kind == CIOK_RECORD)
        {
                self->record.field = tree_get_next_decl(self->record.field);
                cinitialized_object_skip_non_field_decls(self);
        }
}

extern tree_type* cinitialized_object_get_subobject(const cinitialized_object* self)
{
        if (self->kind == CIOK_SCALAR)
                return self->scalar.type;
        else if (self->kind == CIOK_ARRAY)
                return tree_get_array_eltype(self->array.type);
        else if (self->kind == CIOK_RECORD)
                return tree_get_decl_type(self->record.field);

        S_UNREACHABLE();
        return NULL;
}
