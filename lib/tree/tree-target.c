#include "scc/tree/tree-target.h"

extern void tree_init_target_info(tree_target_info* self, tree_target_kind k)
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

        if (k == TTARGET_X32)
        {
                self->_pointer_align = 4;
                self->_pointer_size = 4;

                self->_builtin_align[TBTK_INT64] = 4;
                self->_builtin_align[TBTK_UINT64] = 4;
                self->_builtin_align[TBTK_DOUBLE] = 8;
        }
        else if (k == TTARGET_X64)
        {
                self->_pointer_align = 8;
                self->_pointer_size = 8;

                self->_builtin_align[TBTK_INT64] = 8;
                self->_builtin_align[TBTK_UINT64] = 8;
                self->_builtin_align[TBTK_FLOAT] = 8;
                self->_builtin_align[TBTK_DOUBLE] = 8;
        }
        else
                S_UNREACHABLE();
}

extern tree_target_kind tree_get_target_kind(const tree_target_info* self)
{
        return self->_kind;
}

extern bool tree_target_is(const tree_target_info* self, tree_target_kind k)
{
        return tree_get_target_kind(self) == k;
}

extern ssize tree_get_pointer_size(const tree_target_info* self)
{
        return self->_pointer_size;
}

extern ssize tree_get_pointer_align(const tree_target_info* self)
{
        return self->_pointer_align;
}

extern ssize tree_get_builtin_type_size(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->_builtin_size[k];
}

extern ssize tree_get_builtin_type_align(const tree_target_info* self, tree_builtin_type_kind k)
{
        return self->_builtin_align[k];
}