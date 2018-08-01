#include "scc/c/c-pragma.h"

static errcode on_link(void* d, const char* s)
{
        return EC_NO_ERROR;
}

extern void c_pragma_handlers_init(c_pragma_handlers* self, void* data)
{
        self->data = data;
        self->on_link = on_link;
}

extern errcode c_pragma_handlers_on_link(c_pragma_handlers* self, const char* lib)
{
        return self->on_link(self->data, lib);
}