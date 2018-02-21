#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-info.h"
#include "scc/scl/char-info.h"

extern void c_reswords_init(c_reswords* self, c_context* context)
{
        strmap_init_alloc(&self->reswords, c_context_get_allocator(context));
        strmap_init_alloc(&self->pp_reswords, c_context_get_allocator(context));
}

extern void c_reswords_dispose(c_reswords* self)
{
        strmap_dispose(&self->reswords);
        strmap_dispose(&self->pp_reswords);
}

extern void c_reswords_add(c_reswords* self, const char* string, int kind)
{
        strref r = STRREF(string);
        strmap_insert(&self->reswords, r, (void*)kind);
}

extern void c_reswords_add_pp(c_reswords* self, const char* string, int kind)
{
        strmap_insert(&self->pp_reswords, STRREF(string), (void*)kind);
}

extern int c_reswords_get(const c_reswords* self, const char* string, ssize len)
{
        return c_reswords_get_by_ref(self, STRREFL(string, len));
}

extern int c_reswords_get_by_ref(const c_reswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->reswords, ref, &res)
                ? (int)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}

extern int c_reswords_get_pp(const c_reswords* self, const char* string, ssize len)
{
        return c_reswords_get_pp_by_ref(self, STRREFL(string, len));
}

extern int c_reswords_get_pp_by_ref(const c_reswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->pp_reswords, ref, &res)
                ? (c_token_kind)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}

static inline tree_location c_pplexer_loc(const c_pplexer* self)
{
        return self->loc;
}

static inline int c_pplexer_get_prevc(const c_pplexer* self)
{
        return self->chars[0];
}

static inline int c_pplexer_getc(const c_pplexer* self)
{
        return self->chars[1];
}

static inline int c_pplexer_get_nextc(const c_pplexer* self)
{
        return self->chars[2];
}

static inline bool c_pplexer_at_eof(const c_pplexer* self)
{
        return c_pplexer_getc(self) == RB_ENDC;
}

static inline bool c_pplexer_at_start_of_line(const c_pplexer* self)
{
        return c_pplexer_get_prevc(self) == '\n';
}

static inline void c_pplexer_shift_buf(c_pplexer* self, int c)
{
        self->chars[0] = self->chars[1];
        self->chars[1] = self->chars[2];
        self->chars[2] = c;
        self->loc++;
}

static inline int cpplexer_readc(c_pplexer* self)
{
        c_pplexer_shift_buf(self, readbuf_readc(self->buf));
        if (c_pplexer_getc(self) == '\\' && char_is_newline(c_pplexer_get_nextc(self)))
        {
                self->chars[1] = readbuf_readc(self->buf);
                self->chars[2] = readbuf_readc(self->buf);
                self->loc += 2;
        }

        if (c_pplexer_at_start_of_line(self))
                c_source_save_line_loc(self->source, c_pplexer_loc(self));

        return c_pplexer_getc(self);
}

extern void c_pplexer_init(
        c_pplexer* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
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

extern serrcode c_pplexer_enter_source_file(c_pplexer* self, c_source* source)
{
        if (!source)
                return S_ERROR;
        
        if (!(self->buf = c_source_open(source)))
        {
                c_error_cannot_open_source_file(self->logger, 0, c_source_get_name(source));
                return S_ERROR;
        }

        // save first line location
        if (S_FAILED(c_source_save_line_loc(source, c_source_get_loc_begin(source))))
                return S_ERROR;

        // fill buffer
        cpplexer_readc(self);
        cpplexer_readc(self);
        self->loc = c_source_get_loc_begin(source);
        self->source = source;
        return S_NO_ERROR;
}

typedef struct 
{
        // size of the sequence including trailing zero
        ssize size;
        tree_location loc;
        char val[C_MAX_LINE_LENGTH + 1];
} c_sequence;

static inline void c_sequence_finish(c_sequence* self)
{
        self->val[self->size++] = '\0';
}

static void c_sequence_init(c_sequence* self)
{
        self->size = 0;
        self->loc = 0;
        self->val[0] = '\0';
}

static inline bool c_sequence_append(c_sequence* self, const c_pplexer* lexer, int c)
{
        if (self->size == C_MAX_LINE_LENGTH)
        {
                c_error_token_is_too_long(lexer->logger, self->loc);
                return false;
        }

        self->val[self->size++] = (char)c;
        return true;
}

static tree_id c_pplexer_pool_seq(c_pplexer* self, c_sequence* seq)
{
        tree_id id = tree_get_id_for_string(c_context_get_tree_context(self->context), seq->val, seq->size);
        S_ASSERT(id != TREE_INVALID_ID);
        return id;
}

static bool c_pplex_word_seq(c_pplexer* self, c_sequence* seq)
{
        seq->loc = c_pplexer_loc(self);
        int c = c_pplexer_getc(self);
        while (get_char_info(c) & (ACK_ALPHA | ACK_DIGIT))
        {
                if (!c_sequence_append(seq, self, c))
                        return false;

                c = cpplexer_readc(self);
        }
        c_sequence_finish(seq);
        return true;
}

extern c_token* c_pplex_identifier(c_pplexer* self)
{
        c_sequence word;
        c_sequence_init(&word);
        if (!c_pplex_word_seq(self, &word))
                return NULL;

        return c_token_new_id(self->context, word.loc, c_pplexer_pool_seq(self, &word));
}

static bool c_pplex_quoted_seq(c_pplexer* self, c_sequence* seq, int quote)
{
        seq->loc = c_pplexer_loc(self);
        bool consume = false;
        while (1)
        {
                int c = cpplexer_readc(self);
                if (char_is_newline(c) || c_pplexer_at_eof(self))
                {
                        c_error_missing_closing_quote(self->logger, seq->loc);
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

                if (!c_sequence_append(seq, self, c))
                        return false;
        }
        c_sequence_finish(seq);
        return true;
}

extern c_token* c_pplex_string_literal(c_pplexer* self)
{
        c_sequence schar;
        c_sequence_init(&schar);
        if (!c_pplex_quoted_seq(self, &schar, '"'))
                return NULL;

        return c_token_new_string(self->context, schar.loc, c_pplexer_pool_seq(self, &schar));
}

extern c_token* c_pplex_angle_string_literal(c_pplexer* self)
{
        c_sequence seq;
        c_sequence_init(&seq);
        if (!c_pplex_quoted_seq(self, &seq, '>'))
                return NULL;

        return c_token_new_angle_string(self->context, seq.loc, c_pplexer_pool_seq(self, &seq));
}

extern c_token* c_pplex_const_char(c_pplexer* self)
{
        c_sequence cchar;
        c_sequence_init(&cchar);
        if (!c_pplex_quoted_seq(self, &cchar, '\''))
                return NULL;

        bool escape = cchar.val[0] == '\\';
        if (cchar.size == 1)
        {
                c_error_empty_character_constant(self->logger, cchar.loc);
                return NULL;
        }
        else if ((escape && cchar.size > 3) || (!escape && cchar.size > 2))
        {
                c_error_invalid_character_constant(self->logger, cchar.loc);
                return NULL;
        }

        int c = escape ? char_to_escape(cchar.val[1]) : cchar.val[0];
        return c_token_new_char(self->context, cchar.loc, c);
}

static bool c_pplex_num_seq(c_pplexer* self, c_sequence* seq)
{
        seq->loc = c_pplexer_loc(self);
        int c = c_pplexer_getc(self);
        while(1)
        {
                bool pp = (get_char_info(c) & (ACK_DIGIT | ACK_ALPHA))
                        || c == '.' || c == '+' || c == '-';
                if (!pp)
                {
                        c_sequence_finish(seq);
                        return true;
                }

                if (!c_sequence_append(seq, self, c))
                        return false;

                c = cpplexer_readc(self);
        }
}

extern c_token* c_pplex_number(c_pplexer* self)
{
        c_sequence num;
        c_sequence_init(&num);
        if (!c_pplex_num_seq(self, &num))
                return NULL;

        return ctoken_new_pp_num(self->context, num.loc, c_pplexer_pool_seq(self, &num));
}

static c_token* c_pplex_comment(c_pplexer* self)
{
        tree_location loc = c_pplexer_loc(self);
        int c = cpplexer_readc(self); // '/'
        if (c == '/')
        {
                while (!char_is_newline(c) && !c_pplexer_at_eof(self))
                        c = cpplexer_readc(self);
        }
        else
        {
                while (1)
                {
                        c = cpplexer_readc(self);
                        if (c == '/' && c_pplexer_get_prevc(self) == '*')
                        {
                                cpplexer_readc(self); // '/'
                                break;
                        }
                        else if (c_pplexer_at_eof(self))
                        {
                                c_error_unclosed_comment(self->logger, loc);
                                return NULL;
                        }
                }
        }

        return c_token_new(self->context, CTK_COMMENT, loc);
}

static inline c_token_kind _c_pplex_punctuator(c_pplexer* self)
{
        switch ((char)c_pplexer_getc(self))
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
                        if (c_pplexer_getc(self) == '.')
                        {
                                if (c_pplexer_get_nextc(self) == '.')
                                {
                                        cpplexer_readc(self);
                                        cpplexer_readc(self);
                                        return CTK_ELLIPSIS;
                                }
                        }
                        return CTK_DOT;

                case '-':
                        cpplexer_readc(self);
                        switch (c_pplexer_getc(self))
                        {
                                case '>': cpplexer_readc(self); return CTK_ARROW;
                                case '=': cpplexer_readc(self); return CTK_MINUS_EQ;
                                case '-': cpplexer_readc(self); return CTK_MINUS2;
                                default: return CTK_MINUS;
                        }

                case '<':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '<')
                        {
                                cpplexer_readc(self);
                                if (c_pplexer_getc(self) == '=')
                                {
                                        cpplexer_readc(self);
                                        return CTK_LE2_EQ;
                                }
                                return CTK_LE2;
                        }
                        else if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_LEQ;
                        }
                        return CTK_LE;

                case '>':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '>')
                        {
                                cpplexer_readc(self);
                                if (c_pplexer_getc(self) == '=')
                                {
                                        cpplexer_readc(self);
                                        return CTK_GR2_EQ;
                                }
                                return CTK_GR2;
                        }
                        else if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_GREQ;
                        }
                        return CTK_GR;

                case '!':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_EXCLAIM_EQ;
                        }
                        return CTK_EXCLAIM;

                case '=':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_EQ2;
                        }
                        return CTK_EQ;

                case '&':
                        cpplexer_readc(self);
                        switch (c_pplexer_getc(self))
                        {
                                case '&': cpplexer_readc(self); return CTK_AMP2;
                                case '=': cpplexer_readc(self); return CTK_AMP_EQ;
                                default: return CTK_AMP;
                        }

                case '|':
                        cpplexer_readc(self);
                        switch (c_pplexer_getc(self))
                        {
                                case '|': cpplexer_readc(self); return CTK_VBAR2;
                                case '=': cpplexer_readc(self); return CTK_VBAR_EQ;
                                default: return CTK_VBAR;
                        }

                case '^':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_CARET_EQ;
                        }
                        return CTK_CARET;

                case '+':
                        cpplexer_readc(self);
                        switch (c_pplexer_getc(self))
                        {
                                case '+': cpplexer_readc(self); return CTK_PLUS2;
                                case '=': cpplexer_readc(self); return CTK_PLUS_EQ;
                                default: return CTK_PLUS;
                        }

                case '*':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_STAR_EQ;
                        }
                        return CTK_STAR;

                case '/':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_SLASH_EQ;
                        }
                        return CTK_SLASH;

                case '%':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '=')
                        {
                                cpplexer_readc(self);
                                return CTK_PERCENT_EQ;
                        }
                        return CTK_PERCENT;

                case '#':
                        cpplexer_readc(self);
                        if (c_pplexer_getc(self) == '#')
                        {
                                cpplexer_readc(self);
                                return CTK_HASH2;
                        }
                        return CTK_HASH;

                default:
                        return CTK_UNKNOWN;
        }
}

static c_token* c_pplex_punctuator(c_pplexer* self)
{
        tree_location loc = c_pplexer_loc(self);
        c_token_kind k = _c_pplex_punctuator(self);
        if (k == CTK_UNKNOWN)
        {
                c_error_unknown_punctuator(self->logger, loc, c_pplexer_getc(self));
                return NULL;
        }
        return c_token_new(self->context, k, loc);
}

static c_token* c_pplex_symbols(c_pplexer* self)
{
        int c = c_pplexer_getc(self);
        if (c == '\"')
                return c_pplex_string_literal(self);
        else if (c == '\'')
                return c_pplex_const_char(self);
        else if (c == '<' && self->angle_string_expected)
                return c_pplex_angle_string_literal(self);

        int next = c_pplexer_get_nextc(self);
        if (c == '.' && char_is_digit(next))
                return c_pplex_number(self);
        else if (c == '/' && (next == '/' || next == '*'))
                return c_pplex_comment(self);

        return c_pplex_punctuator(self);
}

static c_token* c_pplex_wspace(c_pplexer* self)
{
        tree_location loc = c_pplexer_loc(self);
        int spaces = 0;
        int c = c_pplexer_getc(self);
        while (char_is_wspace(c) && !c_pplexer_at_eof(self))
        {
                spaces += c == '\t' ? self->tab_to_space : 1;
                c = cpplexer_readc(self);
        }
        return c_token_new_wspace(self->context, loc, spaces);
}

extern c_token* c_pplex_token(c_pplexer* self)
{
        int c = c_pplexer_getc(self);
        int i = get_char_info(c);

        if (i & ACK_ALPHA)
                return c_pplex_identifier(self);
        else if (i & ACK_SYMBOL)
                return c_pplex_symbols(self);
        else if (i & ACK_DIGIT)
                return c_pplex_number(self);
        else if (i & ACK_WSPACE)
                return c_pplex_wspace(self);
        else if (i & ACK_NEWLINE)
        {
                tree_location loc = c_pplexer_loc(self);
                cpplexer_readc(self);
                return c_token_new(self->context, CTK_EOL, loc);

        }
        else if (c_pplexer_at_eof(self))
        {
                tree_location loc = c_pplexer_loc(self);
                cpplexer_readc(self);
                return c_token_new(self->context, CTK_EOF, loc);
        }

        c_error_unknown_symbol(self->logger, c_pplexer_loc(self), c);
        return NULL;
}

extern void c_preprocessor_init(
        c_preprocessor* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        self->reswords = reswords;
        self->state = self->files - 1;
        self->source_manager = source_manager;
        self->logger = logger;
        self->context = context;
        dseq_init_alloc(&self->expansion, c_context_get_allocator(context));
}

extern void c_preprocessor_dispose(c_preprocessor* self)
{
        while (self->state != self->files - 1)
        {
                c_source_close(self->state->source);
                self->state--;
        }
}

extern serrcode c_preprocessor_enter(c_preprocessor* self, c_source* source)
{
        if (!source)
                return S_ERROR;
        if (self->state - self->files >= C_MAX_INCLUDE_NESTING)
        {
                c_error_too_deep_include_nesting(self->logger);
                return S_ERROR;
        }

        c_preprocessor_state* next = self->state + 1;
        c_pplexer* pplexer = &next->lexer;
        c_pplexer_init(
                pplexer,
                self->reswords,
                self->source_manager,
                self->logger, 
                self->context);
        if (S_FAILED(c_pplexer_enter_source_file(pplexer, source)))
                return S_ERROR;

        self->state = next;
        self->state->source = source;
        self->state->nhash = 0;
        self->state->hash_expected = true;
        self->state->in_directive = false;
        return S_NO_ERROR;
}

extern void c_preprocessor_exit(c_preprocessor* self)
{
        S_ASSERT(self->state != self->files);
        c_source_close(self->state->source);
        self->state--;
}

static c_token* c_preprocess_wspace(c_preprocessor* self, bool skip_eol)
{
        while (1)
        {
                c_token* t = c_pplex_token(&self->state->lexer);
                if (!t)
                        return NULL;

                c_token_kind k = c_token_get_kind(t);
                if (k == CTK_WSPACE || k == CTK_COMMENT || (k == CTK_EOL && skip_eol))
                        continue;
                else if (k == CTK_EOF && self->state != self->files)
                {
                        // consume eof of included source file
                        c_preprocessor_exit(self);
                        continue;
                }

                return t;
        }
}

static c_token* c_preprocess_directive(c_preprocessor*);

static c_token* c_preprocess_include(c_preprocessor* self)
{
        self->state->lexer.angle_string_expected = true;
        c_token* t = c_preprocess_wspace(self, false);
        self->state->lexer.angle_string_expected = false;
        if (!t)
                return NULL;

        tree_location token_loc = c_token_get_loc(t);
        if (!c_token_is(t, CTK_CONST_STRING) && !c_token_is(t, CTK_ANGLE_STRING))
        {
                c_error_expected_file_name(self->logger, token_loc);
                return NULL;
        }

        tree_id ref = c_token_get_string(t);
        const char* filename = tree_get_id_string(c_context_get_tree_context(self->context), ref);
        S_ASSERT(filename);

        if (!*filename)
        {
                c_error_empty_file_name_in_include(self->logger, token_loc);
                return NULL;
        }

        c_source* source = c_source_find(self->source_manager, filename);
        if (!source)
        {
                c_error_cannot_open_source_file(self->logger, token_loc, filename);
                return NULL;
        }

        if (S_FAILED(c_preprocessor_enter(self, source)))
                return NULL;

        return c_preprocess_directive(self);
}

static c_token* c_preprocess_directive(c_preprocessor* self)
{
        c_token* t = c_preprocess_wspace(self, true);
        if (!t || !c_token_is(t, CTK_HASH))
                return t;

        if (!self->state->hash_expected)
        {
                c_error_unexpected_hash(self->logger, c_token_get_loc(t));
                return NULL;
        }

        if (!(t = c_preprocess_wspace(self, false)))
                return NULL;

        c_token_kind directive = CTK_UNKNOWN;
        if (c_token_is(t, CTK_ID))
                directive = c_reswords_get_pp_by_ref(self->reswords, c_token_get_string(t));

        if (directive == CTK_UNKNOWN)
        {
                c_error_unknown_preprocessor_directive(self->logger, c_token_get_loc(t));
                return false;
        }

        self->state->in_directive = true;
        if (directive == CTK_PP_INCLUDE)
                t = c_preprocess_include(self);
        else
        {
                t = NULL;
                c_error_unsupported_preprocessor_directive(self->logger, c_token_get_loc(t));
        }
        self->state->in_directive = false;
        return t;
}

static c_token* c_preprocess_macro_id(c_preprocessor* self)
{
        if (dseq_size(&self->expansion))
        {
                c_token* last = *(dseq_end(&self->expansion) - 1);
                dseq_resize(&self->expansion, dseq_size(&self->expansion) - 1);
                return last;
        }

        c_token* t = c_preprocess_directive(self);
        if (!t)
                return NULL;

        if (!c_token_is(t, CTK_ID))
                return t;

        if (0) // token is macro id
        {
                // push back replacement
                // ...
                // return (ctoken*)list_pop_front(&self->expansion);
        }
        return t;
}

static inline void c_preprocessor_unget_macro_id(c_preprocessor* self, c_token* t)
{
        dseq_append(&self->expansion, t);
}

static bool c_preprocessor_collect_adjacent_strings(c_preprocessor* self, dseq* result)
{
        while (1)
        {
                c_token* t = c_preprocess_macro_id(self);
                if (!t)
                        return false;

                if (!c_token_is(t, CTK_CONST_STRING))
                {
                        c_preprocessor_unget_macro_id(self, t);
                        return true;
                }

                dseq_append(result, t);
        }
}

static c_token* c_preprocessor_concat_and_escape_strings(c_preprocessor* self, dseq* strings)
{
        S_ASSERT(dseq_size(strings));

        dseq_u8 concat;
        dseq_u8_init_alloc(&concat, c_context_get_allocator(self->context));
        for (ssize i = 0; i < dseq_size(strings); i++)
        {
                c_token* t = dseq_get(strings, i);
                const char* string = tree_get_id_string(
                        c_context_get_tree_context(self->context), c_token_get_string(t));

                char escaped[C_MAX_LINE_LENGTH + 1];
                ssize size = c_get_escaped_string(escaped, string, strlen(string));
                for (ssize j = 0; j < size - 1; j++)
                        dseq_u8_append(&concat, escaped[j]);
        }
        dseq_u8_append(&concat, '\0');

        tree_id concat_ref = tree_get_id_for_string(
                c_context_get_tree_context(self->context), (char*)dseq_u8_begin(&concat), dseq_u8_size(&concat));
        tree_location loc = c_token_get_loc(dseq_get(strings, 0));
        dseq_u8_dispose(&concat);

        return c_token_new_string(self->context, loc, concat_ref);
}

static c_token* c_preprocess_string(c_preprocessor* self)
{
        c_token* t = c_preprocess_macro_id(self);
        if (!t)
                return NULL;

        if (!c_token_is(t, CTK_CONST_STRING))
                return t;

        dseq adjacent_strings;
        dseq_init_alloc(&adjacent_strings, c_context_get_allocator(self->context));
        dseq_append(&adjacent_strings, t);

        c_token* result = c_preprocessor_collect_adjacent_strings(self, &adjacent_strings)
                ? c_preprocessor_concat_and_escape_strings(self, &adjacent_strings)
                : NULL;

        dseq_dispose(&adjacent_strings);
        return result;
}

extern c_token* c_preprocess(c_preprocessor* self)
{
        return c_preprocess_string(self);
}