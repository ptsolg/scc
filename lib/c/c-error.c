#include "scc/c/c-error.h"
#include "scc/c/c-limits.h"
#include "scc/c/c-source.h"
#include <stdarg.h>

extern void clogger_init(
        clogger* self,
        const csource_manager* source_manager,
        const tree_context* tree,
        FILE* log)
{
        self->enabled = true;
        self->errors = 0;
        self->warnings = 0;
        self->errors_as_warnings = false;
        self->source_manager = source_manager;
        self->tree = tree;
        self->output = log;
}

extern void clogger_set_output(clogger* self, FILE* output)
{
        self->output = output;
}

extern void clogger_set_enabled(clogger* self)
{
        self->enabled = true;
}

extern void clogger_set_disabled(clogger* self)
{
        self->enabled = false;
}

static const char* cerror_severity_to_str[] =
{
        "warning",
        "error",
        "fatal error",
};

extern void cerror(
        clogger* self,
        cerror_severity severity,
        tree_location location,
        const char* description, ...)
{
        if (!self->enabled)
                return;

        clocation l;
        csource_find_loc(self->source_manager, &l, location);

        char buffer[CMAX_LINE_LENGTH];
        va_list args;
        va_start(args, description);
        vsnprintf(buffer, CMAX_LINE_LENGTH, description, args);

        fprintf(self->output, "%s:%d:%d: %s: %s\n",
                path_get_cfile(l.file),
                l.line,
                l.column,
                cerror_severity_to_str[severity],
                buffer);
}