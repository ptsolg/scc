#include "scc/c/c-env.h"
#include "scc/c/c-parse-module.h"

extern serrcode clex_source(
        ccontext* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error,
        dseq* result)
{
        cenv env;
        cenv_init(&env, context, source_lookup, error);
        serrcode err = cenv_lex_source(&env, source, result);
        cenv_dispose(&env);
        return err;
}

extern tree_module* cparse_source(
        ccontext* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error)
{
        cenv env;
        cenv_init(&env, context, source_lookup, error);
        tree_module* module = cenv_parse_source(&env, source);
        cenv_dispose(&env);
        return module;
}


extern void cenv_init(cenv* self, ccontext* context, file_lookup* source_lookup, FILE* err)
{
        self->context = context;
        csource_manager_init(&self->source_manager, source_lookup, context);
        clogger_init(&self->logger, &self->source_manager, self->context->tree, err);
        clexer_init(&self->lexer, &self->source_manager, &self->logger, context);
        clexer_init_reswords(&self->lexer);
        csema_init(&self->sema, context, &self->logger);
        cparser_init(&self->parser, &self->lexer, &self->sema, &self->logger);
}

extern void cenv_dispose(cenv* self)
{
        cparser_dispose(&self->parser);
        csema_dispose(&self->sema);
        clexer_dispose(&self->lexer);
        csource_manager_dispose(&self->source_manager);
}

extern serrcode cenv_lex_source(cenv* self, file_entry* source, dseq* result)
{
        csource* s = csource_get_from_file(&self->source_manager, source);
        if (!source || S_FAILED(clexer_enter_source_file(&self->lexer, s)))
                return S_ERROR;

        while (1)
        {
                ctoken* t = clex(&self->lexer);
                if (!t || S_FAILED(dseq_append(result, t)))
                        return S_ERROR;

                if (ctoken_is(t, CTK_EOF))
                        break;
        }

        return S_NO_ERROR;
}

extern tree_module* cenv_parse_source(cenv* self, file_entry* source)
{
        csource* s = csource_get_from_file(&self->source_manager, source);
        if (!source || S_FAILED(clexer_enter_source_file(&self->lexer, s)))
                return NULL;

        jmp_buf on_parser_error;
        if (setjmp(on_parser_error))
                return NULL;

        tree_module* module = tree_new_module(cget_tree(self->context));
        csema_enter_module(&self->sema, module);
        cparser_set_on_error(&self->parser, on_parser_error);
        cparser_enter_token_stream(&self->parser);
        return cparse_module(&self->parser);
}