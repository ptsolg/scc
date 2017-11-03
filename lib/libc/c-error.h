#ifndef CERRCODE_H
#define CERRCODE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <libscl/error.h>
#include "c-source.h"

typedef struct _cerror_manager
{
        int errors;
        int warnings;
        bool errors_as_warnings;
        bool enabled;
        const csource_manager* source_manager;
        FILE* err;
} cerror_manager;

extern void cerror_manager_init(
        cerror_manager* self, const csource_manager* source_manager, FILE* err);

extern void cerrors_set_enabled(cerror_manager* self);
extern void cerrors_set_disabled(cerror_manager* self);

typedef enum
{
        CES_WARNING,
        CES_ERROR,
        CES_FATAL_ERROR,
} cerror_severity;

extern void cerror(
        cerror_manager* self,
        cerror_severity severity,
        tree_location loc,
        const char* description, ...);

#ifdef __cplusplus
}
#endif

#endif // !CERRCODE_H
