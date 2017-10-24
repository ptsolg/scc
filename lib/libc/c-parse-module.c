#include "c-parse-module.h"
#include "c-parse-decl.h"
#include "c-prog.h"

extern tree_module* cparse_module(cparser* self)
{
        while (!cparser_at(self, CTK_EOF))
                if (!cparse_decl(self))
                        return NULL;

        return cparser_act_on_finish_module(self->prog);
}