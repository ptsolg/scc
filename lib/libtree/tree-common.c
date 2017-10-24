#include "tree-common.h"
#include "tree-context.h"

extern tree_xlocation tree_init_xloc(tree_location begin, tree_location end)
{
        tree_xlocation xloc = 0;
        xloc = tree_set_xloc_begin(xloc, begin);
        return tree_set_xloc_end(xloc, end);
}

extern tree_id tree_get_empty_id()
{
        return STRREF("");
}

extern bool tree_id_is_empty(tree_id id)
{
        return id == tree_get_empty_id();
}

extern void tree_init_platform_info(tree_platform_info* self, tree_platform_kind k)
{
        self->_kind = k;
}