#include "scc/cc/linker.h"

extern void gnu_linker_init(gnu_linker* self, const char* path)
{
        scc_tool_init(&self->tool, "ld", path);
        dseq_init_ptr(&self->files);
}

extern serrcode gnu_linker_add_file(gnu_linker* self, char* file)
{
        return dseq_append_ptr(&self->files, file);
}

extern serrcode gnu_linker_run(gnu_linker* self, int* code)
{
        dseq argv;
        dseq_init_ptr(&argv);
        for (void** file = dseq_begin_ptr(&self->files);
                file != dseq_end_ptr(&self->files); file++)
        {
                if (S_FAILED(dseq_append_ptr(&argv, *file)))
                {
                        dseq_dispose(&argv);
                        return S_ERROR;
                }
        }

        char* opts[] = 
        {
                "-m",
                "i386pe",
                NULL,
        };

        for (int i = 0; i < S_ARRAY_SIZE(opts); i++)
                if (S_FAILED(dseq_append_ptr(&argv, opts[i])))
                {
                        dseq_dispose(&argv);
                        return S_ERROR;
                }

        return scc_tool_exec(&self->tool, code, (char**)dseq_begin_ptr(&argv));
}