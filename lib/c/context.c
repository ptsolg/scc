#include "scc/c/context.h"
#include "scc/c/token.h"
#include "scc/c/source.h"
#include <setjmp.h>
#include <stdarg.h>

extern void c_context_init(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        c_error_handler* error_handler,
        jmp_buf on_bad_alloc)
{
        c_context_init_ex(self, tree, lookup, error_handler, on_bad_alloc, STDALLOC);
}

extern void c_context_init_ex(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        c_error_handler* error_handler,
        jmp_buf on_bad_alloc,
        allocator* alloc)
{
        self->errors_disabled = false;
        self->tree = tree;
        self->error_handler = error_handler;
        mempool_init_ex(&self->memory, NULL, on_bad_alloc, alloc);
        obstack_init_ex(&self->nodes, c_context_get_allocator(self));
        c_source_manager_init(&self->source_manager, lookup, self);
        c_lang_opts_init(&self->lang_opts);
        c_pragma_handlers_init(&self->pragma_handlers, NULL);
}

extern void c_context_dispose(c_context* self)
{
        c_source_manager_dispose(&self->source_manager);
        obstack_dispose(&self->nodes);
        mempool_dispose(&self->memory);
}

extern void c_error(
        const c_context* self,
        c_error_severity severity,
        tree_location location,
        const char* format,
        ...)
{
        if (!self->error_handler || !self->error_handler->on_error || self->errors_disabled)
                return;

        c_location source_location;
        c_source_find_loc(&self->source_manager, &source_location, location);

        char error[C_MAX_ERROR_LEN];
        va_list args;
        va_start(args, format);
        vsnprintf(error, C_MAX_ERROR_LEN, format, args);

        self->error_handler->on_error(self->error_handler, severity, source_location, error);
}