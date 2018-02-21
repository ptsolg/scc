#include "scc/c/c-parse-module.h"
#include "scc/c/c-parse-decl.h"
#include "scc/c/c-sema.h"

extern tree_module* c_parse_module(c_parser* self)
{
        while (!c_parser_at(self, CTK_EOF))
                if (!c_parse_decl(self))
                        return NULL;

        return self->sema->module;
}