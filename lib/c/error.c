#include "scc/c/error.h"
#include "scc/c/limits.h"
#include "scc/c/source.h"
#include "scc/c/context.h"
#include <stdarg.h>

extern void c_logger_init(c_logger* self, const c_context* context, FILE* log)
{
        self->enabled = true;
        self->errors = 0;
        self->warnings = 0;
        self->errors_as_warnings = false;
        self->context = context;
        self->output = log;
}

extern void c_logger_set_output(c_logger* self, FILE* output)
{
        self->output = output;
}

extern void c_logger_set_enabled(c_logger* self)
{
        self->enabled = true;
}

extern void c_logger_set_disabled(c_logger* self)
{
        self->enabled = false;
}

static const char* c_error_severity_to_str[] =
{
        "warning",
        "error",
        "fatal error",
};

extern void c_error(
        c_logger* self,
        c_error_severity severity,
        tree_location location,
        const char* description, ...)
{
        if (!self->enabled)
                return;

        c_location l;
        c_source_find_loc(&self->context->source_manager, &l, location);

        char buffer[C_MAX_LINE_LENGTH];
        va_list args;
        va_start(args, description);
        vsnprintf(buffer, C_MAX_LINE_LENGTH, description, args);

        fprintf(self->output, "%s:%d:%d: %s: %s\n",
                path_get_cfile(l.file),
                l.line,
                l.column,
                c_error_severity_to_str[severity],
                buffer);
}