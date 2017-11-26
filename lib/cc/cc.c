#include "scc/cc/cc.h"
#include "scc/c/c-tree.h"
#include "scc/c/c-source.h"
#include "scc/c/c-error.h"
#include "scc/c/c-lexer.h"
#include "scc/c/c-printer.h"
#include "scc/c/c-parse-module.h"
#include "scc/c/c-sema.h"
#include "scc/ssa/ssa-printer.h"
#include "scc/ssa/ssaizer.h"
#include <stdarg.h>

static void* scc_cc_on_out_of_memory(base_allocator* alloc, ssize bytes, ssize align)
{
        scc_cc* self = (scc_cc*)((char*)alloc - offsetof(scc_cc, alloc));
        scc_cc_error(self, "unable to allocate memory");
        return NULL;
}

static allocator* scc_cc_alloc(scc_cc* self)
{
        return base_allocator_base(&self->alloc);
}

static void* scc_cc_allocate(scc_cc* self, ssize bytes)
{
        return allocate(scc_cc_alloc(self), bytes);
}

static void scc_cc_deallocate(scc_cc* self, void* block)
{
        deallocate(scc_cc_alloc(self), block);
}

extern void scc_cc_init(scc_cc* self, FILE* log, jmp_buf on_fatal_error)
{
        base_allocator_init(&self->alloc, (void*)&scc_cc_on_out_of_memory, on_fatal_error);

        allocator* alloc = scc_cc_alloc(self);
        self->log = log;
        self->out = NULL;

        tree_init_ex(&self->tree, &self->target, alloc);
        cinit_ex(&self->c, &self->tree, on_fatal_error, alloc);
        csource_manager_init(&self->source_manager, &self->c);
        cerror_manager_init(&self->error_manager, &self->source_manager, log);
        ssa_init_ex(&self->ssa, &self->tree, on_fatal_error, alloc);
        dseq_init_ex_ptr(&self->sources, alloc);

        scc_cc_opts* o = &self->opts;
        o->target = SCTK_32;
        o->mode = SCRM_DEFAULT;
        o->output = SCOK_C;

        o->print.flags = SCPF_NONE;
        o->print.double_precision = 4;
        o->print.float_precision = 4;
}

static char* scc_cc_copy_string(scc_cc* self, const char* string)
{
        char* copy = scc_cc_allocate(self, strlen(string) + 1);
        strcpy(copy, string);
        return copy;
}

extern void scc_cc_dispose(scc_cc* self)
{
        dseq_dispose(&self->sources);
        ssa_dispose(&self->ssa);
        csource_manager_dispose(&self->source_manager);
        cdispose(&self->c);
        tree_dispose(&self->tree);
        base_allocator_dispose(&self->alloc);
}

extern void scc_cc_error(scc_cc* self, const char* format, ...)
{
        fprintf(self->log, "error: ");
        va_list args;
        va_start(args, format);
        vfprintf(self->log, format, args);
        fprintf(self->log, "\n");
}

extern void scc_cc_file_doesnt_exist(scc_cc* self, const char* file)
{
        scc_cc_error(self, "no such file or directory '%s'", file);
}

extern void scc_cc_unable_to_open(scc_cc* self, const char* file)
{
        scc_cc_error(self, "unable to open '%s'", file);
}

extern FILE* scc_cc_open_existing_file(scc_cc* self, const char* file, const char* mode)
{
        FILE* f = scc_cc_open_file(self, file, mode);
        if (!f)
        {
                scc_cc_file_doesnt_exist(self, file);
                return NULL;
        }
        return f;
}

extern FILE* scc_cc_open_file(scc_cc* self, const char* file, const char* mode)
{
        char path[S_MAX_PATH_LEN];
        if (S_FAILED(path_get_cd(path) || S_FAILED(path_join(path, file))))
                return NULL;

        return fopen(path, mode);
}

extern void scc_cc_set_mode(scc_cc* self, scc_cc_run_mode mode)
{
        self->opts.mode = mode;
}

extern void scc_cc_set_output(scc_cc* self, FILE* out)
{
        self->out = out;
}

extern void scc_cc_set_log(scc_cc* self, FILE* log)
{
        self->log = log;
        cerror_manager_set_output(&self->error_manager, log);
}

extern serrcode scc_cc_add_input_file(scc_cc* self, const char* file)
{
        S_ASSERT(file);
        csource* s = csource_find(&self->source_manager, file);
        if (!s)
        {
                scc_cc_file_doesnt_exist(self, file);
                return S_ERROR;
        }

        dseq_append_ptr(&self->sources, s);
        return S_NO_ERROR;
}

extern serrcode scc_cc_emulate_file(scc_cc* self, const char* filename, const char* content)
{
        csource* s = csource_emulate(&self->source_manager, filename, content);
        if (!s)
                return S_ERROR;

        dseq_append_ptr(&self->sources, s);
        return S_NO_ERROR;
}

extern void scc_cc_add_lookup_dir(scc_cc* self, const char* dir)
{
        csource_manager_add_lookup(&self->source_manager, dir);
}

static void scc_cc_set_printer_opts(scc_cc* self, cprinter_opts* o)
{
        scc_cc_print_opts* so = &self->opts.print;
        o->double_precision = so->double_precision;
        o->float_precision = so->float_precision;
        o->force_brackets = so->flags & SCPF_FORCE_BRACKETS;
        o->print_eval_result = so->flags & SCPF_EVAL_RESULT;
        o->print_expr_type = so->flags & SCPF_EXPR_TYPE;
        o->print_expr_value = so->flags & SCPF_EXPR_VALUE;
        o->print_impl_casts = so->flags & SCPF_IMPL_CASTS;
}

extern serrcode scc_cc_lex(scc_cc* self)
{
        csource* source = dseq_first_ptr(&self->sources);
        S_ASSERT(source);

        serrcode result = S_ERROR;
        clexer lexer;
        clexer_init(&lexer, &self->source_manager, &self->error_manager, &self->c);
        clexer_init_reswords(&lexer);

        if (S_FAILED(clexer_enter_source_file(&lexer, source)))
                goto cleanup;

        dseq tokens;
        dseq_init_ex_ptr(&tokens, scc_cc_alloc(self));

        while (1)
        {
                ctoken* t = clex(&lexer);
                if (!t)
                        goto cleanup;

                dseq_append_ptr(&tokens, t);
                if (ctoken_is(t, CTK_EOF))
                        break;
        }
        
        result = S_NO_ERROR;
        if (!self->out)
                goto cleanup;

        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &self->c, &self->source_manager);
        scc_cc_set_printer_opts(self, &printer.opts);
        cprint_tokens(&printer, &tokens);
        cprinter_dispose(&printer);

cleanup:
        dseq_dispose(&tokens);
        clexer_dispose(&lexer);
        return S_ERROR;
}

static tree_module* scc_cc_parse_source(scc_cc* self, csource* source)
{
        S_ASSERT(source);

        tree_module* m = tree_new_module(&self->tree);
        tree_module* result = NULL;

        clexer lexer;
        clexer_init(&lexer, &self->source_manager, &self->error_manager, &self->c);
        clexer_init_reswords(&lexer);

        csema sema;
        csema_init(&sema, &self->c, m, &self->error_manager);

        jmp_buf on_parser_error;
        cparser parser;
        cparser_init(&parser, &lexer, &sema, &self->error_manager);

        if (S_FAILED(clexer_enter_source_file(&lexer, source)))
                goto cleanup;
        if (setjmp(on_parser_error))
                goto cleanup;

        cparser_set_on_error(&parser, on_parser_error);
        cparser_enter_token_stream(&parser);
        result = cparse_module(&parser);

cleanup:
        cparser_dispose(&parser);
        csema_dispose(&sema);
        clexer_dispose(&lexer);
        return result;
}

extern serrcode scc_cc_parse(scc_cc* self)
{
        csource* source = dseq_first_ptr(&self->sources);
        S_ASSERT(source);

        tree_module* m = scc_cc_parse_source(self, source);
        if (!m)
                return S_ERROR;

        if (!self->out || self->opts.output != SCOK_C)
                return S_NO_ERROR;

        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        cprinter printer;
        cprinter_init(&printer, fwrite_cb_base(&write), &self->c, &self->source_manager);
        scc_cc_set_printer_opts(self, &printer.opts);
        cprint_module(&printer, m);
        cprinter_dispose(&printer);

        return S_NO_ERROR;
}

extern serrcode scc_cc_gen(scc_cc* self)
{
        csource* source = dseq_first_ptr(&self->sources);
        S_ASSERT(source);

        tree_module* m = scc_cc_parse_source(self, source);
        if (!m)
                return S_ERROR;

        if (!self->out || self->opts.output != SCOK_SSA)
                return S_NO_ERROR;

        ssaizer sr;
        ssaizer_init(&sr, &self->ssa);

        ssa_module* sm = ssaize_module(&sr, m);
        ssaizer_dispose(&sr);
        if (!sm)
                return S_ERROR;

        fwrite_cb write;
        fwrite_cb_init(&write, self->out);
        ssa_printer printer;
        ssa_init_printer(&printer, fwrite_cb_base(&write), &self->ssa);
        ssa_print_module(&printer, sm);
        ssa_dispose_printer(&printer);

        return S_NO_ERROR;
}

extern serrcode scc_cc_run(scc_cc* self)
{
        tree_init_target_info(&self->target,
                self->opts.target == SCTK_32 ? TTARGET_X32 : TTARGET_X64);

        if (!dseq_size(&self->sources))
        {
                scc_cc_error(self, "no input files");
                return S_ERROR;
        }

        scc_cc_run_mode m = self->opts.mode;
        if (m == SCRM_DEFAULT)
                return scc_cc_gen(self);
        else if (m == SCRM_LEX_ONLY)
                return scc_cc_lex(self);
        else if (m == SCRM_SYNTAX_ONLY)
                return scc_cc_parse(self);

        return S_ERROR;
}
