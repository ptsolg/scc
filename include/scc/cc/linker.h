#ifndef SCC_LINKER_H
#define SCC_LINKER_H

#include "tool.h"

typedef struct _gnu_linker
{
        scc_tool tool;
        dseq files;
} gnu_linker;

extern void gnu_linker_init(gnu_linker* self, const char* path);
extern serrcode gnu_linker_add_file(gnu_linker* self, char* file);
extern serrcode gnu_linker_run(gnu_linker* self, int* code);

#endif