#include "scc/cc/tool.h"
#include "scc/scl/args.h"

extern void scc_tool_init(scc_tool* self, const char* name, const char* path)
{
        self->path = path;
        self->name = name;
}

extern serrcode scc_tool_exec(scc_tool* self, int* code, const char** argv)
{
        int argc = 0;
        while (argv[argc])
                argc++;

        return execute(self->path, code, argc, argv);
}
