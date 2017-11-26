#include "scc/cc/cc.h"
#include "scc/scl/arg-parser.h"

typedef struct
{
        aparser_cb base;
        scc_cc* cc;
} scc_cc_arg_handler;

static void scc_cc_missing_argument(scc_cc* self, const char* arg)
{
        scc_cc_error(self, "argument to '%s' is missing", arg);
}

static void scc_cc_syntax_only(scc_cc_arg_handler* self, aparser* p)
{
        scc_cc_set_mode(self->cc, SCRM_SYNTAX_ONLY);
}

static void scc_cc_lex_only(scc_cc_arg_handler* self, aparser* p)
{
        scc_cc_set_mode(self->cc, SCRM_LEX_ONLY);
}

static void scc_cc_log(scc_cc_arg_handler* self, aparser* p)
{
        const char* filename = aparser_get_string(p);
        if (!filename)
        {
                scc_cc_missing_argument(self->cc, "-log");
                return;
        }

        FILE* log = scc_cc_open_file(self->cc, filename, "w");
        if (!log)
                return;

        scc_cc_set_log(self->cc, log);
}

static void scc_cc_o(scc_cc_arg_handler* self, aparser* p)
{
        const char* filename = aparser_get_string(p);
        if (!filename)
        {
                scc_cc_missing_argument(self->cc, "-o");
                return;
        }

        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path)) || S_FAILED(path_join(path, filename)))
        {
                scc_cc_unable_to_open(self->cc, filename);
                return;
        }

        FILE* out = scc_cc_open_file(self->cc, filename, "w");
        if (!out)
        {
                scc_cc_unable_to_open(self->cc, filename);
                return;
        }

        scc_cc_set_output(self->cc, out);
}

static void scc_cc_i(scc_cc_arg_handler* self, aparser* p)
{
        const char* dir = aparser_get_string(p);
        if (!dir)
        {
                scc_cc_missing_argument(self->cc, "-i");
                return;
        }

        if (path_is_dir(dir))
        {
                scc_cc_add_lookup_dir(self->cc, dir);
                return;
        }

        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path))
                || S_FAILED(path_join(path, dir)) || !path_is_dir(path))
        {
                scc_cc_file_doesnt_exist(self->cc, dir);
                return;
        }

        scc_cc_add_lookup_dir(self->cc, path);
}

static void scc_cc_print_eval_result(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.print.flags |= SCPF_EVAL_RESULT;
}

static void scc_cc_print_expr_value(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.print.flags |= SCPF_EXPR_VALUE;
}

static void scc_cc_print_expr_type(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.print.flags |= SCPF_EXPR_TYPE;
}

static void scc_cc_print_impl_casts(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.print.flags |= SCPF_IMPL_CASTS;
}

static void scc_cc_force_brackets(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.print.flags |= SCPF_FORCE_BRACKETS;
}

static void scc_cc_float_precision(scc_cc_arg_handler* self, aparser* p)
{
        S_UNREACHABLE();
}

static void scc_cc_double_precision(scc_cc_arg_handler* self, aparser* p)
{
        S_UNREACHABLE();
}

static void scc_cc_x32(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.target = SCTK_32;
}

static void scc_cc_x64(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.target = SCTK_64;
}

static void scc_cc_unknown(scc_cc_arg_handler* self, aparser* p)
{
        const char* file = aparser_get_string(p);
        if (!file)
                return;

        scc_cc_add_input_file(self->cc, file);
}

static void scc_cc_emit_ssa(scc_cc_arg_handler* self, aparser* p)
{
        self->cc->opts.log = SCOK_SSA;
}

static void scc_cc_help(scc_cc_arg_handler* self, aparser* p)
{
        const char* d =
                "Options:\n"
                " -syntax-only       Syntax analysis only\n"
                " -lex-only          Lexical analysis only\n"
                " -log <file>        Specify the log file\n"
                " -o <file>          Specify the output file\n"
                " -i <dir>           Specify the include directory\n"
                " -emit-ssa          Use the SSA representation for output files\n"
                " -print-eval-result Print constant expression value\n"
                " -print-expr-value  Print expression value (lvalue or rvalue)\n"
                " -print-expr-type   Print expression type\n"
                " -print-impl-casts  Print implicit casts\n"
                " -force-brackets    Force bracket printing\n"
                " -float-precision   Specify the float precision, when printing\n"
                " -double-precision  Specify the double precision, when printing\n"
                " -x32               Set X86 target architecture\n"
                " -x64               Set X86-64 target architecture\n"
                ;
        fprintf(self->cc->log, "%s\n", d);
}

typedef enum
{
        SCAK_UNKNOWN,
        SCAK_SYNTAX_ONLY,
        SCAK_LEX_ONLY,
        SCAK_LOG,
        SCAK_O,
        SCAL_I,
        SCAK_PRINT_EVAL_RESULT,
        SCAK_PRINT_EXPR_VALUE,
        SCAK_PRINT_EXPR_TYPE,
        SCAK_PRINT_IMPL_CASTS,
        SCAK_FORCE_BRACKETS,
        SCAK_FLOAT_PRECISION,
        SCAK_DOUBLE_PRECISION,
        SCAK_X32,
        SCAK_X64,
        SCAK_EMIT_SSA,
        SCAK_HELP,

        SCAK_SIZE,
} scc_cc_arg_kind;

typedef struct
{
        void(*handler)(scc_cc_arg_handler*, aparser*);
        const char* string;
} scc_cc_arg_info;

static scc_cc_arg_info scc_cc_arg_table[SCAK_SIZE] =
{
        { &scc_cc_unknown, "" },
        { &scc_cc_syntax_only, "-syntax-only" },
        { &scc_cc_lex_only, "-lex-only" },
        { &scc_cc_log, "-log" },
        { &scc_cc_o, "-o" },
        { &scc_cc_i, "-i" },
        { &scc_cc_print_eval_result, "-print-eval-result" },
        { &scc_cc_print_expr_value, "-print-expr-value" },
        { &scc_cc_print_expr_type, "-print-expr-type" },
        { &scc_cc_print_impl_casts, "-print-impl-casts" },
        { &scc_cc_force_brackets, "-force-brackets" },
        { &scc_cc_float_precision, "-float-precision" },
        { &scc_cc_double_precision, "-double-precision" },
        { &scc_cc_x32, "-x32" },
        { &scc_cc_x64, "-x64" },
        { &scc_cc_emit_ssa, "-emit-ssa" },
        { &scc_cc_help, "-help" },
};

static void scc_cc_arg_handler_init(scc_cc_arg_handler* self, scc_cc* cc, scc_cc_arg_kind k)
{
        self->cc = cc;
        aparser_cb_init(&self->base, scc_cc_arg_table[k].handler);
}

extern serrcode scc_cc_parse_opts(scc_cc* self, int argc, const char** argv)
{
        scc_cc_arg_handler handlers[SCAK_SIZE];
        for (int i = 0; i < SCAK_SIZE; i++)
                scc_cc_arg_handler_init(&handlers[i], self, i);

        serrcode result = S_ERROR;
        aparser parser;
        aparser_init(&parser, argc, argv);

        aparser_add_default_handler(&parser, &handlers[0].base);

        for (int i = 1; i < SCAK_SIZE; i++)
                if (S_FAILED(aparser_add_handler(&parser,
                        scc_cc_arg_table[i].string, &handlers[i].base)))
                {
                        goto cleanup;
                }


        result = S_NO_ERROR;
        aparse(&parser);

cleanup:
        aparser_dispose(&parser);
        return result;
}