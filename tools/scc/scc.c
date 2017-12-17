#include "scc.h"
#include <string.h>

extern void scc_env_init(scc_env* self)
{
        cc_init(&self->cc, stdout);
        self->llc_path[0] = '\0';
        self->lld_path[0] = '\0';
        self->cc.opts.llc_path = self->llc_path;
        self->cc.opts.lld_path = self->lld_path;
}

extern void scc_env_dispose(scc_env* self)
{
        cc_dispose(&self->cc);
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
#error
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
#error
#endif
        return S_NO_ERROR;
}

extern serrcode scc_env_setup(scc_env* self, int argc, const char** argv)
{
        return 0
                || S_FAILED(cc_parse_opts(&self->cc, argc, argv))
                || S_FAILED(scc_env_add_cd_dir(self))
                || S_FAILED(scc_env_add_stdlibc_dir(self, argv[0]))
                || S_FAILED(scc_env_add_stdlibc_libs(self, argv[0]))
                || S_FAILED(scc_env_setup_llc_lld(self, argv[0]))
                ? S_ERROR : S_NO_ERROR;
}