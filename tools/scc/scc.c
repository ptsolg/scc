#include "scc.h"
#include <string.h>
#include <stdarg.h>
#include <scc/cc/cc.h>

extern void scc_env_init(scc_env* self)
{
        cc_init(&self->cc, stdout);
        self->llc_path[0] = '\0';
        self->lld_path[0] = '\0';
        self->cc.input.llc_path = self->llc_path;
        self->cc.input.lld_path = self->lld_path;
        self->cc.opts.name = "scc";
        self->link_stdlib = true;
        self->mode = SRM_LINK;
}

extern void scc_env_dispose(scc_env* self)
{
        cc_dispose(&self->cc);
}

extern void scc_error(scc_env* self, const char* format, ...)
{
        printf("scc: error: ");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        printf("\n");
}

static serrcode scc_env_add_cd_dir(scc_env* self)
{
        char cd[S_MAX_PATH_LEN + 1];
        if (S_FAILED(path_get_cd(cd)))
                return S_ERROR;
        return cc_add_source_dir(&self->cc, cd);
}

static serrcode scc_env_add_stdlibc_dir(scc_env* self, const char* argv0)
{
        char dir[S_MAX_PATH_LEN];
        strncpy(dir, argv0, S_MAX_PATH_LEN);
        path_goto_parent_dir(dir);
        if (S_FAILED(path_join(dir, "libstdc")))
                return S_ERROR;

        return cc_add_source_dir(&self->cc, dir);
}

static serrcode scc_env_add_stdlibc_libs(scc_env* self, const char* argv0)
{
        char dir[S_MAX_PATH_LEN];
        strncpy(dir, argv0, S_MAX_PATH_LEN);
        path_strip_file(dir);

#if S_WIN
        if (S_FAILED(path_join(dir,
                self->cc.opts.target == CTK_X86_32 ? "win\\x86" : "win\\x64")))
        {
                return S_ERROR;
        }
        if (S_FAILED(cc_add_lib_dir(&self->cc, dir)))
                return S_ERROR;
        if (S_FAILED(cc_add_lib(&self->cc, "msvcrt.lib")))
                return S_ERROR;
        if (S_FAILED(cc_add_lib(&self->cc, "legacy_stdio_definitions.lib")))
                return S_ERROR;
#else
        // todo
#endif
        return S_NO_ERROR;
}

static serrcode scc_env_setup_llc_lld(scc_env* self, const char* argv0)
{
        char dir[S_MAX_PATH_LEN];
        strncpy(dir, argv0, S_MAX_PATH_LEN);
        path_strip_file(dir);
        strcpy(self->llc_path, dir);
        strcpy(self->lld_path, dir);

#if S_WIN
        if (S_FAILED(path_join(self->llc_path, "win\\llc.exe")))
                return S_ERROR;
        if (S_FAILED(path_join(self->lld_path, "win\\lld-link.exe")))
                return S_ERROR;
#else
        if (S_FAILED(path_join(self->llc_path, "osx/llc")))
                return S_ERROR;
        if (S_FAILED(path_join(self->lld_path, "osx/ld")))
                return S_ERROR;
#endif
        return S_NO_ERROR;
}

extern serrcode scc_env_setup(scc_env* self, int argc, const char** argv)
{
        extern serrcode scc_parse_opts(scc_env*, int, const char**);
        if (S_FAILED(scc_parse_opts(self, argc, argv)))
                return S_ERROR;

#if S_WIN
        self->cc.input.entry = self->link_stdlib ? NULL : "main";
#elif S_OSX
        self->cc.input.entry = "_main";
#else
#error
#endif
        return S_FAILED(scc_env_add_cd_dir(self))
                || S_FAILED(scc_env_add_stdlibc_dir(self, argv[0]))
                || (self->link_stdlib && S_FAILED(scc_env_add_stdlibc_libs(self, argv[0])))
                || S_FAILED(scc_env_setup_llc_lld(self, argv[0]))
                ? S_ERROR : S_NO_ERROR;
}