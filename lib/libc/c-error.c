#include "c-error.h"
#include <stdarg.h>

extern void cerror_manager_init(
        cerror_manager* self, const csource_manager* source_manager, FILE* err)
{
        self->enabled            = true;
        self->errors             = 0;
        self->warnings           = 0;
        self->errors_as_warnings = false;
        self->source_manager     = source_manager;
        self->err                = err;
}

extern void cerrors_set_enabled(cerror_manager* self)
{
        self->enabled = true;
}

extern void cerrors_set_disabled(cerror_manager* self)
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
        tree_location   loc,
        const char*     description, ...)
{
        if (!self->enabled)
                return;

        clocation l;
        csource_find_loc(self->source_manager, &l, loc);

        char buffer[1024];
        va_list args;
        va_start(args, description);
        vsprintf(buffer, description, args);

        fprintf(self->err, "%s:%d:%d: %s: %s",
                path_get_cfile(l.file),
                l.line,
                l.column,
                cerror_severity_to_str[severity],
                buffer);
}