#include "scc.h"
#include "scc/core/common.h"
#include "scc/core/file.h"
#include <string.h>
#include <stdarg.h>
#include <scc/cc/cc.h>

extern void scc_init(scc_env* self)
{
        cc_init(&self->cc, stdout);
        self->llc_path.buf[0] = '\0';
        self->lld_path.buf[0] = '\0';
        self->cc.input.llc_path = self->llc_path.buf;
        self->cc.input.lld_path = self->lld_path.buf;
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

static void scc_add_cd_dir(scc_env* self)
{
        struct pathbuf cd;
        cwd(&cd);
        cc_add_source_dir(&self->cc, cd.buf);
}

static void scc_add_stdlibc_dir(scc_env* self, const char* exec_path)
{
        struct pathbuf dir = pathbuf_from_str(exec_path);
        join(&dir, "\\..\\libstdc\\");
        cc_add_source_dir(&self->cc, dir.buf);
}

static errcode scc_add_stdlibc_libs(scc_env* self, const char* exec_dir)
{
        struct pathbuf dir = pathbuf_from_str(exec_dir);
        join(&dir, self->cc.opts.target == CTK_X86_32 ? "win\\x86" : "win\\x64");
        cc_add_lib_dir(&self->cc, dir.buf);
        if (EC_FAILED(cc_add_lib(&self->cc, "msvcrt.lib", true)))
                return EC_ERROR;
        if (EC_FAILED(cc_add_lib(&self->cc, "legacy_stdio_definitions.lib", true)))
                return EC_ERROR;
        return EC_NO_ERROR;
}

static errcode scc_add_tm_sources(scc_env* self, const char* exec_dir)
{
        struct pathbuf libtm = pathbuf_from_str(exec_dir);
        join(&libtm, "\\..\\libtm\\");
        cc_add_source_dir(&self->cc, libtm.buf);
        struct pathbuf tmh = libtm;
        struct pathbuf tmc = libtm;
        join(&tmh, "_tm.h");
        join(&tmc, "_tm.c");
        if (!(self->cc.input.tm_decls = file_get(&self->cc.input.source_lookup, tmh.buf)))
                return EC_ERROR;
        return cc_add_source_file(&self->cc, tmc.buf, true);
}

static void scc_setup_llc_lld(scc_env* self, const char* exec_dir)
{
        strncpy(self->llc_path.buf, exec_dir, MAX_PATH_LEN);
        strncpy(self->lld_path.buf, exec_dir, MAX_PATH_LEN);
        join(&self->llc_path, "win\\llc.exe");
        join(&self->lld_path, "win\\lld-link.exe");
}

extern errcode scc_setup(scc_env* self, int argc, const char** argv)
{
        extern void scc_parse_opts(scc_env*, int, const char**);
        scc_parse_opts(self, argc, argv);

        self->cc.input.entry = self->link_stdlib ? NULL : "main";
        struct pathbuf exec_dir;
        abspath(&exec_dir, argv[0]);
        *(char*)basename(exec_dir.buf) = 0;

        scc_add_cd_dir(self);
        scc_add_stdlibc_dir(self, exec_dir.buf);
        scc_setup_llc_lld(self, exec_dir.buf);
        return (self->link_stdlib && EC_FAILED(scc_add_stdlibc_libs(self, exec_dir.buf)))
                || (self->cc.opts.ext.enable_tm && EC_FAILED(scc_add_tm_sources(self, exec_dir.buf)))
                ? EC_ERROR : EC_NO_ERROR;
}
