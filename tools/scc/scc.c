#include "scc.h"
#include <string.h>
#include <stdarg.h>
#include <scc/cc/cc.h>

extern void scc_init(scc_env* self)
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

extern void scc_dispose(scc_env* self)
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

static errcode scc_add_cd_dir(scc_env* self)
{
        char cd[MAX_PATH_LEN + 1];
        if (EC_FAILED(path_get_cd(cd)))
                return EC_ERROR;
        return cc_add_source_dir(&self->cc, cd);
}

static errcode scc_add_stdlibc_dir(scc_env* self, const char* exec_path)
{
        char dir[MAX_PATH_LEN];
        strncpy(dir, exec_path, MAX_PATH_LEN);
        path_goto_parent_dir(dir);
        if (EC_FAILED(path_join(dir, "libstdc")))
                return EC_ERROR;

        return cc_add_source_dir(&self->cc, dir);
}

static errcode scc_add_stdlibc_libs(scc_env* self, const char* exec_dir)
{
        char dir[MAX_PATH_LEN];
        strncpy(dir, exec_dir, MAX_PATH_LEN);

#if OS_WIN
        if (EC_FAILED(path_join(dir,
                self->cc.opts.target == CTK_X86_32 ? "win\\x86" : "win\\x64")))
        {
                return EC_ERROR;
        }
        if (EC_FAILED(cc_add_lib_dir(&self->cc, dir)))
                return EC_ERROR;
        if (EC_FAILED(cc_add_lib(&self->cc, "msvcrt.lib")))
                return EC_ERROR;
        if (EC_FAILED(cc_add_lib(&self->cc, "legacy_stdio_definitions.lib")))
                return EC_ERROR;
#else
        // todo
#endif
        return EC_NO_ERROR;
}

static errcode scc_add_tm_sources(scc_env* self, const char* exec_dir)
{
        char path[MAX_PATH_LEN];
        strncpy(path, exec_dir, MAX_PATH_LEN);
        path_goto_parent_dir(path);

        if (EC_FAILED(path_join(path, "libtm")))
                return EC_ERROR;
        if (EC_FAILED(cc_add_source_dir(&self->cc, path)))
                return EC_ERROR;
        if (EC_FAILED(path_join(path, "_tm.h")))
                return EC_ERROR;
        if (!(self->cc.input.tm_decls = file_get(&self->cc.input.source_lookup, path)))
                return EC_ERROR;
        if (EC_FAILED(path_change_ext(path, "c")))
                return EC_ERROR;
        
        return cc_add_source_file(&self->cc, path, true);
}

static errcode scc_setup_llc_lld(scc_env* self, const char* exec_dir)
{
        char dir[MAX_PATH_LEN];
        strncpy(dir, exec_dir, MAX_PATH_LEN);
        strcpy(self->llc_path, dir);
        strcpy(self->lld_path, dir);

#if OS_WIN
        if (EC_FAILED(path_join(self->llc_path, "win\\llc.exe")))
                return EC_ERROR;
        if (EC_FAILED(path_join(self->lld_path, "win\\lld-link.exe")))
                return EC_ERROR;
#else
        if (EC_FAILED(path_join(self->llc_path, "osx/llc")))
                return EC_ERROR;
        if (EC_FAILED(path_join(self->lld_path, "osx/ld")))
                return EC_ERROR;
#endif
        return EC_NO_ERROR;
}

extern errcode scc_setup(scc_env* self, int argc, const char** argv)
{
        extern errcode scc_parse_opts(scc_env*, int, const char**);

        if (EC_FAILED(scc_parse_opts(self, argc, argv)))
                return EC_ERROR;

#if OS_WIN
        self->cc.input.entry = self->link_stdlib ? NULL : "main";
#elif OS_OSX
        self->cc.input.entry = "_main";
#else
#error
#endif
        char exec_dir[MAX_PATH_LEN];
        if (EC_FAILED(path_get_abs(exec_dir, argv[0])))
                return EC_ERROR;
        path_strip_file(exec_dir);


        return EC_FAILED(scc_add_cd_dir(self))
                || EC_FAILED(scc_add_stdlibc_dir(self, exec_dir))
                || (self->link_stdlib && EC_FAILED(scc_add_stdlibc_libs(self, exec_dir)))
                || EC_FAILED(scc_setup_llc_lld(self, exec_dir))
                || (self->cc.opts.ext.enable_tm && EC_FAILED(scc_add_tm_sources(self, exec_dir)))
                ? EC_ERROR : EC_NO_ERROR;
}