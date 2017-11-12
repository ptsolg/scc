#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssaize-expr.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssa-block.h"
#include "scc/tree/tree-stmt.h"

extern ssa_value* ssaize_expr_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return ssaize_expr(self, tree_get_expr_stmt_root(stmt));
}

extern bool ssaize_decl_stmt(ssaizer* self, const tree_stmt* stmt)
{
        return ssaize_loc_decl(self, tree_get_decl_stmt_entity(stmt));
}

extern ssa_block* ssaize_compound_stmt(ssaizer* self, const tree_stmt* stmt)
{
        ssa_block* b = ssa_new_block(self->context, tree_get_empty_id(), NULL);
        ssaizer_enter_block(self, b);
        const tree_scope* s = tree_get_compound_cscope(stmt);
        TREE_FOREACH_STMT(s, it)
                if (!ssaize_stmt(self, it))
                {
                        ssaizer_exit_block(self);
                        return NULL;
                }

        ssaizer_exit_block(self);
        return b;
}

extern bool ssaize_stmt(ssaizer* self, const tree_stmt* stmt)
{
        tree_stmt_kind k = tree_get_stmt_kind(stmt);
        if (k == TSK_EXPR)
                return (bool)ssaize_expr_stmt(self, stmt);
        else if (k == TSK_DECL)
                return ssaize_decl_stmt(self, stmt);
        else if (k == TSK_COMPOUND)
                return (bool)ssaize_compound_stmt(self, stmt);

        S_UNREACHABLE(); //todo
        return false;
}