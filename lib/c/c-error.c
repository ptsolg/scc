#include "scc/c/c-error.h"
#include "scc/c/c-limits.h"
#include <stdarg.h>

extern void cerror_manager_init(
        cerror_manager* self, const csource_manager* source_manager, FILE* log)
{
        self->enabled = true;
        self->errors = 0;
        self->warnings = 0;
        self->errors_as_warnings = false;
        self->source_manager = source_manager;
        self->log = log;
}

extern void cerror_manager_set_output(cerror_manager* self, FILE* log)
{
        self->log = log;
}

extern void cerror_manager_set_enabled(cerror_manager* self)
{
        self->enabled = true;
}

extern void cerror_manager_set_disabled(cerror_manager* self)
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
        cerror_manager* self,
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

        fprintf(self->log, "%s:%d:%d: %s: %s\n",
                path_get_cfile(l.file),
                l.line,
                l.column,
                cerror_severity_to_str[severity],
                buffer);
}