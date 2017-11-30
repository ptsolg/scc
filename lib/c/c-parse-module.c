#include "scc/c/c-parse-module.h"
#include "scc/c/c-parse-decl.h"
#include "scc/c/c-sema.h"

extern tree_module* cparse_module(cparser* self)
{
        while (!cparser_at(self, CTK_EOF))
                if (!cparse_decl(self))
                        return NULL;

        return csema_finish_module(self->sema);
}