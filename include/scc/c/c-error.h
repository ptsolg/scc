#ifndef C_ERROR_H
#define C_ERROR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "scc/tree/tree-common.h"

typedef struct _c_source_manager c_source_manager;
typedef struct _tree_context tree_context;

typedef struct _c_logger
{
        int errors;
        int warnings;
        bool errors_as_warnings;
        bool enabled;
        const c_source_manager* source_manager;
        const tree_context* tree;
        FILE* output;
} c_logger;

extern void c_logger_init(
        c_logger* self,
        const c_source_manager* source_manager,
        const tree_context* tree,
        FILE* log);

extern void c_logger_set_output(c_logger* self, FILE* output);
extern void c_logger_set_enabled(c_logger* self);
extern void c_logger_set_disabled(c_logger* self);

typedef enum
{
        CES_WARNING,
        CES_ERROR,
        CES_FATAL_ERROR,
} c_error_severity;

extern void c_error(
        c_logger* self,
        c_error_severity severity,
        tree_location location,
        const char* description,
        ...);

#ifdef __cplusplus
}
#endif

#endif
