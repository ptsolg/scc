#ifndef CERRCODE_H
#define CERRCODE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "scc/tree/tree-common.h"

typedef struct _csource_manager csource_manager;
typedef struct _tree_context tree_context;

typedef struct _clogger
{
        int errors;
        int warnings;
        bool errors_as_warnings;
        bool enabled;
        const csource_manager* source_manager;
        const tree_context* tree;
        FILE* output;
} clogger;

extern void clogger_init(
        clogger* self,
        const csource_manager* source_manager,
        const tree_context* tree,
        FILE* log);

extern void clogger_set_output(clogger* self, FILE* output);
extern void clogger_set_enabled(clogger* self);
extern void clogger_set_disabled(clogger* self);

typedef enum
{
        CES_WARNING,
        CES_ERROR,
        CES_FATAL_ERROR,
} cerror_severity;

extern void cerror(
        clogger* self,
        cerror_severity severity,
        tree_location location,
        const char* description,
        ...);

#ifdef __cplusplus
}
#endif

#endif // !CERRCODE_H
