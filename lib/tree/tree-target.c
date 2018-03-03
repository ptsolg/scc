#include "scc/tree/tree-target.h"
#include "scc/tree/tree-eval.h"

extern void tree_init_target_info(tree_target_info* self, tree_target_architecture_kind k)
{
        self->_kind = k;
        
        self->_builtin_align[TBTK_INVALID] = 0;
        self->_builtin_align[TBTK_VOID] = 1;
        self->_builtin_align[TBTK_INT8] = 1;
        self->_builtin_align[TBTK_UINT8] = 1;
        self->_builtin_align[TBTK_INT16] = 2;
        self->_builtin_align[TBTK_UINT16] = 2;
        self->_builtin_align[TBTK_INT32] = 4;
        self->_builtin_align[TBTK_UINT32] = 4;
        self->_builtin_align[TBTK_FLOAT] = 4;

        self->_builtin_size[TBTK_INVALID] = 0;
        self->_builtin_size[TBTK_VOID] = 1;
        self->_builtin_size[TBTK_INT8] = 1;
        self->_builtin_size[TBTK_UINT8] = 1;
        self->_builtin_size[TBTK_INT16] = 2;
        self->_builtin_size[TBTK_UINT16] = 2;
        self->_builtin_size[TBTK_INT32] = 4;
        self->_builtin_size[TBTK_UINT32] = 4;
        self->_builtin_size[TBTK_INT64] = 8;
        self->_builtin_size[TBTK_UINT64] = 8;
        self->_builtin_size[TBTK_FLOAT] = 4;
        self->_builtin_size[TBTK_DOUBLE] = 8;

        if (k == TTAK_X86_32)
        {
                self->_pointer_align = 4;
                self->_pointer_size = 4;

                self->_builtin_align[TBTK_INT64] = 4;
                self->_builtin_align[TBTK_UINT64] = 4;
                self->_builtin_align[TBTK_DOUBLE] = 8;
        }
        else if (k == TTAK_X86_64)
        {
                self->_pointer_align = 8;
                self->_pointer_size = 8;

                self->_builtin_align[TBTK_INT64] = 8;
                self->_builtin_align[TBTK_UINT64] = 8;
                self->_builtin_align[TBTK_FLOAT] = 8;
                self->_builtin_align[TBTK_DOUBLE] = 8;
        }
        else
                UNREACHABLE();
}

extern tree_target_architecture_kind tree_get_target_kind(const tree_target_info* self)
{
        return self->_kind;
}

extern bool tree_target_is(const tree_target_info* self, tree_target_architecture_kind k)
{
        return tree_get_target_kind(self) == k;
}

extern size_t tree_get_pointer_size(const tree_target_info* self)
{
        return self->_pointer_size;
}

extern size_t tree_get_pointer_align(const tree_target_info* self)
{
        return self->_pointer_align;
}

extern size_t tree_get_builtin_type_size(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->_builtin_size[k];
}

extern size_t tree_get_builtin_type_align(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->_builtin_align[k];
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
                if (is_union && member_size > total_size)
                        total_size = member_size;
                else
                        total_size += member_size;
        }

        return total_size;
}

extern size_t tree_get_sizeof(const tree_target_info* info, const tree_type* t)
{
        assert(t);
        t = tree_desugar_ctype(t);

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
                return int_get_u32(tree_get_constant_array_size_cvalue(t))
                        * tree_get_sizeof(info, tree_get_array_eltype(t));

        // probably function type
        return 0;
}

extern size_t tree_get_alignof_record(const tree_target_info* info, const tree_decl* d)
{
        bool is_union = tree_record_is_union(d);
        const tree_decl_scope* scope = tree_get_record_cfields(d);
        size_t max_align = 0;

        TREE_FOREACH_DECL_IN_SCOPE(scope, member)
        {
                size_t member_align = tree_get_alignof(info, tree_get_decl_type(member));
                if (member_align > max_align)
                        max_align = member_align;
        }

        return max_align;
}

extern size_t tree_get_alignof(const tree_target_info* info, const tree_type* t)
{
        assert(t);
        t = tree_desugar_ctype(t);

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
