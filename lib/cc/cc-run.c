#include "scc/cc/cc-run.h"
#include "scc/c/c-env.h"
#include "scc/c/c-printer.h"
#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-printer.h"

static scc_cc_set_cprinter_opts(scc_cc* self, cprinter* printer)
{
        scc_cc_print_opts* so = &self->opts.print;
        cprinter_opts* o = &printer->opts;

        o->force_brackets = so->flags & SCPF_FORCE_BRACKETS;
        o->print_eval_result = so->flags & SCPF_PRINT_EVAL_RESULT;
        o->print_expr_type = so->flags & SCPF_PRINT_EXPR_TYPE;
        o->print_expr_value = so->flags & SCPF_PRINT_EXPR_VALUE;
        o->print_impl_casts = so->flags & SCPF_PRINT_IMPL_CASTS;
}

static scc_cc_print_tokens(scc_cc* self, const csource_manager* sm, const dseq* tokens)
{
        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &self->c, sm);
        scc_cc_set_cprinter_opts(self, &printer);
        cprint_tokens(&printer, tokens);
        cprinter_dispose(&printer);
}

extern serrcode scc_cc_lex(scc_cc* self)
{
        cenv env;
        cenv_init(&env, &self->c, &self->source_lookup, self->log);

        dseq tokens;
        dseq_init_ex_ptr(&tokens, scc_cc_alloc(self));

        serrcode result = cenv_lex_source(&env, dseq_first_ptr(&self->sources), &tokens);
        if (S_SUCCEEDED(result))
                scc_cc_print_tokens(self, &env.source_manager, &tokens);

        dseq_dispose(&tokens);
        cenv_dispose(&env);
        return result;
}

static void scc_cc_print_module(scc_cc* self, const csource_manager* sm, tree_module* module)
{
        if (!self->out || self->opts.output != SCOK_C)
                return;

        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &self->c, sm);
        scc_cc_set_cprinter_opts(self, &printer);
        cprint_module(&printer, module);
        cprinter_dispose(&printer);
}

extern serrcode scc_cc_parse(scc_cc* self)
{
        cenv env;
        cenv_init(&env, &self->c, &self->source_lookup, self->log);

        tree_module* module = cenv_parse_source(&env, dseq_first_ptr(&self->sources));
        if (module)
                scc_cc_print_module(self, &env.source_manager, module);

        cenv_dispose(&env);
        return module ? S_NO_ERROR : S_ERROR;
}

static void scc_cc_set_ssa_optimizer_opts(scc_cc* self, ssa_optimizer* opt)
{
        if (self->opts.optimization.constant_folding)
                ssa_optimizer_enable_pass(opt, SOK_CONSTANT_FOLDING);
        if (self->opts.optimization.eliminate_dead_code)
                ssa_optimizer_enable_pass(opt, SOK_DEAD_CODE_ELIMINATION);
}

static serrcode scc_cc_compile_ssa(scc_cc* self, tree_module** ptm, ssa_module** psm)
{
        tree_module* tm = cparse_source(&self->c,
                &self->source_lookup, dseq_first_ptr(&self->sources), self->log);
        if (!tm)
                return S_ERROR;

        ssaizer sr;
        ssaizer_init(&sr, &self->ssa);
        ssa_module* sm = ssaize_module(&sr, tm);
        ssaizer_dispose(&sr);
        if (!sm)
                return S_ERROR;

        ssa_optimizer so;
        ssa_init_optimizer(&so);
        scc_cc_set_ssa_optimizer_opts(self, &so);
        ssa_optimizer_run(&so, &self->ssa, sm);
        
        *ptm = tm;
        *psm = sm;
        return S_NO_ERROR;
}

static void scc_cc_print_ssa_module(scc_cc* self, ssa_module* sm)
{
        if (!self->out || self->opts.output != SCOK_SSA)
                return;

        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        ssa_printer printer;
        ssa_init_printer(&printer, fwrite_cb_base(&write), &self->ssa);
        ssa_print_module(&printer, sm);
        ssa_dispose_printer(&printer);
}

extern serrcode scc_cc_compile(scc_cc* self)
{
        tree_module* tm;
        ssa_module* sm;
        if (S_FAILED(scc_cc_compile_ssa(self, &tm, &sm)))
                return S_ERROR;
       
        scc_cc_print_ssa_module(self, sm);
        return S_NO_ERROR;
}

extern serrcode scc_cc_link(scc_cc* self)
{
        return S_ERROR;
}