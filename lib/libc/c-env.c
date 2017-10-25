#include "c-env.h"
#include "c-info.h"
#include "c-lexer.h"
#include "c-parse-module.h"
#include "c-prog.h"

extern void cenv_init(cenv* self, FILE* err)
{
        ctree_context_init(&self->context);
        csource_manager_init(&self->source_manager, &self->context);
        cerror_manager_init(&self->error_manager, &self->source_manager, err);
}

extern void cenv_dispose(cenv* self)
{
        csource_manager_dispose(&self->source_manager);
        ctree_context_dispose(&self->context);
}

extern serrcode cenv_add_lookup(cenv* self, const char* dir)
{
        return csource_manager_add_lookup(&self->source_manager, dir);
}

static csource* cenv_open_file(cenv* self, const char* file)
{
        csource* source = csource_find(&self->source_manager, file);
        if (!source)
        {
                cerror(&self->error_manager, CES_FATAL_ERROR, -1,
                        "cannot open source file %s", path_get_cfile(file));
                return NULL;
        }
        return source;
}

extern tree_module* cparse_source(cenv* self, csource* source, tree_platform_info* platform)
{
        if (!source)
                return NULL;

        tree_module* m = tree_new_module(ctree_context_base(&self->context), platform);

        clexer  lexer;
        cprog   prog;
        cparser parser;

        clexer_init(&lexer, &self->source_manager, &self->error_manager, &self->context);
        cprog_init(&prog, &self->context, m, &self->error_manager);
        cparser_init(&parser, &lexer, &prog, &self->error_manager);

        bool failed = false;
        if (S_FAILED(clexer_init_reswords(&lexer)))
                failed = true;
        if (S_FAILED(clexer_enter_source_file(&lexer, source)))
                failed = true;

        jmp_buf on_parser_error;
        if (setjmp((int*)&on_parser_error))
                failed = true;
        else
        {
                cparser_set_on_error(&parser, (int*)&on_parser_error);
                cparser_enter_token_stream(&parser);
        }

        if (!failed)
                m = cparse_module(&parser);
        
        clexer_dispose(&lexer);
        cprog_dispose(&prog);
        cparser_dispose(&parser);

        return m;
}

extern tree_module* cparse_file(cenv* self, const char* file, tree_platform_info* platform)
{
        return cparse_source(self, cenv_open_file(self, file), platform);
}

extern tree_module* cparse_string(cenv* self, const char* str, tree_platform_info* platform)
{
        return cparse_source(self, csource_emulate(&self->source_manager, "", str), platform);
}

extern serrcode clex_source(cenv* self, csource* source, objgroup* result)
{
        if (!source)
                return false;

        clexer lexer;
        clexer_init(&lexer, &self->source_manager, &self->error_manager, &self->context);

        bool failed = false;
        if (S_FAILED(clexer_init_reswords(&lexer)))
                failed = true;
        if (S_FAILED(clexer_enter_source_file(&lexer, source)))
                failed = true;

        while (!failed)
        {
                ctoken* t = clex(&lexer);
                if (!t || S_FAILED(objgroup_push_back(result, t)))
                {
                        failed = true;
                        break;
                }

                if (ctoken_is(t, CTK_EOF))
                        break;
        }
        clexer_dispose(&lexer);
        return failed ? S_ERROR : S_NO_ERROR;
}

extern serrcode clex_file(cenv* self, const char* file, objgroup* result)
{
        return clex_source(self, cenv_open_file(self, file), result);
}

extern serrcode clex_string(cenv* self, const char* str, objgroup* result)
{
        return clex_source(self, csource_emulate(&self->source_manager, "", str), result);
}