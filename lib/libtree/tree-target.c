#include "tree-target.h"

extern void tree_init_target_info(tree_target_info* self, tree_target_kind k)
{
        self->_kind = k;
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