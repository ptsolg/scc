#ifndef SCC_H
#define SCC_H

#include "scc/cc/cc.h"
#include "scc/core/file.h"

typedef enum
{
        // -dump-tree -fsyntax-only ...
        SRM_OTHER,
        // -S
        SRM_ASSEMBLE,
        // -c
        SRM_COMPILE,

        SRM_LINK,
} scc_run_mode;

typedef struct
{
        cc_instance cc;
        bool link_stdlib;
        scc_run_mode mode;
} scc_env;

extern void scc_init(scc_env* self);
extern void scc_dispose(scc_env* self);
extern void scc_error(scc_env* self, const char* format, ...);
extern errcode scc_setup(scc_env* self, int argc, const char** argv);

#endif
