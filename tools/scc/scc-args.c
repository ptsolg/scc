#include "scc.h"
#include "scc/core/args.h"

static void scc_missing_argument(scc_env* self, const char* arg)
{
        scc_error(self, "argument to '%s' is missing", arg);
}

extern FILE* scc_open_file(scc_env* self, const char* file, const char* mode)
{
        FILE* f = fopen(file, mode);
        if (f)
                return f;

        char path[S_MAX_PATH_LEN + 1];
        if (S_FAILED(path_get_cd(path)
                || S_FAILED(path_join(path, file))) || !(f = fopen(path, mode)))
        {
                return NULL;
        }

        return f;
}

typedef enum
{
        SAK_S,
        SAK_c,
        SAK_o,
        SAK_nostdlib,
        SAK_log,
        SAK_help,
        SAK_fsyntax_only,
        SAK_dump_tokens,
        SAK_dump_tree,
        SAK_fprint_eval_result,
        SAK_fprint_expr_value,
        SAK_fprint_expr_type,
        SAK_fprint_impl_casts,
        SAK_fforce_brackets,
        SAK_fdce,
        SAK_fcf,
        SAK_emit_ssa,
        SAK_emit_llvm,
        SAK_I,
        SAK_l,
        SAK_L,
        SAK_m32,
        SAK_m64,
} scc_arg_kind;

static void scc_S(scc_env* self, aparser* parser)
{
        self->mode = SRM_ASSEMBLE;
        self->cc.output.kind = COK_ASM;
}

static void scc_c(scc_env* self, aparser* parser)
{
        self->mode = SRM_COMPILE;
        self->cc.output.kind = COK_OBJ;
}

static void scc_o(scc_env* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
        {
                scc_missing_argument(self, "-o");
                return;
        }
        cc_set_output_file(&self->cc, file);
}

static void scc_nostdlib(scc_env* self, aparser* parser)
{
        self->link_stdlib = false;
}

static void scc_log(scc_env* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
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

static void scc_help(scc_env* self, aparser* parser)
{
}

static void scc_fsyntax_only(scc_env* self, aparser* parser)
{
        self->cc.output.kind = COK_NONE;
        self->mode = SRM_OTHER;
}

static void scc_dump_tokens(scc_env* self, aparser* parser)
{
        self->cc.output.kind = COK_LEXEMES;
        self->mode = SRM_OTHER;
}

static void scc_dump_tree(scc_env* self, aparser* parser)
{
        self->cc.output.kind = COK_C;
        self->mode = SRM_OTHER;
}

static void scc_fprint_eval_result(scc_env* self, aparser* parser)
{
        self->cc.opts.cprint.print_eval_result = true;
}

static void scc_fprint_expr_value(scc_env* self, aparser* parser)
{
        self->cc.opts.cprint.print_expr_value = true;
}

static void scc_fprint_expr_type(scc_env* self, aparser* parser)
{
        self->cc.opts.cprint.print_expr_type = true;
}

static void scc_fprint_impl_casts(scc_env* self, aparser* parser)
{
        self->cc.opts.cprint.print_impl_casts = true;
}

static void scc_fforce_brackets(scc_env* self, aparser* parser)
{
        self->cc.opts.cprint.force_brackets = true;
}

static void scc_fdce(scc_env* self, aparser* parser)
{
        self->cc.opts.optimization.eliminate_dead_code = true;
}

static void scc_fcf(scc_env* self, aparser* parser)
{
        self->cc.opts.optimization.fold_constants = true;
}

static void scc_emit_ssa(scc_env* self, aparser* parser)
{
        if (self->mode != SRM_ASSEMBLE)
        {
                scc_error(self, "-emit-ssa cannot be used when linking");
                return;
        }

        self->cc.output.kind = COK_SSA;
}

static void scc_emit_llvm(scc_env* self, aparser* parser)
{
        if (self->mode != SRM_ASSEMBLE)
        {
                scc_error(self, "-emit-llvm cannot be used when linking");
                return;
        }

        self->cc.output.kind = COK_LLVM_IR;
}

static void scc_I(scc_env* self, aparser* parser)
{
        const char* dir = aparser_get_string(parser);
        if (!dir)
        {
                scc_missing_argument(self, "-I");
                return;
        }

        cc_add_source_dir(&self->cc, dir);
}

static void scc_l(scc_env* self, aparser* parser)
{
        const char* lib = aparser_get_string(parser);
        if (!lib)
        {
                scc_missing_argument(self, "-l");
                return;
        }

        cc_add_lib(&self->cc, lib);
}

static void scc_L(scc_env* self, aparser* parser)
{
        const char* dir = aparser_get_string(parser);
        if (!dir)
        {
                scc_missing_argument(self, "-L");
                return;
        }

        cc_add_lib_dir(&self->cc, dir);
}

static void scc_m32(scc_env* self, aparser* parser)
{
        self->cc.opts.target = CTK_X86_32;
}

static void scc_m64(scc_env* self, aparser* parser)
{
        self->cc.opts.target = CTK_X86_64;
}

static void scc_source_file(scc_env* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
                return;

        cc_add_source_file(&self->cc, file);
}

extern errcode scc_parse_opts(scc_env* self, int argc, const char** argv)
{
        arg_handler handlers[] =
        {
                ARG_HANDLER_INIT("-S", &scc_S, self),
                ARG_HANDLER_INIT("-c", &scc_c, self),
                ARG_HANDLER_INIT("-o", &scc_o, self),
                ARG_HANDLER_INIT("-nostdlib", &scc_nostdlib, self),
                ARG_HANDLER_INIT("-log", &scc_log, self),
                ARG_HANDLER_INIT("-fsyntax-only", &scc_fsyntax_only, self),
                ARG_HANDLER_INIT("-dump-tokens", &scc_dump_tokens, self),
                ARG_HANDLER_INIT("-dump-tree", &scc_dump_tree, self),
                ARG_HANDLER_INIT("-fprint-eval-result", &scc_fprint_eval_result, self),
                ARG_HANDLER_INIT("-fprint-expr-value", &scc_fprint_expr_value, self),
                ARG_HANDLER_INIT("-fprint-expr-type", &scc_fprint_expr_type, self),
                ARG_HANDLER_INIT("-fprint-impl-casts", &scc_fprint_impl_casts, self),
                ARG_HANDLER_INIT("-fforce-brackets", &scc_fforce_brackets, self),
                ARG_HANDLER_INIT("-fdce", &scc_fdce, self),
                ARG_HANDLER_INIT("-fcf", &scc_fcf, self),
                ARG_HANDLER_INIT("-emit-ssa", &scc_emit_ssa, self),
                ARG_HANDLER_INIT("-emit-llvm", &scc_emit_llvm, self),
                ARG_HANDLER_INIT("-I", &scc_I, self),
                ARG_HANDLER_INIT("-l", &scc_l, self),
                ARG_HANDLER_INIT("-L", &scc_L, self),
                ARG_HANDLER_INIT("-m32", &scc_m32, self),
                ARG_HANDLER_INIT("-m64", &scc_m64, self),
        };

        arg_handler source = ARG_HANDLER_INIT("", &scc_source_file, self);

        aparser parser;
        aparser_init(&parser, argc, argv);
        aparse(&parser, handlers, S_ARRAY_SIZE(handlers), &source);
        return S_NO_ERROR;
}