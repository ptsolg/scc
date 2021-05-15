#include "scc.h"
#include "scc/cc/cc.h"
#include "scc/core/cmd.h"
#include "scc/core/common.h"
#include "scc/core/file.h"

#include <string.h>

struct parser
{
        struct arg_parser p;
        scc_env* env;
};

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

static void scc_S(struct parser* p)
{
        p->env->mode = SRM_ASSEMBLE;
        p->env->cc.output.kind = COK_ASM;
}

static void scc_c(struct parser* p)
{
        p->env->mode = SRM_COMPILE;
        p->env->cc.output.kind = COK_OBJ;
}

static void scc_o(struct parser* p)
{
        const char* file = arg_parser_next_str(&p->p);
        if (!file)
        {
                scc_missing_argument(p->env, "-o");
                return;
        }
        cc_set_output_file(&p->env->cc, file);
}

static void scc_nostdlib(struct parser* p)
{
        p->env->link_stdlib = false;
}

static void scc_log(struct parser* p)
{
        const char* file = arg_parser_next_str(&p->p);
        if (!file)
        {
                scc_missing_argument(p->env, "-log");
                return;
        }

        FILE* log = scc_open_file(p->env, file, "w");
        if (!log)
                return;

        p->env->cc.output.message = log;
}

static void scc_help(struct parser* p)
{
}

static void scc_fsyntax_only(struct parser* p)
{
        p->env->cc.output.kind = COK_NONE;
        p->env->mode = SRM_OTHER;
}

static void scc_dump_tokens(struct parser* p)
{
        p->env->cc.output.kind = COK_LEXEMES;
        p->env->mode = SRM_OTHER;
}

static void scc_dump_tree(struct parser* p)
{
        p->env->cc.output.kind = COK_C;
        p->env->mode = SRM_OTHER;
}

static void scc_fprint_eval_result(struct parser* p)
{
        p->env->cc.opts.cprint.print_eval_result = true;
}

static void scc_fprint_expr_value(struct parser* p)
{
        p->env->cc.opts.cprint.print_expr_value = true;
}

static void scc_fprint_expr_type(struct parser* p)
{
        p->env->cc.opts.cprint.print_expr_type = true;
}

static void scc_fprint_sem_init(struct parser* p)
{
        p->env->cc.opts.cprint.print_semantic_init = true;
}

static void scc_fprint_impl_casts(struct parser* p)
{
        p->env->cc.opts.cprint.print_impl_casts = true;
}

static void scc_fforce_brackets(struct parser* p)
{
        p->env->cc.opts.cprint.force_brackets = true;
}

static void scc_fdce(struct parser* p)
{
        p->env->cc.opts.optimization.eliminate_dead_code = true;
}

static void scc_fcf(struct parser* p)
{
        p->env->cc.opts.optimization.fold_constants = true;
}

static void scc_ftm(struct parser* p)
{
        p->env->cc.opts.ext.enable_tm = true;
}

static void scc_emit_ssa(struct parser* p)
{
        if (p->env->mode != SRM_ASSEMBLE)
        {
                scc_error(p->env, "-emit-ssa cannot be used when linking");
                return;
        }

        p->env->cc.output.kind = COK_SSA;
}

static void scc_emit_llvm(struct parser* p)
{
        if (p->env->mode != SRM_ASSEMBLE)
        {
                scc_error(p->env, "-emit-llvm cannot be used when linking");
                return;
        }

        p->env->cc.output.kind = COK_LLVM_IR;
}

static void scc_I(struct parser* p)
{
        const char* dir = arg_parser_next_str(&p->p);
        if (!dir)
        {
                scc_missing_argument(p->env, "-I");
                return;
        }

        cc_add_source_dir(&p->env->cc, dir);
}

static void scc_l(struct parser* p)
{
        const char* lib = arg_parser_next_str(&p->p);
        if (!lib)
        {
                scc_missing_argument(p->env, "-l");
                return;
        }

        cc_add_lib(&p->env->cc, lib);
}

static void scc_L(struct parser* p)
{
        const char* dir = arg_parser_next_str(&p->p);
        if (!dir)
        {
                scc_missing_argument(p->env, "-L");
                return;
        }

        cc_add_lib_dir(&p->env->cc, dir);
}

static void scc_m32(struct parser* p)
{
        p->env->cc.opts.target = CTK_X86_32;
}

static void scc_m64(struct parser* p)
{
        p->env->cc.opts.target = CTK_X86_64;
}

static void scc_fpa(struct parser* p)
{
        p->env->cc.opts.optimization.promote_allocas = true;
}

static void scc_finline(struct parser* p)
{
        UNREACHABLE();
}

static void scc_O3(struct parser* p)
{
        p->env->cc.opts.optimization.eliminate_dead_code = true;
        p->env->cc.opts.optimization.promote_allocas = true;
        p->env->cc.opts.optimization.fold_constants = true;
        p->env->cc.opts.optimization.level = 3;
}

static void scc_file(struct parser* p)
{
        const char* file = arg_parser_next_str(&p->p);
        if (!file)
                return;

        const char* ext = pathext(file);
        bool ok = strcmp(ext, "o") == 0 || strcmp(ext, "obj") == 0
                ? EC_SUCCEEDED(cc_add_obj_file(&p->env->cc, file))
                : EC_SUCCEEDED(cc_add_source_file(&p->env->cc, file, false));
        if (!ok)
                return;

        struct pathbuf dir = pathbuf_from_str(file);
        basename(dir.buf);
        cc_add_source_dir(&p->env->cc, dir.buf);
}

extern void scc_parse_opts(scc_env* self, int argc, const char** argv)
{
        struct arg_handler handlers[] =
        {
                ARG_HANDLER("-S", &scc_S),
                ARG_HANDLER("-c", &scc_c ),
                ARG_HANDLER("-o", &scc_o),
                ARG_HANDLER("-nostdlib", &scc_nostdlib),
                ARG_HANDLER("-log", &scc_log),
                ARG_HANDLER("-fsyntax-only", &scc_fsyntax_only),
                ARG_HANDLER("-dump-tokens", &scc_dump_tokens),
                ARG_HANDLER("-dump-tree", &scc_dump_tree),
                ARG_HANDLER("-fprint-eval-result", &scc_fprint_eval_result),
                ARG_HANDLER("-fprint-expr-value",  &scc_fprint_expr_value),
                ARG_HANDLER("-fprint-expr-type", &scc_fprint_expr_type),
                ARG_HANDLER("-fprint-sem-init", &scc_fprint_sem_init),
                ARG_HANDLER("-fprint-impl-casts", &scc_fprint_impl_casts),
                ARG_HANDLER("-fforce-brackets", &scc_fforce_brackets),
                ARG_HANDLER("-fdce", &scc_fdce),
                ARG_HANDLER("-fcf", &scc_fcf),
                ARG_HANDLER("-fpa", &scc_fpa),
                ARG_HANDLER("-ftm", &scc_ftm),
                ARG_HANDLER("-finline", &scc_finline),
                ARG_HANDLER("-emit-ssa", &scc_emit_ssa),
                ARG_HANDLER("-emit-llvm", &scc_emit_llvm),
                ARG_HANDLER("-I", &scc_I),
                ARG_HANDLER("-l", &scc_l),
                ARG_HANDLER("-L", &scc_L),
                ARG_HANDLER("-m32", &scc_m32),
                ARG_HANDLER("-m64", &scc_m64),
                ARG_HANDLER("-O3", &scc_O3),
        };
        struct arg_handler src = ARG_HANDLER("", &scc_file);
        struct parser p;
        init_arg_parser(&p.p, argc, argv);
        p.env = self;
        run_arg_parser(&p.p, handlers, ARRAY_SIZE(handlers), &src);
}
