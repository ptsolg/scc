#include "scc/cc/llvm.h"
#include "scc/core/file.h"
#include "scc/core/cmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <scc/cc/llvm.h>

#define MAX_ARGC 1024

struct args
{
        int argc;
        const char* argv[MAX_ARGC];
};

static void args_init(struct args* self)
{
        *self->argv = NULL;
        self->argc = 0;
}

static void add_arg(struct args* self, const char* arg)
{
        if (self->argc + 1 >= MAX_ARGC)
        {
                assert(0 && "Too many args");
                return;
        }

        assert(arg);
        self->argv[self->argc++] = arg;
        self->argv[self->argc] = NULL;
}

void llc_init(struct llc* self, const char* llc_path)
{
        self->path = pathbuf_from_str(llc_path);
        self->num_opts = 0;
        self->input = NULL;
        self->output = NULL;
        self->is_clang = false;
}

static bool cmd_exists(const char* cmd)
{
        char buf[128];
        snprintf(buf, 128, "%s --version >NUL", cmd);
        return system(buf) == 0;
}

static bool check_program_files(struct pathbuf* pb, const char* rel)
{
        *pb = pathbuf_from_str("C:\\PROGRA~1\\");
        join(pb, rel);
        return isfile(pb->buf);
}

static bool try_detect(struct pathbuf* result, int num_opts, const char** opts)
{
        for (int i = 0; i < num_opts; i++)
        {
                if (!strchr(opts[i], '\\') && cmd_exists(opts[i]))
                {
                        *result = pathbuf_from_str(opts[i]);
                        return true;
                }
                else if (check_program_files(result, opts[i]))
                        return true;
        }
        return false;
}

bool llc_try_detect(struct llc* self)
{
        const char* opts[] = {
                "LLVM\\bin\\llc.exe",
                "llc.exe",
                // Fall back to clang if llc is not available
                "LLVM\\bin\\clang.exe",
                "clang.exe", 
        };
        llc_init(self, "");
        if (!try_detect(&self->path, ARRAY_SIZE(opts), opts))
                return false;

        self->is_clang = strstr(self->path.buf, "clang") != NULL;
        return true;
}

void llc_add_opt(struct llc* self, int opt)
{
        if (self->num_opts + 1 >= LLC_MAX_OPTS)
        {
                assert(0 && "Too many options");
                return;
        }

        self->opts[self->num_opts++] = opt;
}

void llc_set_input(struct llc* self, const char* in)
{
        self->input = in;
}

void llc_set_output(struct llc* self, const char* out)
{
        self->output = out;
}

static const char* llc_opts[] = {
        "-O=0", "-O=1", "-O=2", "-O=3",
        "-filetype=obj",
        "-filetype=asm",
        "-march=x86",
        "-march=x86-64"
};

static const char* clang_opts[] = {
        "-O0", "-O1", "-O2", "-O3",
        "-c",
        "-S",
        "-m32",
        "-m64"
};

int llc_run(struct llc* self)
{
        struct args args;
        args_init(&args);

        add_arg(&args, self->input);

        struct pathbuf clang_out;
        if (!self->output && self->is_clang)
        {
                // Force clang to emit .obj file for compatibility
                clang_out = pathbuf_from_str(self->input);
                strcpy((char*)pathext(clang_out.buf), "obj");
                add_arg(&args, "-o");
                add_arg(&args, clang_out.buf);
        }
        else if (self->output)
        {
                add_arg(&args, "-o");
                add_arg(&args, self->output);
        }

        for (int i = 0; i < self->num_opts; i++)
        {
                int opt = self->opts[i];
                assert(opt < ARRAY_SIZE(llc_opts) && "Unknown option");
                add_arg(&args, self->is_clang ? clang_opts[opt] : llc_opts[opt]);
        }
        if (self->is_clang)
                add_arg(&args, "-Wno-override-module");

        // printf("llc >> %s\n", self->path.buf);
        // for (int i = 0; i < args.argc; i++)
        //        printf("llc >> %s\n", args.argv[i]);

        return execute(self->path.buf, args.argc, args.argv);
}

void lld_init(struct lld* self, const char* lld_path)
{
        self->path = pathbuf_from_str(lld_path);
        vec_init(&self->files);
        vec_init(&self->dirs);
        self->output = NULL;
        self->entry = NULL;
        self->num_opts = 0;
}

void lld_drop(struct lld* self)
{
        VEC_FOREACH(&self->files, it, end)
                dealloc(*it);
        VEC_FOREACH(&self->dirs, it, end)
                dealloc(*it);
        vec_drop(&self->files);
        vec_drop(&self->dirs);
}

bool lld_try_detect(struct lld* self)
{
        const char* opts[] = {
                "LLVM\\bin\\lld-link.exe",
                "lld-link.exe",
        };
        lld_init(self, "");
        return try_detect(&self->path, ARRAY_SIZE(opts), opts);
}

void lld_add_opt(struct lld* self, int opt)
{
        if (self->num_opts + 1 >= LLD_MAX_OPTS)
        {
                assert(0 && "Too many options");
                return;
        }

        self->opts[self->num_opts++] = opt; 
}

void lld_add_dir(struct lld* self, const char* dir)
{
        size_t len = strlen(dir) + sizeof("/LIBPATH:\"\"");
        char* copy = alloc(len + 1);
        snprintf(copy, len, "/LIBPATH:\"%s\"", dir);
        vec_push(&self->dirs, copy);
}

void lld_add_file(struct lld* self, const char* file)
{
        char* copy = alloc(strlen(file) + 1);
        strcpy(copy, file);
        vec_push(&self->files, copy);
}

void lld_set_entry(struct lld* self, const char* entry)
{
        self->entry = entry;
}

void lld_set_output(struct lld* self, const char* out)
{
        self->output = out;
}

static const char* lld_opts[] = {
        "/DEBUG:FULL",
};

int lld_run(struct lld* self)
{
        struct args args;
        args_init(&args);

        VEC_FOREACH(&self->files, it, end)
                add_arg(&args, *it);
        VEC_FOREACH(&self->dirs, it, end)
                add_arg(&args, *it);

        char out[MAX_PATH_LEN];
        if (self->output)
        {   
                snprintf(out, ARRAY_SIZE(out), "/OUT:\"%s\"", self->output);
                add_arg(&args, out);
        }

        char entry[128];
        if (self->entry)
        {
                snprintf(entry, ARRAY_SIZE(entry), "/ENTRY:%s", self->entry);
                add_arg(&args, entry);
        }

        for (int i = 0; i < self->num_opts; i++)
        {
                int opt = self->opts[i];
                assert(opt < ARRAY_SIZE(lld_opts) && "Unknown option");
                add_arg(&args, lld_opts[opt]);
        }

        // printf("lld >> %s\n", self->path.buf);
        // for (int i = 0; i < args.argc; i++)
        //        printf("lld >> %s\n", args.argv[i]);

        return execute(self->path.buf, args.argc, args.argv);
}
