#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/tree/tree-decl.h"

extern bool ssaize_var_decl(ssaizer* self, const tree_decl* decl)
{
        ssa_value* alloca = ssa_build_alloca(&self->builder, tree_get_decl_type(decl));
        if (!alloca)
                return false;

        ssaizer_set_def(self, decl, alloca);
        tree_expr* init = tree_get_var_init(decl);
        ssa_value* init_value = NULL;
        if (init && !(init_value = ssaize_expr(self, init)))
                return false;

        if (init && !ssa_build_store(&self->builder, init_value, alloca))
                return false;

        return true;
}

extern bool ssaize_decl_group(ssaizer* self, const tree_decl* decl)
{
        TREE_FOREACH_DECL_IN_GROUP(decl, it)
                if (!ssaize_decl(self, *it))
                        return false;
        return true;
}

extern bool ssaize_decl(ssaizer* self, const tree_decl* decl)
{
        if (tree_decl_is(decl, TDK_VAR))
                return ssaize_var_decl(self, decl);
        else if (tree_decl_is(decl, TDK_GROUP))
                return ssaize_decl_group(self, decl);

        // just ignore unknown decl
        return true;
}