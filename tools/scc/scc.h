#ifndef SCC_H
#define SCC_H

#include "scc/cc/cc.h"

typedef enum
{
        SRM_ASSEMBLE,
        SRM_COMPILE,
        SRM_LINK,
} scc_run_mode;

typedef struct
{
        cc_instance cc;
        bool link_stdlib;
        char llc_path[S_MAX_PATH_LEN];
        char lld_path[S_MAX_PATH_LEN];
        scc_run_mode mode;
} scc_env;

extern void scc_env_init(scc_env* self);
extern void scc_env_dispose(scc_env* self);
extern void scc_error(scc_env* self, const char* format, ...);
extern serrcode scc_env_setup(scc_env* self, int argc, const char** argv);

#endif