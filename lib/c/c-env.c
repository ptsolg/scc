#include "scc/c/c-env.h"
#include "scc/c/c-parse-module.h"

extern errcode c_lex_source(
        c_context* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error,
        dseq* result)
{
        c_env env;
        c_env_init(&env, context, source_lookup, error);
        errcode err = c_env_lex_source(&env, source, result);
        c_env_dispose(&env);
        return err;
}

extern tree_module* c_parse_source(
        c_context* context,
        file_lookup* source_lookup,
        file_entry* source,
        FILE* error)
{
        c_env env;
        c_env_init(&env, context, source_lookup, error);
        tree_module* module = c_env_parse_source(&env, source);
        c_env_dispose(&env);
        return module;
}


extern void c_env_init(c_env* self, c_context* context, file_lookup* source_lookup, FILE* err)
{
        self->context = context;
        c_source_manager_init(&self->source_manager, source_lookup, context);
        c_logger_init(&self->logger, &self->source_manager, self->context->tree, err);
        c_lexer_init(&self->lexer, &self->source_manager, &self->logger, context);
        c_lexer_init_reswords(&self->lexer);
        c_sema_init(&self->sema, context, &self->logger);
        c_parser_init(&self->parser, &self->lexer, &self->sema, &self->logger);
}

extern void c_env_dispose(c_env* self)
{
        c_parser_dispose(&self->parser);
        c_sema_dispose(&self->sema);
        c_lexer_dispose(&self->lexer);
        c_source_manager_dispose(&self->source_manager);
}

extern errcode c_env_lex_source(c_env* self, file_entry* source, dseq* result)
{
        c_source* s = c_source_get_from_file(&self->source_manager, source);
        if (!source || EC_FAILED(c_lexer_enter_source_file(&self->lexer, s)))
                return EC_ERROR;

        while (1)
        {
                c_token* t = c_lex(&self->lexer);
                if (!t || EC_FAILED(dseq_append(result, t)))
                        return EC_ERROR;

                if (c_token_is(t, CTK_EOF))
                        break;
        }

        return EC_NO_ERROR;
}

extern tree_module* c_env_parse_source(c_env* self, file_entry* source)
{
        c_source* s = c_source_get_from_file(&self->source_manager, source);
        if (!source || EC_FAILED(c_lexer_enter_source_file(&self->lexer, s)))
                return NULL;

        jmp_buf on_parser_error;
        if (setjmp(on_parser_error))
                return NULL;

        tree_module* module = tree_new_module(c_context_get_tree_context(self->context));
        c_sema_enter_module(&self->sema, module);
        c_parser_set_on_error(&self->parser, on_parser_error);
        c_parser_enter_token_stream(&self->parser);
        return c_parse_module(&self->parser);
}