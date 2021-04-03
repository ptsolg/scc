#include "scc/lex/preprocessor.h"
#include "scc/c-common/context.h"
#include "scc/c-common/source.h"
#include "scc/c-common/limits.h"
#include "scc/core/hashmap.h"
#include "scc/lex/pragma.h"
#include "scc/lex/preprocessor-lexer.h"
#include "scc/lex/token.h"
#include "scc/lex/reswords.h"
#include "scc/lex/misc.h"
#include "scc/tree/context.h"
#include "scc/tree/target.h"
#include "preprocessor-directive.h"
#include "macro.h"
#include "errors.h"
#include <time.h>

static c_macro* _c_preprocessor_init_builtin_macro(
        c_preprocessor* self, const char* name, c_token_kind token_kind, const char* string)
{
        c_macro* m = c_macro_new(self->context, true, false, TREE_INVALID_LOC,
                tree_get_id_for_string(self->context->tree, name));
        bool r = c_preprocessor_define_macro(self, m);
        assert(r);
        c_token* t = c_token_new_ex(self->context, token_kind, TREE_INVALID_LOC, sizeof(c_token));
        c_token_set_string(t, tree_get_id_for_string(self->context->tree, string));
        c_macro_add_token(m, self->context, t);
        return m;
}

static void c_preprocessor_set_file(c_preprocessor* self, const c_source* source)
{
        char file[MAX_PATH_LEN * 2];
        file[0] = '\0';
        if (source)
        {
                const char* path = source->file->path;
                c_get_unescaped_string(file, ARRAY_SIZE(file), path, strlen(path) + 1);
        }

        c_token* t = c_macro_get_token(self->builtin_macro.file, 0);
        c_token_set_string(t, tree_get_id_for_string(self->context->tree, file));
}

static void c_preprocessor_update_line(c_preprocessor* self)
{
        c_pp_lexer* lexer = c_lexer_stack_get(&self->lexer_stack, self->token_lexer_depth);
        char num[100];
        snprintf(num, ARRAY_SIZE(num), "%d", lexer->token_lexer.line);
        c_token* t = c_macro_get_token(self->builtin_macro.line, 0);
        c_token_set_string(t, tree_get_id_for_string(self->context->tree, num));
}

static void c_preprocessor_init_builtin_macro(c_preprocessor* self)
{
        static const char months[][4] = {
                "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        time_t raw_time;
        time(&raw_time);
        struct tm* lt = localtime(&raw_time);
        char time[100];
        snprintf(time, ARRAY_SIZE(time), "%s %d %d", months[lt->tm_mon], lt->tm_mday, 1900 + lt->tm_year);
        _c_preprocessor_init_builtin_macro(self, "__DATE__", CTK_CONST_STRING, time);

        snprintf(time, ARRAY_SIZE(time), "%d:%d:%d", lt->tm_hour, lt->tm_min, lt->tm_sec);
        _c_preprocessor_init_builtin_macro(self, "__TIME__", CTK_CONST_STRING, time);

        self->builtin_macro.file = _c_preprocessor_init_builtin_macro(self, "__FILE__", CTK_CONST_STRING, "");
        self->builtin_macro.line = _c_preprocessor_init_builtin_macro(self, "__LINE__", CTK_PP_NUM, "1");
        _c_preprocessor_init_builtin_macro(self, "__STDC__", CTK_PP_NUM, "1");
        _c_preprocessor_init_builtin_macro(self, "__STDC_HOSTED__", CTK_PP_NUM, "1");
        _c_preprocessor_init_builtin_macro(self, "__STDC_MB_MIGHT_NEQ_WC__", CTK_PP_NUM, "1");
        _c_preprocessor_init_builtin_macro(self, "__STDC_VERSION__", CTK_PP_NUM, "199901L");
        
        _c_preprocessor_init_builtin_macro(self, "__SCC__", CTK_PP_NUM, "1");

        if (self->context->tree->target->kind == TTAK_X86_32) {
                _c_preprocessor_init_builtin_macro(self, "_WIN32", CTK_PP_NUM, "1");
                _c_preprocessor_init_builtin_macro(self, "_M32", CTK_PP_NUM, "1");
        }
        else {
                _c_preprocessor_init_builtin_macro(self, "_WIN64", CTK_PP_NUM, "1");
                _c_preprocessor_init_builtin_macro(self, "_M64", CTK_PP_NUM, "1");
        }       
}

extern void c_preprocessor_init(
        c_preprocessor* self, const c_reswords* reswords, c_context* context)
{
        self->lexer = NULL;
        self->token_lexer_depth = -1;
        c_init_lexer_stack(&self->lexer_stack, context);
        self->lookahead.next_unexpanded_token = NULL;
        self->lookahead.next_expanded_token = NULL;
        hashmap_init(&self->macro_lookup);
        self->reswords = reswords;
        self->context = context;
        self->id.defined = tree_get_id_for_string(self->context->tree, "defined");
        self->id.link = tree_get_id_for_string(self->context->tree, "link");
        c_preprocessor_init_builtin_macro(self);
        c_pragma_handlers_init(&self->pragma_handlers, 0);
}

extern void c_preprocessor_dispose(c_preprocessor* self)
{
        while (c_lexer_stack_depth(&self->lexer_stack))
                c_preprocessor_exit(self);
        c_dispose_lexer_stack(&self->lexer_stack);
        hashmap_drop(&self->macro_lookup);
}

extern errcode c_preprocessor_enter_source(c_preprocessor* self, c_source* source)
{
        assert(source);

        self->lexer = c_push_token_lexer(
                &self->lexer_stack, self->context);
        c_preprocessor_set_file(self, source);
        self->token_lexer_depth = c_lexer_stack_depth(&self->lexer_stack) - 1;
        return c_token_lexer_enter(&self->lexer->token_lexer, source);
}

static void c_preprocessor_enter_macro(
        c_preprocessor* self, c_macro* macro, tree_location loc)
{
        assert(macro);
        macro->used = true;
        self->lexer = c_push_macro_lexer(
                &self->lexer_stack, self->context, macro, loc);
}

extern void c_preprocessor_exit(c_preprocessor* self)
{
        if (self->lexer->kind == CPLK_MACRO)
                self->lexer->macro_lexer.macro->used = false;
        else if (self->lexer->kind != CPLK_TOKEN)
                UNREACHABLE();

        c_pop_lexer(&self->lexer_stack);
        if (!c_lexer_stack_depth(&self->lexer_stack))
        {
                self->lexer = NULL;
                return;
        }

        self->lexer = c_lexer_stack_top(&self->lexer_stack);
        if (self->lexer->kind == CPLK_TOKEN)
        {
                c_preprocessor_set_file(self, self->lexer->token_lexer.source);
                self->token_lexer_depth = c_lexer_stack_depth(&self->lexer_stack) - 1;
        }
}

extern c_macro* c_preprocessor_get_macro(const c_preprocessor* self, tree_id name)
{
        struct hashmap_entry* entry = hashmap_lookup(&self->macro_lookup, name);
        return entry ? entry->value : NULL;
}

extern bool c_preprocessor_define_macro(c_preprocessor* self, c_macro* macro)
{
        if (c_preprocessor_macro_defined(self, macro->name))
        {
                c_error_macro_redefenition(self->context, macro->name, macro->loc);
                return false;
        }
        hashmap_insert(&self->macro_lookup, macro->name, macro);
        return true;
}

extern bool c_preprocessor_macro_defined(const c_preprocessor* self, tree_id name)
{
        return c_preprocessor_get_macro(self, name) != NULL;
}

extern bool c_preprocessor_undef(c_preprocessor* self, tree_id name)
{
        return hashmap_erase(&self->macro_lookup, name);
}

extern c_token* c_preprocess_non_comment(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_pp_lex(self->lexer);
                if (!t)
                        return NULL;

                if (c_token_is(t, CTK_COMMENT))
                        return c_token_new_wspace(self->context, c_token_get_loc(t), 1);

                if (!c_token_is(t, CTK_EOF))
                {
                        if (c_token_is(t, CTK_EOL) || c_token_is(t, CTK_EOD))
                                c_preprocessor_update_line(self);
                        return t;
                }

                if (c_cond_stack_depth(self->lexer))
                {
                        c_error_unterminated_directive(self->context,
                                c_get_cond_directive(self->lexer)->token);
                        return NULL;
                }
                if (c_lexer_stack_depth(&self->lexer_stack) > 1)
                {
                        c_preprocessor_exit(self);
                        continue; // consume eof of included file
                }

                return t;
        }
}

extern c_token* c_preprocess_non_wspace(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_preprocess_non_comment(self);
                if (!t)
                        return NULL;

                if (c_token_is(t, CTK_WSPACE) || c_token_is(t, CTK_EOL))
                        continue;

                return t;
        }
}

extern c_token* c_preprocess_non_directive(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_preprocess_non_wspace(self);
                if (!t || !c_token_is(t, CTK_HASH))
                        return t;

                // this happens when we get '#' from macro e.g:
                //      #define A #
                //      A
                if (self->lexer->kind != CPLK_TOKEN)
                {
                        c_error_stray_symbol(self->context, c_token_get_loc(t), '#');
                        return NULL;
                }

                self->lexer->token_lexer.in_directive = true;
                if (!(t = c_preprocess_non_wspace(self)))
                        return NULL;
 
                if (c_token_is(t, CTK_ID))
                {
                        c_token_kind directive = c_reswords_get_pp_resword_by_ref(
                                self->reswords, c_token_get_string(t));
                        c_token_set_kind(t, directive);
                }
                if (!c_preprocessor_handle_directive(self, t))
                        return NULL;
        }
}

typedef struct
{
        size_t num_args;
        size_t num_params;
        c_macro_args* args;
        c_macro* macro;
        tree_location loc;
} c_preprocessing_args;

static void c_preprocessing_args_init(
        c_preprocessing_args* self, c_macro* macro, c_macro_args* args, tree_location loc)
{
        self->num_args = 0;
        self->num_params = c_macro_get_params_size(macro);
        self->args = args;
        self->macro = macro;
        self->loc = loc;
}

static bool c_preprocessor_check_macro_args_overflow(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        if (pp_args->num_args > pp_args->num_params)
        {
                c_error_macro_argument_list_overflow(
                        self->context, pp_args->macro, pp_args->num_args, pp_args->loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_check_macro_args_underflow(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        if (pp_args->num_args < pp_args->num_params)
        {
                c_error_macro_argument_list_underflow(
                        self->context, pp_args->macro, pp_args->num_args, pp_args->loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_add_macro_arg(c_preprocessor* self, c_preprocessing_args* pp_args, c_token* arg)
{
        if (!c_preprocessor_check_macro_args_overflow(self, pp_args))
                return false;

        tree_id arg_name = c_macro_get_param(pp_args->macro, pp_args->num_args - 1);
        c_macro_args_add(pp_args->args, arg_name, arg);
        return true;
}

static bool c_preprocessor_append_empty_macro_arg(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        pp_args->num_args++;
        if (!c_preprocessor_check_macro_args_overflow(self, pp_args))
                return false;

        tree_id arg_name = c_macro_get_param(pp_args->macro, pp_args->num_args - 1);
        c_macro_args_set_empty(pp_args->args, arg_name);
        return true;
}

static bool c_preprocessor_read_macro_args(
        c_preprocessor* self, c_macro* macro, c_macro_args* args, tree_location loc)
{
        // true if the argument is empty e.g:
        //      #define A(a)
        //      A()
        bool empty_arg = true;
        int bracket_nesting = 1; // '(' already consumed

        c_preprocessing_args pp_args;
        c_preprocessing_args_init(&pp_args, macro, args, loc);
        while (1)
        {
                c_token* t = c_preprocess_non_comment(self);
                if (!t)
                        return false;

                if (c_token_is(t, CTK_EOF))
                {
                        c_error_unterminated_macro_argument_list(self->context, macro, loc);
                        return false;
                }
                else if (c_token_is(t, CTK_LBRACKET))
                        bracket_nesting++;
                else if (c_token_is(t, CTK_RBRACKET))
                {
                        bracket_nesting--;
                        if (bracket_nesting == 0)
                        {
                                if (empty_arg && !c_preprocessor_append_empty_macro_arg(self, &pp_args))
                                        return false;
                                return c_preprocessor_check_macro_args_underflow(self, &pp_args);
                        }
                }
                else if (c_token_is(t, CTK_COMMA) && bracket_nesting == 1)
                {
                        if (empty_arg && !c_preprocessor_append_empty_macro_arg(self, &pp_args))
                                return false;
                        empty_arg = true;
                        continue;
                }

                if (empty_arg)
                        pp_args.num_args++;
                if (!c_preprocessor_add_macro_arg(self, &pp_args, t))
                        return false;
                empty_arg = false;
        }
}

extern c_token* c_preprocess_non_macro(c_preprocessor* self)
{
        while (1)
        {
                c_token* t;
                if ((t = self->lookahead.next_unexpanded_token))
                        self->lookahead.next_unexpanded_token = NULL;
                else if (!(t = c_preprocess_non_directive(self)))
                        return NULL;

                if (c_token_is(t, CTK_EOM))
                {
                        c_preprocessor_exit(self);
                        continue;
                }
                else if (!c_token_is(t, CTK_ID))
                        return t;

                tree_id id = c_token_get_string(t);
                c_macro* macro = c_preprocessor_get_macro(self, id);
                if (!macro || macro->used)
                        return t;

                // start macro expansion
                tree_location loc = c_token_get_loc(t);
                c_macro_args args;
                c_macro_args_init(&args, self->context);
                if (macro->function_like)
                {
                        c_token* next = c_preprocess_non_wspace(self);
                        if (!next)
                                return NULL;

                        if (!c_token_is(next, CTK_LBRACKET))
                        {
                                self->lookahead.next_unexpanded_token = next;
                                return t;
                        }
                        if (!c_preprocessor_read_macro_args(self, macro, &args, loc))
                        {
                                c_macro_args_dispose(&args);
                                return NULL;
                        }
                }

                c_preprocessor_enter_macro(self, macro, loc);
                bool failed = EC_FAILED(c_macro_lexer_substitute_macro_args(
                        &self->lexer->macro_lexer, &args));
                c_macro_args_dispose(&args);
                if (failed)
                        return NULL;
        }
}

static bool c_preprocessor_collect_adjacent_strings(c_preprocessor* self, struct vec* result)
{
        while (1)
        {
                c_token* t = c_preprocess_non_macro(self);
                if (!t)
                        return false;

                if (!c_token_is(t, CTK_CONST_STRING))
                {
                        self->lookahead.next_expanded_token = t;
                        return true;
                }

                vec_push(result, t);
        }
}

#define VEC   i8vec
#define VEC_T int8_t
#include "scc/core/vec.inc"

static c_token* c_preprocessor_concat_and_escape_strings(c_preprocessor* self, struct vec* strings)
{
        assert(strings->size);

        struct i8vec concat;
        i8vec_init(&concat);
        for (size_t i = 0; i < strings->size; i++)
        {
                c_token* t = vec_get(strings, i);
                const char* string = tree_get_id_string(self->context->tree, c_token_get_string(t));
                char escaped[C_MAX_LINE_LENGTH + 1];
                size_t size = c_get_escaped_string(escaped, ARRAY_SIZE(escaped), string, strlen(string) + 1);
                for (size_t j = 0; j < size - 1; j++)
                        i8vec_push(&concat, escaped[j]);
        }
        i8vec_push(&concat, '\0');

        tree_id concat_ref = tree_get_id_for_string_s(self->context->tree,
                (char*)i8vec_begin(&concat), concat.size);
        tree_location loc = c_token_get_loc(vec_get(strings, 0));
        i8vec_drop(&concat);

        return c_token_new_string(self->context, loc, concat_ref);
}

extern c_token* c_preprocess(c_preprocessor* self)
{
        c_token* t;
        if ((t = self->lookahead.next_expanded_token))
                self->lookahead.next_expanded_token = NULL;
        else if (!(t = c_preprocess_non_macro(self)))
                return NULL;

        if (!c_token_is(t, CTK_CONST_STRING))
                return t;

        struct vec adjacent_strings;
        vec_init(&adjacent_strings);
        vec_push(&adjacent_strings, t);

        c_token* result = c_preprocessor_collect_adjacent_strings(self, &adjacent_strings)
                ? c_preprocessor_concat_and_escape_strings(self, &adjacent_strings)
                : NULL;

        vec_drop(&adjacent_strings);
        return result;
}
