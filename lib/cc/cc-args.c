#include "scc/cc/cc.h"
#include "cc-impl.h"
#include "scc/scl/args.h"

static void cc_missing_argument(cc_instance* self, const char* arg)
{
        cc_error(self, "argument to '%s' is missing", arg);
}

typedef enum
{
        CRM_ASSEMBLE,
        CRM_COMPILE,
        CRM_LINK,
} cc_run_mode;

typedef struct
{
        cc_instance* cc;
        cc_run_mode mode;
} cc_arg_info;

extern void cc_arg_info_init(cc_arg_info* self, cc_instance* cc)
{
        self->cc = cc;
        self->mode = CRM_LINK;
}

typedef enum
{
        CAK_S,
        CAK_c,
        CAK_o,
        CAK_log,
        CAK_help,
        CAK_fsyntax_only,
        CAK_dump_tokens,
        CAK_dump_tree,
        CAK_fprint_eval_result,
        CAK_fprint_expr_value,
        CAK_fprint_expr_type,
        CAK_fprint_impl_casts,
        CAK_fforce_brackets,
        CAK_fdce,
        CAK_fcf,
        CAK_emit_ssa,
        CAK_emit_llvm,
        CAK_I,
        CAK_l,    
        CAK_L,
        CAK_m32,
        CAK_m64,
} cc_arg_kind;

static void cc_parse_S(cc_arg_info* self, aparser* parser)
{
        self->mode = CRM_ASSEMBLE;
        self->cc->output.kind = COK_ASM;
}

static void cc_parse_c(cc_arg_info* self, aparser* parser)
{
        self->mode = CRM_COMPILE;
        self->cc->output.kind = COK_OBJ;
}

static void cc_parse_o(cc_arg_info* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
        {
                cc_missing_argument(self->cc, "-o");
                return;
        }
        cc_set_output_file(self->cc, file);
}

static void cc_parse_log(cc_arg_info* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
        {
                cc_missing_argument(self->cc, "-log");
                return;
        }

        FILE* log = cc_open_file(self->cc, file, "w");
        if (!log)
                return;

        self->cc->output.message = log;
}

static void cc_parse_help(cc_arg_info* self, aparser* parser)
{
}

static void cc_parse_fsyntax_only(cc_arg_info* self, aparser* parser)
{
        self->cc->output.kind = COK_NONE;
}

static void cc_parse_dump_tokens(cc_arg_info* self, aparser* parser)
{
        self->cc->output.kind = COK_LEXEMES;
}

static void cc_parse_dump_tree(cc_arg_info* self, aparser* parser)
{
        self->cc->output.kind = COK_C;
}

static void cc_parse_fprint_eval_result(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.cprint.print_eval_result = true;
}

static void cc_parse_fprint_expr_value(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.cprint.print_expr_value = true;
}

static void cc_parse_fprint_expr_type(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.cprint.print_expr_type = true;
}

static void cc_parse_fprint_impl_casts(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.cprint.print_impl_casts = true;
}

static void cc_parse_fforce_brackets(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.cprint.force_brackets = true;
}

static void cc_parse_fdce(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.optimization.eliminate_dead_code = true;
}

static void cc_parse_fcf(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.optimization.fold_constants = true;
}

static void cc_parse_emit_ssa(cc_arg_info* self, aparser* parser)
{
        if (self->mode != CRM_ASSEMBLE)
        {
                cc_error(self->cc, "-emit-ssa cannot be used when linking");
                return;
        }

        self->cc->output.kind = COK_SSA;
}

static void cc_parse_emit_llvm(cc_arg_info* self, aparser* parser)
{
        if (self->mode != CRM_ASSEMBLE)
        {
                cc_error(self->cc, "-emit-llvm cannot be used when linking");
                return;
        }

        self->cc->output.kind = COK_LLVM_IR;
}

static void cc_parse_I(cc_arg_info* self, aparser* parser)
{
        const char* dir = aparser_get_string(parser);
        if (!dir)
        {
                cc_missing_argument(self->cc, "-I");
                return;
        }

        cc_add_source_dir(self->cc, dir);
}

static void cc_parse_l(cc_arg_info* self, aparser* parser)
{
        const char* lib = aparser_get_string(parser);
        if (!lib)
        {
                cc_missing_argument(self->cc, "-l");
                return;
        }

        cc_add_lib(self->cc, lib);
}

static void cc_parse_L(cc_arg_info* self, aparser* parser)
{
        const char* dir = aparser_get_string(parser);
        if (!dir)
        {
                cc_missing_argument(self->cc, "-L");
                return;
        }

        cc_add_lib_dir(self->cc, dir);
}

static void cc_parse_m32(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.target = CTK_X86_32;
}

static void cc_parse_m64(cc_arg_info* self, aparser* parser)
{
        self->cc->opts.target = CTK_X86_64;
}

static void cc_source_file(cc_arg_info* self, aparser* parser)
{
        const char* file = aparser_get_string(parser);
        if (!file)
                return;

        cc_add_source_file(self->cc, file);
}

extern serrcode cc_parse_opts(cc_instance* self, int argc, const char** argv)
{
        cc_arg_info info;
        cc_arg_info_init(&info, self);

        arg_handler handlers[] = 
        {
                ARG_HANDLER_INIT("-S", &cc_parse_S, &info),
                ARG_HANDLER_INIT("-c", &cc_parse_c, &info),
                ARG_HANDLER_INIT("-o", &cc_parse_o, &info),
                ARG_HANDLER_INIT("-log", &cc_parse_log, &info),
                ARG_HANDLER_INIT("-fsyntax-only", &cc_parse_fsyntax_only, &info),
                ARG_HANDLER_INIT("-dump-tokens", &cc_parse_dump_tokens, &info),
                ARG_HANDLER_INIT("-dump-tree", &cc_parse_dump_tree, &info),
                ARG_HANDLER_INIT("-fprint-eval-result", &cc_parse_fprint_eval_result, &info),
                ARG_HANDLER_INIT("-fprint-expr-value", &cc_parse_fprint_expr_value, &info),
                ARG_HANDLER_INIT("-fprint-expr-type", &cc_parse_fprint_expr_type, &info),
                ARG_HANDLER_INIT("-fprint-impl-casts", &cc_parse_fprint_impl_casts, &info),
                ARG_HANDLER_INIT("-fforce-brackets", &cc_parse_fforce_brackets, &info),
                ARG_HANDLER_INIT("-fdce", &cc_parse_fdce, &info),
                ARG_HANDLER_INIT("-fcf", &cc_parse_fcf, &info),
                ARG_HANDLER_INIT("-emit-ssa", &cc_parse_emit_ssa, &info),
                ARG_HANDLER_INIT("-emit-llvm", &cc_parse_emit_llvm, &info),
                ARG_HANDLER_INIT("-I", &cc_parse_I, &info),
                ARG_HANDLER_INIT("-l", &cc_parse_l, &info),
                ARG_HANDLER_INIT("-L", &cc_parse_L, &info),
                ARG_HANDLER_INIT("-m32", &cc_parse_m32, &info),
                ARG_HANDLER_INIT("-m64", &cc_parse_m64, &info),
        };

        arg_handler source;
        arg_handler_init(&source, "", &cc_source_file, &info);

        aparser parser;
        aparser_init(&parser, argc, argv);
        aparse(&parser, handlers, S_ARRAY_SIZE(handlers), &source);
        return S_NO_ERROR;
}