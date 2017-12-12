#include "scc/cc/cc.h"
#include "scc/scl/args.h"

static void scc_cc_missing_argument(scc_cc* self, const char* arg)
{
        scc_cc_error(self, "argument to '%s' is missing", arg);
}

typedef enum
{
        SCAK_FILE,

        SCAK_S,
        SCAK_o,
        SCAK_help,
        SCAK_fsyntax_only,
        SCAK_flex_only,
        SCAK_fprint_eval_result,
        SCAK_fprint_expr_value,
        SCAK_fprint_expr_type,
        SCAK_fprint_impl_casts,
        SCAK_fforce_brackets,
        SCAK_fdce,
        SCAK_fcf,
        SCAK_emit_ssa,
        SCAK_I,
        SCAK_l,    
        SCAK_L,
        SCAK_m32,
        SCAK_m64,
} scc_cc_arg_kind;

static void scc_cc_S(scc_cc* self, aparser* parser)
{
        scc_cc_set_mode(self, SCRM_ASM_ONLY);
}

static void scc_cc_o(scc_cc* self, aparser* parser)
{
        const char* filename = aparser_get_string(parser);
        if (!filename)
        {
                scc_cc_missing_argument(self, "-o");
                return;
        }

        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path)) || S_FAILED(path_join(path, filename)))
        {
                scc_cc_unable_to_open(self, filename);
                return;
        }

        FILE* out = scc_cc_open_file(self, filename, "w");
        if (!out)
        {
                scc_cc_unable_to_open(self, filename);
                return;
        }

        scc_cc_set_output(self, out);
}

static void scc_cc_log(scc_cc* self, aparser* parser)
{
        const char* filename = aparser_get_string(parser);
        if (!filename)
        {
                scc_cc_missing_argument(self, "-log");
                return;
        }

        FILE* log = scc_cc_open_file(self, filename, "w");
        if (!log)
                return;

        scc_cc_set_log(self, log);
}

static void scc_cc_help(scc_cc* self, aparser* parser)
{
}

static void scc_cc_fsyntax_only(scc_cc* self, aparser* parser)
{
        scc_cc_set_mode(self, SCRM_SYNTAX_ONLY);
}

static void scc_cc_flex_only(scc_cc* self, aparser* parser)
{
        scc_cc_set_mode(self, SCRM_LEX_ONLY);
}

static void scc_cc_fprint_eval_result(scc_cc* self, aparser* parser)
{
        self->opts.print.flags |= SCPF_PRINT_EVAL_RESULT;
}

static void scc_cc_fprint_expr_value(scc_cc* self, aparser* parser)
{
        self->opts.print.flags |= SCPF_PRINT_EXPR_VALUE;
}

static void scc_cc_fprint_expr_type(scc_cc* self, aparser* parser)
{
        self->opts.print.flags |= SCPF_PRINT_EXPR_TYPE;
}

static void scc_cc_fprint_impl_casts(scc_cc* self, aparser* parser)
{
        self->opts.print.flags |= SCPF_PRINT_IMPL_CASTS;
}

static void scc_cc_fforce_brackets(scc_cc* self, aparser* parser)
{
        self->opts.print.flags |= SCPF_FORCE_BRACKETS;
}

static void scc_cc_fdce(scc_cc* self, aparser* parser)
{
        self->opts.optimization.eliminate_dead_code = true;
}

static void scc_cc_fcf(scc_cc* self, aparser* parser)
{
        self->opts.optimization.fold_constants = true;
}

static void scc_cc_emit_ssa(scc_cc* self, aparser* parser)
{
        self->opts.output = SCOK_SSA;
}

static void scc_cc_I(scc_cc* self, aparser* parser)
{
        const char* dir = aparser_get_string(parser);
        if (!dir)
        {
                scc_cc_missing_argument(self, "-i");
                return;
        }

        if (path_is_dir(dir))
        {
                scc_cc_add_source_dir(self, dir);
                return;
        }

        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path))
                || S_FAILED(path_join(path, dir)) || !path_is_dir(path))
        {
                scc_cc_file_doesnt_exist(self, dir);
                return;
        }

        scc_cc_add_source_dir(self, path);
}

static void scc_cc_l(scc_cc* self, aparser* parser)
{
}

static void scc_cc_L(scc_cc* self, aparser* parser)
{
}

static void scc_cc_m32(scc_cc* self, aparser* parser)
{
        self->opts.target = SCTK_32;
}

static void scc_cc_m64(scc_cc* self, aparser* parser)
{
        self->opts.target = SCTK_64;
}

static void scc_cc_file(scc_cc* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
                return;

        scc_cc_add_source_file(self, file);
}

extern serrcode scc_cc_parse_opts(scc_cc* self, int argc, const char** argv)
{
        arg_handler handlers[] = 
        {
                ARG_HANDLER_INIT("-S", &scc_cc_S, self),
                ARG_HANDLER_INIT("-o", &scc_cc_o, self),
                ARG_HANDLER_INIT("-log", &scc_cc_log, self),
                ARG_HANDLER_INIT("--help", &scc_cc_help, self),
                ARG_HANDLER_INIT("-fsyntax-only", &scc_cc_fsyntax_only, self),
                ARG_HANDLER_INIT("-flex-only", &scc_cc_flex_only, self),
                ARG_HANDLER_INIT("-fprint-eval-result", &scc_cc_fprint_eval_result, self),
                ARG_HANDLER_INIT("-fprint-expr-value", &scc_cc_fprint_expr_value, self),
                ARG_HANDLER_INIT("-fprint-expr-type", &scc_cc_fprint_expr_type, self),
                ARG_HANDLER_INIT("-fprint-impl-casts", &scc_cc_fprint_impl_casts, self),
                ARG_HANDLER_INIT("-fforce-brackets", &scc_cc_fforce_brackets, self),
                ARG_HANDLER_INIT("-fdce", &scc_cc_fdce, self),
                ARG_HANDLER_INIT("-fcf", &scc_cc_fcf, self),
                ARG_HANDLER_INIT("-emit-ssa", &scc_cc_emit_ssa, self),
                ARG_HANDLER_INIT("-I", &scc_cc_I, self),
                ARG_HANDLER_INIT("-L", &scc_cc_L, self),
                ARG_HANDLER_INIT("-m32", &scc_cc_m32, self),
                ARG_HANDLER_INIT("-m64", &scc_cc_m64, self),
        };

        arg_handler def;
        arg_handler_init(&def, "", &scc_cc_file, self);

        aparser parser;
        aparser_init(&parser, argc, argv);
        aparse(&parser, handlers, S_ARRAY_SIZE(handlers), &def);
        return S_NO_ERROR;
}