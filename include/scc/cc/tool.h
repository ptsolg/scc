#ifndef SCC_TOOL_H
#define SCC_TOOL_H

#include "scc/scl/common.h"
#include "scc/scl/dseq-ext.h"

typedef struct _scc_tool
{
        const char* path;
        const char* name;
} scc_tool;

extern void scc_tool_init(scc_tool* self, const char* name, const char* path);
extern serrcode scc_tool_exec(scc_tool* self, int* code, const char** argv);

#endif