#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-info.h"
#include "scc/scl/char-info.h"

extern void creswords_init(creswords* self, ccontext* context)
{
        strmap_init_alloc(&self->reswords, cget_alloc(context));
        strmap_init_alloc(&self->pp_reswords, cget_alloc(context));
}

extern void creswords_dispose(creswords* self)
{
        strmap_dispose(&self->reswords);
        strmap_dispose(&self->pp_reswords);
}

extern void creswords_add(creswords* self, const char* string, ctoken_kind k)
{
        strref r = STRREF(string);
        strmap_insert(&self->reswords, r, (void*)k);
}

extern void creswords_add_pp(creswords* self, const char* string, ctoken_kind k)
{
        strmap_insert(&self->pp_reswords, STRREF(string), (void*)k);
}

extern ctoken_kind creswords_get(const creswords* self, const char* string, ssize len)
{
        return creswords_get_by_ref(self, STRREFL(string, len));
}

extern ctoken_kind creswords_get_by_ref(const creswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->reswords, ref, &res)
                ? (ctoken_kind)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}

extern ctoken_kind creswords_get_pp(const creswords* self, const char* string, ssize len)
{
        return creswords_get_pp_by_ref(self, STRREFL(string, len));
}

extern ctoken_kind creswords_get_pp_by_ref(const creswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->pp_reswords, ref, &res)
                ? (ctoken_kind)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}

static inline tree_location cpplexer_loc(const cpplexer* self)
{
        return self->loc;
}

static inline int cpplexer_get_prevc(const cpplexer* self)
{
        return self->chars[0];
}

static inline int cpplexer_getc(const cpplexer* self)
{
        return self->chars[1];
}

static inline int cpplexer_get_nextc(const cpplexer* self)
{
        return self->chars[2];
}

static inline bool cpplexer_at_eof(const cpplexer* self)
{
        return cpplexer_getc(self) == RB_ENDC;
}

static inline bool cpplexer_at_start_of_line(const cpplexer* self)
{
        return cpplexer_get_prevc(self) == '\n';
}

static inline void cpplexer_shift_buf(cpplexer* self, int c)
{
        self->chars[0] = self->chars[1];
        self->chars[1] = self->chars[2];
        self->chars[2] = c;
        self->loc++;
}

static inline int cpplexer_readc(cpplexer* self)
{
        cpplexer_shift_buf(self, readbuf_readc(self->buf));
        if (cpplexer_getc(self) == '\\' && char_is_newline(cpplexer_get_nextc(self)))
        {
                self->chars[1] = readbuf_readc(self->buf);
                self->chars[2] = readbuf_readc(self->buf);
                self->loc += 2;
        }

        if (cpplexer_at_start_of_line(self))
                csource_save_line_loc(self->source, cpplexer_loc(self));

        return cpplexer_getc(self);
}

extern void cpplexer_init(
        cpplexer* self,
        const creswords* reswords,
        csource_manager* source_manager,
        clogger* logger,
        ccontext* context)
{
        self->reswords = reswords;
        self->context = context;
        self->logger = logger;
        self->source = NULL;
        self->source_manager = source_manager;
        self->buf = NULL;
        self->tab_to_space = 4;
        self->chars[0] = RB_ENDC;
        self->chars[1] = RB_ENDC;
        self->chars[2] = RB_ENDC;
        self->loc = TREE_INVALID_LOC;
        self->angle_string_expected = false;
}

extern serrcode cpplexer_enter_source_file(cpplexer* self, csource* source)
{
        if (!source)
                return S_ERROR;
        
        if (!(self->buf = csource_open(source)))
        {
                cerror_cannot_open_source_file(self->logger, 0, csource_get_name(source));
                return S_ERROR;
        }

        // save first line location
        if (S_FAILED(csource_save_line_loc(source, csource_get_loc_begin(source))))
                return S_ERROR;

        // fill buffer
        cpplexer_readc(self);
        cpplexer_readc(self);
        self->loc = csource_get_loc_begin(source);
        self->source = source;
        return S_NO_ERROR;
}

typedef struct 
{
        // size of the sequence including trailing zero
        ssize size;
        tree_location loc;
        char val[CMAX_LINE_LENGTH + 1];
} csequence;

static inline void csequence_finish(csequence* self)
{
        self->val[self->size++] = '\0';
}

static void csequence_init(csequence* self)
{
        self->size = 0;
        self->loc = 0;
        self->val[0] = '\0';
}

static inline bool csequence_append(csequence* self, const cpplexer* lexer, int c)
{
        if (self->size == CMAX_LINE_LENGTH)
        {
                cerror_token_is_too_long(lexer->logger, self->loc);
                return false;
        }

        self->val[self->size++] = (char)c;
        return true;
}

static tree_id cpplexer_pool_seq(cpplexer* self, csequence* seq)
{
        tree_id id = tree_get_id_for_string(cget_tree(self->context), seq->val, seq->size);
        S_ASSERT(id != TREE_INVALID_ID);
        return id;
}

static bool cpplex_word_seq(cpplexer* self, csequence* seq)
{
        seq->loc = cpplexer_loc(self);
        int c = cpplexer_getc(self);
        while (get_char_info(c) & (ACK_ALPHA | ACK_DIGIT))
        {
                if (!csequence_append(seq, self, c))
                        return false;

                c = cpplexer_readc(self);
        }
        csequence_finish(seq);
        return true;
}

extern ctoken* cpplex_identifier(cpplexer* self)
{
        csequence word;
        csequence_init(&word);
        if (!cpplex_word_seq(self, &word))
                return NULL;

        return ctoken_new_id(self->context, word.loc, cpplexer_pool_seq(self, &word));
}

static bool cpplex_quoted_seq(cpplexer* self, csequence* seq, int quote)
{
        seq->loc = cpplexer_loc(self);
        bool consume = false;
        while (1)
        {
                int c = cpplexer_readc(self);
                if (char_is_newline(c) || cpplexer_at_eof(self))
                {
                        cerror_missing_closing_quote(self->logger, seq->loc);
                        return false;
                }
                else if (c == '\\')
                        consume = true;
                else if (c == quote && !consume)
                {
                        cpplexer_readc(self);
                        break;
                }
                else
                        consume = false;

                if (!csequence_append(seq, self, c))
                        return false;
        }
        csequence_finish(seq);
        return true;
}

extern ctoken* cpplex_string_literal(cpplexer* self)
{
        csequence schar;
        csequence_init(&schar);
        if (!cpplex_quoted_seq(self, &schar, '"'))
                return NULL;

        return ctoken_new_string(self->context, schar.loc, cpplexer_pool_seq(self, &schar));
}

extern ctoken* cpplex_angle_string_literal(cpplexer* self)
{
        csequence seq;
        csequence_init(&seq);
        if (!cpplex_quoted_seq(self, &seq, '>'))
                return NULL;

        return ctoken_new_angle_string(self->context, seq.loc, cpplexer_pool_seq(self, &seq));
}

extern ctoken* cpplex_const_char(cpplexer* self)
{
        csequence cchar;
        csequence_init(&cchar);
        if (!cpplex_quoted_seq(self, &cchar, '\''))
                return NULL;

        bool escape = cchar.val[0] == '\\';
        if (cchar.size == 1)
        {
                cerror_empty_character_constant(self->logger, cchar.loc);
                return NULL;
        }
        else if ((escape && cchar.size > 3) || (!escape && cchar.size > 2))
        {
                cerror_invalid_character_constant(self->logger, cchar.loc);
                return NULL;
        }

        int c = escape ? char_to_escape(cchar.val[1]) : cchar.val[0];
        return ctoken_new_char(self->context, cchar.loc, c);
}

static bool cpplex_num_seq(cpplexer* self, csequence* seq)
{
        seq->loc = cpplexer_loc(self);
        int c = cpplexer_getc(self);
        while(1)
        {
                bool pp = (get_char_info(c) & (ACK_DIGIT | ACK_ALPHA))
                        || c == '.' || c == '+' || c == '-';
                if (!pp)
                {
                        csequence_finish(seq);
                        return true;
                }

                if (!csequence_append(seq, self, c))
                        return false;

                c = cpplexer_readc(self);
        }
}

extern ctoken* cpplex_number(cpplexer* self)
{
        csequence num;
        csequence_init(&num);
        if (!cpplex_num_seq(self, &num))
                return NULL;

        return ctoken_new_pp_num(self->context, num.loc, cpplexer_pool_seq(self, &num));
}

static ctoken* cpplex_comment(cpplexer* self)
{
        tree_location loc = cpplexer_loc(self);
        int c = cpplexer_readc(self); // '/'
        if (c == '/')
        {
                while (!char_is_newline(c) && !cpplexer_at_eof(self))
                        c = cpplexer_readc(self);
        }
        else
        {
                while (1)
                {
                        c = cpplexer_readc(self);
                        if (c == '/' && cpplexer_get_prevc(self) == '*')
                        {
                                cpplexer_readc(self); // '/'
                                break;
                        }
                        else if (cpplexer_at_eof(self))
                        {
                                cerror_unclosed_comment(self->logger, loc);
                                return NULL;
                        }
                }
        }

        return ctoken_new(self->context, CTK_COMMENT, loc);
}

static inline ctoken_kind _cpplex_punctuator(cpplexer* self)
{
        switch ((char)cpplexer_getc(self))
        {
                case '{': cpplexer_readc(self); return CTK_LBRACE;
                case '}': cpplexer_readc(self); return CTK_RBRACE;
                case '[': cpplexer_readc(self); return CTK_LSBRACKET;
                case ']': cpplexer_readc(self); return CTK_RSBRACKET;
                case '(': cpplexer_readc(self); return CTK_LBRACKET;
                case ')': cpplexer_readc(self); return CTK_RBRACKET;
                case ':': cpplexer_readc(self); return CTK_COLON;
                case '?': cpplexer_readc(self); return CTK_QUESTION;
                case ',': cpplexer_readc(self); return CTK_COMMA;
                case '~': cpplexer_readc(self); return CTK_TILDE;
                case ';': cpplexer_readc(self); return CTK_SEMICOLON;

                case '.':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '.')
                        {
                                if (cpplexer_get_nextc(self) == '.')
                                {
                                        cpplexer_readc(self);
                                        cpplexer_readc(self);
                                        return CTK_ELLIPSIS;
                                }
                        }
                        return CTK_DOT;

                case '-':
                        cpplexer_readc(self);
                        switch (cpplexer_getc(self))
                        {
                                case '>': cpplexer_readc(self); return CTK_ARROW;
                                case '=': cpplexer_readc(self); return CTK_MINUS_EQ;
                                case '-': cpplexer_readc(self); return CTK_MINUS2;
                                default: return CTK_MINUS;
                        }

                case '<':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '<')
                        {
                                cpplexer_readc(self);
                                if (cpplexer_getc(self) == '=')
                                {
                                        cpplexer_readc(self);
                                        return CTK_LE2_EQ;
                                }
                                return CTK_LE2;
                        }
                        else if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_LEQ;
                        }
                        return CTK_LE;

                case '>':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '>')
                        {
                                cpplexer_readc(self);
                                if (cpplexer_getc(self) == '=')
                                {
                                        cpplexer_readc(self);
                                        return CTK_GR2_EQ;
                                }
                                return CTK_GR2;
                        }
                        else if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_GREQ;
                        }
                        return CTK_GR;

                case '!':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_EXCLAIM_EQ;
                        }
                        return CTK_EXCLAIM;

                case '=':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_EQ2;
                        }
                        return CTK_EQ;

                case '&':
                        cpplexer_readc(self);
                        switch (cpplexer_getc(self))
                        {
                                case '&': cpplexer_readc(self); return CTK_AMP2;
                                case '=': cpplexer_readc(self); return CTK_AMP_EQ;
                                default: return CTK_AMP;
                        }

                case '|':
                        cpplexer_readc(self);
                        switch (cpplexer_getc(self))
                        {
                                case '|': cpplexer_readc(self); return CTK_VBAR2;
                                case '=': cpplexer_readc(self); return CTK_VBAR_EQ;
                                default: return CTK_VBAR;
                        }

                case '^':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_CARET_EQ;
                        }
                        return CTK_CARET;

                case '+':
                        cpplexer_readc(self);
                        switch (cpplexer_getc(self))
                        {
                                case '+': cpplexer_readc(self); return CTK_PLUS2;
                                case '=': cpplexer_readc(self); return CTK_PLUS_EQ;
                                default: return CTK_PLUS;
                        }

                case '*':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_STAR_EQ;
                        }
                        return CTK_STAR;

                case '/':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_SLASH_EQ;
                        }
                        return CTK_SLASH;

                case '%':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_PERCENT_EQ;
                        }
                        return CTK_PERCENT;

                case '#':
                        cpplexer_readc(self);
                        if (cpplexer_getc(self) == '#')
                        {
                                cpplexer_readc(self);
                                return CTK_HASH2;
                        }
                        return CTK_HASH;

                default:
                        return CTK_UNKNOWN;
        }
}

static ctoken* cpplex_punctuator(cpplexer* self)
{
        tree_location loc = cpplexer_loc(self);
        ctoken_kind k = _cpplex_punctuator(self);
        if (k == CTK_UNKNOWN)
        {
                cerror_unknown_punctuator(self->logger, loc, cpplexer_getc(self));
                return NULL;
        }
        return ctoken_new(self->context, k, loc);
}

static ctoken* cpplex_symbols(cpplexer* self)
{
        int c = cpplexer_getc(self);
        if (c == '\"')
                return cpplex_string_literal(self);
        else if (c == '\'')
                return cpplex_const_char(self);
        else if (c == '<' && self->angle_string_expected)
                return cpplex_angle_string_literal(self);

        int next = cpplexer_get_nextc(self);
        if (c == '.' && char_is_digit(next))
                return cpplex_number(self);
        else if (c == '/' && (next == '/' || next == '*'))
                return cpplex_comment(self);

        return cpplex_punctuator(self);
}

static ctoken* cpplex_wspace(cpplexer* self)
{
        tree_location loc = cpplexer_loc(self);
        int spaces = 0;
        int c = cpplexer_getc(self);
        while (char_is_wspace(c) && !cpplexer_at_eof(self))
        {
                spaces += c == '\t' ? self->tab_to_space : 1;
                c = cpplexer_readc(self);
        }
        return ctoken_new_wspace(self->context, loc, spaces);
}

extern ctoken* cpplex_token(cpplexer* self)
{
        int c = cpplexer_getc(self);
        int i = get_char_info(c);

        if (i & ACK_ALPHA)
                return cpplex_identifier(self);
        else if (i & ACK_SYMBOL)
                return cpplex_symbols(self);
        else if (i & ACK_DIGIT)
                return cpplex_number(self);
        else if (i & ACK_WSPACE)
                return cpplex_wspace(self);
        else if (i & ACK_NEWLINE)
        {
                tree_location loc = cpplexer_loc(self);
                cpplexer_readc(self);
                return ctoken_new(self->context, CTK_EOL, loc);

        }
        else if (cpplexer_at_eof(self))
        {
                tree_location loc = cpplexer_loc(self);
                cpplexer_readc(self);
                return ctoken_new(self->context, CTK_EOF, loc);
        }

        cerror_unknown_symbol(self->logger, cpplexer_loc(self), c);
        return NULL;
}

extern void cpproc_init(
        cpproc* self,
        const creswords* reswords,
        csource_manager* source_manager,
        clogger* logger,
        ccontext* context)
{
        self->reswords = reswords;
        self->state = self->files - 1;
        self->source_manager = source_manager;
        self->logger = logger;
        self->context = context;
        dseq_init_alloc(&self->expansion, cget_alloc(context));
}

extern void cpproc_dispose(cpproc* self)
{
        while (self->state != self->files - 1)
        {
                csource_close(self->state->source);
                self->state--;
        }
}

extern serrcode cpproc_enter_source_file(cpproc* self, csource* source)
{
        if (!source)
                return S_ERROR;
        if (self->state - self->files >= CMAX_INCLUDE_NESTING)
        {
                cerror_too_deep_include_nesting(self->logger);
                return S_ERROR;
        }

        cpproc_state* next = self->state + 1;
        cpplexer* pplexer = &next->lexer;
        cpplexer_init(
                pplexer,
                self->reswords,
                self->source_manager,
                self->logger, 
                self->context);
        if (S_FAILED(cpplexer_enter_source_file(pplexer, source)))
                return S_ERROR;

        self->state = next;
        self->state->source = source;
        self->state->nhash = 0;
        self->state->hash_expected = true;
        self->state->in_directive = false;
        return S_NO_ERROR;
}

extern void cpproc_exit_source_file(cpproc* self)
{
        S_ASSERT(self->state != self->files);
        csource_close(self->state->source);
        self->state--;
}

static ctoken* cpproc_lex_wspace(cpproc* self, bool skip_eol)
{
        while (1)
        {
                ctoken* t = cpplex_token(&self->state->lexer);
                if (!t)
                        return NULL;

                ctoken_kind k = ctoken_get_kind(t);
                if (k == CTK_WSPACE || k == CTK_COMMENT || (k == CTK_EOL && skip_eol))
                        continue;
                else if (k == CTK_EOF && self->state != self->files)
                {
                        // consume eof of included source file
                        cpproc_exit_source_file(self);
                        continue;
                }

                return t;
        }
}

static ctoken* cpproc_lex_directive(cpproc*);

static ctoken* cpproc_lex_include(cpproc* self)
{
        self->state->lexer.angle_string_expected = true;
        ctoken* t = cpproc_lex_wspace(self, false);
        self->state->lexer.angle_string_expected = false;
        if (!t)
                return NULL;

        tree_location token_loc = ctoken_get_loc(t);
        if (!ctoken_is(t, CTK_CONST_STRING) && !ctoken_is(t, CTK_ANGLE_STRING))
        {
                cerror_expected_file_name(self->logger, token_loc);
                return NULL;
        }

        tree_id ref = ctoken_get_string(t);
        const char* filename = tree_get_id_string(cget_tree(self->context), ref);
        S_ASSERT(filename);

        if (!*filename)
        {
                cerror_empty_file_name_in_include(self->logger, token_loc);
                return NULL;
        }

        csource* source = csource_find(self->source_manager, filename);
        if (!source)
        {
                cerror_cannot_open_source_file(self->logger, token_loc, filename);
                return NULL;
        }

        if (S_FAILED(cpproc_enter_source_file(self, source)))
                return NULL;

        return cpproc_lex_directive(self);
}

static ctoken* cpproc_lex_directive(cpproc* self)
{
        ctoken* t = cpproc_lex_wspace(self, true);
        if (!t || !ctoken_is(t, CTK_HASH))
                return t;

        if (!self->state->hash_expected)
        {
                cerror_unexpected_hash(self->logger, ctoken_get_loc(t));
                return NULL;
        }

        if (!(t = cpproc_lex_wspace(self, false)))
                return NULL;

        ctoken_kind directive = CTK_UNKNOWN;
        if (ctoken_is(t, CTK_ID))
                directive = creswords_get_pp_by_ref(self->reswords, ctoken_get_string(t));

        if (directive == CTK_UNKNOWN)
        {
                cerror_unknown_preprocessor_directive(self->logger, ctoken_get_loc(t));
                return false;
        }

        self->state->in_directive = true;
        if (directive == CTK_PP_INCLUDE)
                t = cpproc_lex_include(self);
        else
        {
                t = NULL;
                cerror_unsupported_preprocessor_directive(self->logger, ctoken_get_loc(t));
        }
        self->state->in_directive = false;
        return t;
}

static ctoken* cpproc_lex_macro_id(cpproc* self)
{
        if (dseq_size(&self->expansion))
        {
                ctoken* last = *(dseq_end(&self->expansion) - 1);
                dseq_resize(&self->expansion, dseq_size(&self->expansion) - 1);
                return last;
        }

        ctoken* t = cpproc_lex_directive(self);
        if (!t)
                return NULL;

        if (!ctoken_is(t, CTK_ID))
                return t;

        if (0) // token is macro id
        {
                // push back replacement
                // ...
                // return (ctoken*)list_pop_front(&self->expansion);
        }
        return t;
}

static inline void cpproc_unget_macro_id(cpproc* self, ctoken* t)
{
        dseq_append(&self->expansion, t);
}

static bool cpproc_collect_adjacent_strings(cpproc* self, dseq* result)
{
        while (1)
        {
                ctoken* t = cpproc_lex_macro_id(self);
                if (!t)
                        return false;

                if (!ctoken_is(t, CTK_CONST_STRING))
                {
                        cpproc_unget_macro_id(self, t);
                        return true;
                }

                dseq_append(result, t);
        }
}

static ctoken* cpproc_concat_and_escape_strings(cpproc* self, dseq* strings)
{
        S_ASSERT(dseq_size(strings));

        dseq_u8 concat;
        dseq_u8_init_alloc(&concat, cget_alloc(self->context));
        for (ssize i = 0; i < dseq_size(strings); i++)
        {
                ctoken* t = dseq_get(strings, i);
                const char* string = tree_get_id_string(
                        cget_tree(self->context), ctoken_get_string(t));

                char escaped[CMAX_LINE_LENGTH + 1];
                ssize size = cget_escaped_string(escaped, string, strlen(string));
                for (ssize j = 0; j < size - 1; j++)
                        dseq_u8_append(&concat, escaped[j]);
        }
        dseq_u8_append(&concat, '\0');

        tree_id concat_ref = tree_get_id_for_string(
                cget_tree(self->context), (char*)dseq_u8_begin(&concat), dseq_u8_size(&concat));
        tree_location loc = ctoken_get_loc(dseq_get(strings, 0));
        dseq_u8_dispose(&concat);

        return ctoken_new_string(self->context, loc, concat_ref);
}

static ctoken* cpproc_lex_string(cpproc* self)
{
        ctoken* t = cpproc_lex_macro_id(self);
        if (!t)
                return NULL;

        if (!ctoken_is(t, CTK_CONST_STRING))
                return t;

        dseq adjacent_strings;
        dseq_init_alloc(&adjacent_strings, cget_alloc(self->context));
        dseq_append(&adjacent_strings, t);

        ctoken* result = cpproc_collect_adjacent_strings(self, &adjacent_strings)
                ? cpproc_concat_and_escape_strings(self, &adjacent_strings)
                : NULL;

        dseq_dispose(&adjacent_strings);
        return result;
}

extern ctoken* cpreprocess(cpproc* self)
{
        return cpproc_lex_string(self);
}