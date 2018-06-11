#ifndef C_PRAGMA_H
#define C_PRAGMA_H

#include "scc/core/common.h"

typedef struct _c_pragma_handlers
{
        errcode(*on_link)(void*, const char*);
        void* data;
} c_pragma_handlers;

extern void c_pragma_handlers_init(c_pragma_handlers* self, void* data);

extern errcode c_pragma_handles_on_link(c_pragma_handlers* self, const char* lib);

#endif