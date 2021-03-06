#include "scc/tree/target.h"
#include "scc/tree/eval.h"

extern void tree_init_target_info(tree_target_info* self, tree_target_architecture_kind k)
{
        self->kind = k;
        
        self->builtin_align[TBTK_INVALID] = 0;
        self->builtin_align[TBTK_VOID] = 1;
        self->builtin_align[TBTK_INT8] = 1;
        self->builtin_align[TBTK_UINT8] = 1;
        self->builtin_align[TBTK_INT16] = 2;
        self->builtin_align[TBTK_UINT16] = 2;
        self->builtin_align[TBTK_INT32] = 4;
        self->builtin_align[TBTK_UINT32] = 4;
        self->builtin_align[TBTK_FLOAT] = 4;

        self->builtin_size[TBTK_INVALID] = 0;
        self->builtin_size[TBTK_VOID] = 1;
        self->builtin_size[TBTK_INT8] = 1;
        self->builtin_size[TBTK_UINT8] = 1;
        self->builtin_size[TBTK_INT16] = 2;
        self->builtin_size[TBTK_UINT16] = 2;
        self->builtin_size[TBTK_INT32] = 4;
        self->builtin_size[TBTK_UINT32] = 4;
        self->builtin_size[TBTK_INT64] = 8;
        self->builtin_size[TBTK_UINT64] = 8;
        self->builtin_size[TBTK_FLOAT] = 4;
        self->builtin_size[TBTK_DOUBLE] = 8;

        if (k == TTAK_X86_32)
        {
                self->pointer_align = 4;
                self->pointer_size = 4;

                self->builtin_align[TBTK_INT64] = 4;
                self->builtin_align[TBTK_UINT64] = 4;
                self->builtin_align[TBTK_DOUBLE] = 8;
        }
        else if (k == TTAK_X86_64)
        {
                self->pointer_align = 8;
                self->pointer_size = 8;

                self->builtin_align[TBTK_INT64] = 8;
                self->builtin_align[TBTK_UINT64] = 8;
                self->builtin_align[TBTK_FLOAT] = 8;
                self->builtin_align[TBTK_DOUBLE] = 8;
        }
        else
                UNREACHABLE();
}

extern tree_target_architecture_kind tree_get_target_kind(const tree_target_info* self)
{
        return self->kind;
}

extern bool tree_target_is(const tree_target_info* self, tree_target_architecture_kind k)
{
        return tree_get_target_kind(self) == k;
}

extern size_t tree_get_pointer_size(const tree_target_info* self)
{
        return self->pointer_size;
}

extern size_t tree_get_pointer_align(const tree_target_info* self)
{
        return self->pointer_align;
}

extern size_t tree_get_intmax_t_size(const tree_target_info* self)
{
        return tree_get_builtin_type_size(self, TBTK_INT64);
}

extern size_t tree_get_uintmax_t_size(const tree_target_info* self)
{
        return tree_get_builtin_type_size(self, TBTK_UINT64);
}

extern size_t tree_get_builtin_type_size(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->builtin_size[k];
}

extern size_t tree_get_builtin_type_align(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->builtin_align[k];
}

extern size_t tree_get_sizeof_record(const tree_target_info* info, const tree_decl* d)
{
        bool is_union = tree_record_is_union(d);
        const tree_decl_scope* scope = tree_get_record_cfields(d);
        size_t total_size = 0;

        // todo: padding?
        TREE_FOREACH_DECL_IN_SCOPE(scope, member)
        {
                if (!tree_decl_is(member, TDK_FIELD))
                        continue;

                size_t member_size = tree_get_sizeof(info, tree_get_decl_type(member));
                size_t max_size = total_size > member_size ? total_size : member_size;
                total_size = is_union
                        ? max_size
                        : total_size + member_size;
        }

        return total_size;
}

extern size_t tree_get_sizeof(const tree_target_info* info, const tree_type* t)
{
        assert(t);
        t = tree_desugar_type_c(t);

        if (tree_type_is_pointer(t))
                return tree_get_pointer_size(info);
        else if (tree_type_is(t, TTK_BUILTIN))
                return tree_get_builtin_type_size(info, tree_get_builtin_type_kind(t));
        else if (tree_type_is(t, TTK_DECL))
        {
                tree_decl* entity = tree_get_decl_type_entity(t);
                tree_decl_kind dk = tree_get_decl_kind(entity);

                if (dk == TDK_ENUM)
                        return tree_get_builtin_type_size(info, TBTK_INT32);
                else if (dk == TDK_RECORD)
                        return tree_get_sizeof_record(info, entity);
        }
        else if (tree_type_is(t, TTK_ARRAY) && tree_array_is(t, TAK_CONSTANT))
                return tree_get_array_size(t) * tree_get_sizeof(info, tree_get_array_eltype(t));

        // probably function type
        return 0;
}

extern size_t tree_get_alignof_record(const tree_target_info* info, const tree_decl* d)
{
        tree_expr* alignment = tree_get_record_alignment(d);
        if (alignment)
        {
                tree_eval_result er;
                bool ok = tree_eval_expr_as_integer(info, alignment, &er);
                assert(ok);
                return num_as_u64(&er.value);
        }

        bool is_union = tree_record_is_union(d);
        const tree_decl_scope* scope = tree_get_record_cfields(d);
        size_t max_align = 0;

        TREE_FOREACH_DECL_IN_SCOPE(scope, member)
        {
                if (!tree_decl_is(member, TDK_FIELD))
                        continue;
                size_t member_align = tree_get_alignof(info, tree_get_decl_type(member));
                if (member_align > max_align)
                        max_align = member_align;
        }

        return max_align;
}

extern size_t tree_get_alignof(const tree_target_info* info, const tree_type* t)
{
        assert(t);
        t = tree_desugar_type_c(t);

        if (tree_type_is_pointer(t))
                return tree_get_pointer_align(info);
        else if (tree_type_is(t, TTK_BUILTIN))
                return tree_get_builtin_type_align(info, tree_get_builtin_type_kind(t));
        else if (tree_type_is(t, TTK_DECL))
        {
                tree_decl* entity = tree_get_decl_type_entity(t);
                tree_decl_kind dk = tree_get_decl_kind(entity);

                if (dk == TDK_ENUM)
                        return tree_get_builtin_type_align(info, TBTK_INT32);
                else if (dk == TDK_RECORD)
                        return tree_get_alignof_record(info, entity);
        }
        else if (tree_type_is(t, TTK_ARRAY))
                return tree_get_alignof(info, tree_get_array_eltype(t));

        // probably function type
        return 0;
}

static size_t tree_get_offsetof_field(const tree_target_info* info, const tree_decl* field)
{
        assert(tree_decl_is(field, TDK_FIELD));
        tree_decl* record = tree_get_field_record(field);
        if (tree_record_is_union(record))
                return 0;

        size_t offset = 0;
        const tree_decl_scope* scope = tree_get_record_cfields(record);
        TREE_FOREACH_DECL_IN_SCOPE(scope, it)
        {
                if (!tree_decl_is(it, TDK_FIELD))
                        continue;
                if (it == field)
                        break;
                offset += tree_get_sizeof(info, tree_get_decl_type(it));
        }

        return offset;
}

extern size_t tree_get_offsetof(const tree_target_info* info, const tree_decl* field)
{
        assert(tree_decl_is(field, TDK_FIELD) || tree_decl_is(field, TDK_INDIRECT_FIELD));

        size_t offset = 0;
        while (tree_decl_is(field, TDK_INDIRECT_FIELD))
        {
                tree_decl* anon_field = tree_get_indirect_field_anon_record(field);
                offset += tree_get_offsetof_field(info, anon_field);
                tree_decl* anon_rec = tree_get_decl_type_entity(tree_get_decl_type(anon_field));
                field = tree_decl_scope_lookup(tree_get_record_fields(anon_rec),
                        TLK_DECL, tree_get_decl_name(field), false);
        }

        return offset + tree_get_offsetof_field(info, field);
}
