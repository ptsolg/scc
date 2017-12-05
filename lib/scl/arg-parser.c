#include "scc/scl/arg-parser.h"
#include "scc/scl/sstring.h"
#include <stdlib.h> // strtoi

extern void arg_handler_init(arg_handler* self,
        const char* prefix, void(*cb)(void*, aparser*), void* data)
{
        self->_prefix = prefix;
        self->_cb = cb;
        self->_data = data;
}

extern void aparser_init(aparser* self, int argc, const char** argv)
{
        self->_argv = argv;
        self->_argc = argc;
        self->_pos = 1;
}

extern void aparse(aparser* self, arg_handler* handlers, ssize nhandlers, arg_handler* def)
{
        while (self->_pos < self->_argc)
        {
                const char* arg = self->_argv[self->_pos++];

                arg_handler* handler = NULL;
                for (ssize i = 0; i < nhandlers; i++)
                {
                        const char* prefix = handlers[i]._prefix;
                        if (*prefix && strncmp(arg, prefix, strlen(prefix)) == 0)
                        {
                                handler = handlers + i;
                                break;
                        }
                }

                if (!handler)
                {
                        handler = def;
                        self->_pos--;
                }

                handler->_cb(handler->_data, self);
        }
}

extern int aparser_args_remain(const aparser* self)
{
        int n = self->_argc - self->_pos;
        return n < 0 ? 0 : n;
}

extern const char* aparser_get_string(aparser* self)
{
        return aparser_args_remain(self)
                ? self->_argv[self->_pos++]
                : NULL;
}

extern serrcode aparser_get_int(aparser* self, int* pint)
{
        const char* num = aparser_get_string(self);
        if (!num)
                return S_ERROR;

        *pint = atoi(num);
        return S_NO_ERROR;
}
