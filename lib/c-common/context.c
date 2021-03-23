#include "scc/c-common/context.h"
#include <setjmp.h>
#include <stdarg.h>

extern void c_context_init(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        c_error_handler* error_handler,
        jmp_buf on_bad_alloc)
{
        self->errors_disabled = false;
        self->tree = tree;
        self->error_handler = error_handler;
        init_stack_alloc(&self->alloc);
        c_source_manager_init(&self->source_manager, lookup);
        c_lang_opts_init(&self->lang_opts);
}

extern void c_context_dispose(c_context* self)
{
        c_source_manager_dispose(&self->source_manager);
        drop_stack_alloc(&self->alloc);
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