#include "scc/scl/arg-parser.h"
#include "scc/scl/sstring.h"
#include <stdlib.h> // strtoi

extern void aparser_init(aparser* self, int argc, const char** argv)
{
        self->_argv = argv;
        self->_argc = argc;
        self->_pos = 1;
        htab_init(&self->_handlers);
}

extern void aparser_dispose(aparser* self)
{
        htab_dispose(&self->_handlers);
}

extern void aparse(aparser* self)
{
        while (self->_pos < self->_argc)
        {
                const char* arg = self->_argv[self->_pos++];
                aparser_cb* handler = htab_find(&self->_handlers, STRREF(arg));

                if (!handler)
                {
                        handler = self->_default;
                        self->_pos--;
                }

                if (handler)
                        handler->_fn(handler, self);
        }
}

extern int aparser_args_remain(const aparser* self)
{
        int n = self->_argc - self->_pos;
        return n < 0 ? 0 : n;
}

extern serrcode aparser_add_handler(aparser* self, const char* arg, aparser_cb* cb)
{
        return htab_insert(&self->_handlers, STRREF(arg), cb);
}

extern void aparser_add_default_handler(aparser* self, aparser_cb* cb)
{
        self->_default = cb;
}

extern bool aparser_has_handler(const aparser* self, const char* arg)
{
        return htab_exists(&self->_handlers, STRREF(arg));
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

extern void aparser_cb_init(aparser_cb* self, void* callback)
{
        self->_fn = callback;
}