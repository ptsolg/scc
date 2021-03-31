#include "scc.h"
#include "scc/cc/cc.h"
#include "scc/core/cmd.h"
#include "scc/core/file.h"

#include <string.h>

static void scc_missing_argument(scc_env* self, const char* arg)
{
        scc_error(self, "argument to '%s' is missing", arg);
}

extern FILE* scc_open_file(scc_env* self, const char* file, const char* mode)
{
        FILE* f = fopen(file, mode);
        if (f)
                return f;

        struct pathbuf path;
        cwd(&path);
        join(&path, file);
        return fopen(path.buf, mode);
}

static void scc_S(scc_env* self, cmd_parser* parser)
{
        self->mode = SRM_ASSEMBLE;
        self->cc.output.kind = COK_ASM;
}

static void scc_c(scc_env* self, cmd_parser* parser)
{
        self->mode = SRM_COMPILE;
        self->cc.output.kind = COK_OBJ;
}

static void scc_o(scc_env* self, cmd_parser* parser)
{
        const char* file = cmd_parser_get_string(parser);
        if (!file)
        {
                scc_missing_argument(self, "-o");
                return;
        }
        cc_set_output_file(&self->cc, file);
}

static void scc_nostdlib(scc_env* self, cmd_parser* parser)
{
        self->link_stdlib = false;
}

static void scc_log(scc_env* self, cmd_parser* parser)
{
        const char* file = cmd_parser_get_string(parser);
        if (!file)
        {
                scc_missing_argument(self, "-log");
                return;
        }

        FILE* log = scc_open_file(self, file, "w");
        if (!log)
                return;

        self->cc.output.message = log;
}

static void scc_help(scc_env* self, cmd_parser* parser)
{
}

static void scc_fsyntax_only(scc_env* self, cmd_parser* parser)
{
        self->cc.output.kind = COK_NONE;
        self->mode = SRM_OTHER;
}

static void scc_dump_tokens(scc_env* self, cmd_parser* parser)
{
        self->cc.output.kind = COK_LEXEMES;
        self->mode = SRM_OTHER;
}

static void scc_dump_tree(scc_env* self, cmd_parser* parser)
{
        self->cc.output.kind = COK_C;
        self->mode = SRM_OTHER;
}

static void scc_fprint_eval_result(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.cprint.print_eval_result = true;
}

static void scc_fprint_expr_value(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.cprint.print_expr_value = true;
}

static void scc_fprint_expr_type(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.cprint.print_expr_type = true;
}

static void scc_fprint_impl_casts(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.cprint.print_impl_casts = true;
}

static void scc_fforce_brackets(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.cprint.force_brackets = true;
}

static void scc_fdce(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.optimization.eliminate_dead_code = true;
}

static void scc_fcf(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.optimization.fold_constants = true;
}

static void scc_ftm(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.ext.enable_tm = true;
}

static void scc_emit_ssa(scc_env* self, cmd_parser* parser)
{
        if (self->mode != SRM_ASSEMBLE)
        {
                scc_error(self, "-emit-ssa cannot be used when linking");
                return;
        }

        self->cc.output.kind = COK_SSA;
}

static void scc_emit_llvm(scc_env* self, cmd_parser* parser)
{
        if (self->mode != SRM_ASSEMBLE)
        {
                scc_error(self, "-emit-llvm cannot be used when linking");
                return;
        }

        self->cc.output.kind = COK_LLVM_IR;
}

static void scc_I(scc_env* self, cmd_parser* parser)
{
        const char* dir = cmd_parser_get_string(parser);
        if (!dir)
        {
                scc_missing_argument(self, "-I");
                return;
        }

        cc_add_source_dir(&self->cc, dir);
}

static void scc_l(scc_env* self, cmd_parser* parser)
{
        const char* lib = cmd_parser_get_string(parser);
        if (!lib)
        {
                scc_missing_argument(self, "-l");
                return;
        }

        cc_add_lib(&self->cc, lib);
}

static void scc_L(scc_env* self, cmd_parser* parser)
{
        const char* dir = cmd_parser_get_string(parser);
        if (!dir)
        {
                scc_missing_argument(self, "-L");
                return;
        }

        cc_add_lib_dir(&self->cc, dir);
}

static void scc_m32(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.target = CTK_X86_32;
}

static void scc_m64(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.target = CTK_X86_64;
}

static void scc_fpa(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.optimization.promote_allocas = true;
}

static void scc_finline(scc_env* self, cmd_parser* parser)
{
        UNREACHABLE();
}

static void scc_O3(scc_env* self, cmd_parser* parser)
{
        self->cc.opts.optimization.eliminate_dead_code = true;
        self->cc.opts.optimization.promote_allocas = true;
        self->cc.opts.optimization.fold_constants = true;
        self->cc.opts.optimization.level = 3;
}

static void scc_source_file(scc_env* self, cmd_parser* parser)
{
        const char* file = cmd_parser_get_string(parser);
        if (!file || EC_FAILED(cc_add_source_file(&self->cc, file, false)))
                return;

        struct pathbuf dir = pathbuf_from_str(file);
        basename(dir.buf);
        cc_add_source_dir(&self->cc, dir.buf);
}

extern errcode scc_parse_opts(scc_env* self, int argc, const char** argv)
{
        cmd_handler handlers[] =
        {
                CMD_HANDLER_INIT("-S", &scc_S, self),
                CMD_HANDLER_INIT("-c", &scc_c, self),
                CMD_HANDLER_INIT("-o", &scc_o, self),
                CMD_HANDLER_INIT("-nostdlib", &scc_nostdlib, self),
                CMD_HANDLER_INIT("-log", &scc_log, self),
                CMD_HANDLER_INIT("-fsyntax-only", &scc_fsyntax_only, self),
                CMD_HANDLER_INIT("-dump-tokens", &scc_dump_tokens, self),
                CMD_HANDLER_INIT("-dump-tree", &scc_dump_tree, self),
                CMD_HANDLER_INIT("-fprint-eval-result", &scc_fprint_eval_result, self),
                CMD_HANDLER_INIT("-fprint-expr-value", &scc_fprint_expr_value, self),
                CMD_HANDLER_INIT("-fprint-expr-type", &scc_fprint_expr_type, self),
                CMD_HANDLER_INIT("-fprint-impl-casts", &scc_fprint_impl_casts, self),
                CMD_HANDLER_INIT("-fforce-brackets", &scc_fforce_brackets, self),
                CMD_HANDLER_INIT("-fdce", &scc_fdce, self),
                CMD_HANDLER_INIT("-fcf", &scc_fcf, self),
                CMD_HANDLER_INIT("-fpa", &scc_fpa, self),
                CMD_HANDLER_INIT("-ftm", &scc_ftm, self),
                CMD_HANDLER_INIT("-finline", &scc_finline, self),
                CMD_HANDLER_INIT("-emit-ssa", &scc_emit_ssa, self),
                CMD_HANDLER_INIT("-emit-llvm", &scc_emit_llvm, self),
                CMD_HANDLER_INIT("-I", &scc_I, self),
                CMD_HANDLER_INIT("-l", &scc_l, self),
                CMD_HANDLER_INIT("-L", &scc_L, self),
                CMD_HANDLER_INIT("-m32", &scc_m32, self),
                CMD_HANDLER_INIT("-m64", &scc_m64, self),
                CMD_HANDLER_INIT("-O3", &scc_O3, self),
        };

        cmd_handler source = CMD_HANDLER_INIT("", &scc_source_file, self);

        cmd_parser parser;
        cmd_parser_init(&parser, argc, argv);
        cmd_parser_run(&parser, handlers, ARRAY_SIZE(handlers), &source);
        return EC_NO_ERROR;
}
