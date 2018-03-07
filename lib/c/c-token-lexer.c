#include "scc/c/c-token-lexer.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-info.h"
#include "scc/c/c-source.h"
#include "scc/c/c-limits.h"
#include "scc/core/char-info.h"
#include "scc/core/read-write.h"

static inline tree_location c_token_lexer_get_loc(const c_token_lexer* self)
{
        return self->loc;
}

static inline int c_token_lexer_get_prevc(const c_token_lexer* self)
{
        return self->chars[0];
}

static inline int c_token_lexer_getc(const c_token_lexer* self)
{
        return self->chars[1];
}

static inline int c_token_lexer_get_nextc(const c_token_lexer* self)
{
        return self->chars[2];
}

extern bool c_token_lexer_at_eof(const c_token_lexer* self)
{
        return c_token_lexer_getc(self) == RB_ENDC;
}

static inline bool c_token_lexer_at_start_of_line(const c_token_lexer* self)
{
        return c_token_lexer_get_prevc(self) == '\n';
}

static inline void c_token_lexer_shift_buf(c_token_lexer* self, int c)
{
        self->chars[0] = self->chars[1];
        self->chars[1] = self->chars[2];
        self->chars[2] = c;
        self->loc++;
}

static inline int c_token_lexer_readc(c_token_lexer* self)
{
        c_token_lexer_shift_buf(self, readbuf_readc(self->buf));
        if (c_token_lexer_getc(self) == '\\'
                && char_is_newline(c_token_lexer_get_nextc(self)))
        {
                self->chars[1] = readbuf_readc(self->buf);
                self->chars[2] = readbuf_readc(self->buf);
                self->loc += 2;
        }

        if (c_token_lexer_at_start_of_line(self) && self->source)
                c_source_save_line_loc(self->source, c_token_lexer_get_loc(self));

        return c_token_lexer_getc(self);
}

extern void c_token_lexer_init(
        c_token_lexer* self,
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
        self->hash_expected = true;
        self->in_directive = false;
}

extern void c_token_lexer_enter_char_stream(
        c_token_lexer* self, readbuf* buf, tree_location start_loc)
{
        self->buf = buf;
        // fill buffer
        c_token_lexer_readc(self);
        c_token_lexer_readc(self);
        self->loc = start_loc;
        self->hash_expected = true;
        self->in_directive = false;
}

extern errcode c_token_lexer_enter(c_token_lexer* self, c_source* source)
{
        if (!source)
                return EC_ERROR;

        readbuf* buf = c_source_open(source);
        if (!buf)
        {
                c_error_cannot_open_source_file(self->logger, 0, c_source_get_name(source));
                return EC_ERROR;
        }

        // save first line location
        if (EC_FAILED(c_source_save_line_loc(source, c_source_get_loc_begin(source))))
                return EC_ERROR;

        self->source = source;
        c_token_lexer_enter_char_stream(self, buf, c_source_get_loc_begin(source));
        return EC_NO_ERROR;
}

typedef struct
{
        // size of the sequence including trailing zero
        size_t size;
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

static inline bool c_sequence_append(c_sequence* self, const c_token_lexer* lexer, int c)
{
        if (self->size == C_MAX_LINE_LENGTH)
        {
                c_error_token_is_too_long(lexer->logger, self->loc);
                return false;
        }

        self->val[self->size++] = (char)c;
        return true;
}

static tree_id c_token_lexer_get_sequence_id(c_token_lexer* self, c_sequence* seq)
{
        tree_id id = tree_get_id_for_string_s(
                c_context_get_tree_context(self->context), seq->val, seq->size);
        assert(id != TREE_INVALID_ID);
        return id;
}

static bool c_token_lexer_read_word_sequence(c_token_lexer* self, c_sequence* seq)
{
        seq->loc = c_token_lexer_get_loc(self);
        int c = c_token_lexer_getc(self);
        while (get_char_info(c) & (ACK_ALPHA | ACK_DIGIT))
        {
                if (!c_sequence_append(seq, self, c))
                        return false;

                c = c_token_lexer_readc(self);
        }
        c_sequence_finish(seq);
        return true;
}

extern c_token* c_token_lexer_lex_identifier(c_token_lexer* self)
{
        c_sequence word;
        c_sequence_init(&word);
        if (!c_token_lexer_read_word_sequence(self, &word))
                return NULL;

        return c_token_new_id(self->context,
                word.loc, c_token_lexer_get_sequence_id(self, &word));
}

static bool c_token_lexer_read_quoted_sequence(
        c_token_lexer* self, c_sequence* seq, int quote)
{
        seq->loc = c_token_lexer_get_loc(self);
        bool consume = false;
        while (1)
        {
                int c = c_token_lexer_readc(self);
                if (char_is_newline(c) || c_token_lexer_at_eof(self))
                {
                        c_error_missing_closing_quote(self->logger, seq->loc);
                        return false;
                }
                else if (c == '\\')
                        consume = true;
                else if (c == quote && !consume)
                {
                        c_token_lexer_readc(self);
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

extern c_token* c_token_lexer_lex_string_literal(c_token_lexer* self)
{
        c_sequence schar;
        c_sequence_init(&schar);
        if (!c_token_lexer_read_quoted_sequence(self, &schar, '"'))
                return NULL;

        return c_token_new_string(self->context,
                schar.loc, c_token_lexer_get_sequence_id(self, &schar));
}

extern c_token* c_token_lexer_lex_angle_string_literal(c_token_lexer* self)
{
        c_sequence seq;
        c_sequence_init(&seq);
        if (!c_token_lexer_read_quoted_sequence(self, &seq, '>'))
                return NULL;

        return c_token_new_angle_string(self->context,
                seq.loc, c_token_lexer_get_sequence_id(self, &seq));
}

extern c_token* c_token_lexer_lex_character_constant(c_token_lexer* self)
{
        c_sequence cchar;
        c_sequence_init(&cchar);
        if (!c_token_lexer_read_quoted_sequence(self, &cchar, '\''))
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

static bool c_token_lexer_read_number_sequence(c_token_lexer* self, c_sequence* seq)
{
        seq->loc = c_token_lexer_get_loc(self);
        int c = c_token_lexer_getc(self);
        while (1)
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

                c = c_token_lexer_readc(self);
        }
}

extern c_token* c_token_lexer_lex_number(c_token_lexer* self)
{
        c_sequence num;
        c_sequence_init(&num);
        if (!c_token_lexer_read_number_sequence(self, &num))
                return NULL;

        return c_token_new_pp_num(self->context,
                num.loc, c_token_lexer_get_sequence_id(self, &num));
}

static c_token* c_token_lexer_lex_comment(c_token_lexer* self)
{
        tree_location loc = c_token_lexer_get_loc(self);
        int c = c_token_lexer_readc(self); // '/'
        if (c == '/')
        {
                while (!char_is_newline(c) && !c_token_lexer_at_eof(self))
                        c = c_token_lexer_readc(self);
        }
        else
        {
                while (1)
                {
                        c = c_token_lexer_readc(self);
                        if (c == '/' && c_token_lexer_get_prevc(self) == '*')
                        {
                                c_token_lexer_readc(self); // '/'
                                break;
                        }
                        else if (c_token_lexer_at_eof(self))
                        {
                                c_error_unclosed_comment(self->logger, loc);
                                return NULL;
                        }
                }
        }

        return c_token_new(self->context, CTK_COMMENT, loc);
}

static inline c_token_kind _c_token_lexer_lex_punctuator(c_token_lexer* self)
{
        switch ((char)c_token_lexer_getc(self))
        {
                case '{': c_token_lexer_readc(self); return CTK_LBRACE;
                case '}': c_token_lexer_readc(self); return CTK_RBRACE;
                case '[': c_token_lexer_readc(self); return CTK_LSBRACKET;
                case ']': c_token_lexer_readc(self); return CTK_RSBRACKET;
                case '(': c_token_lexer_readc(self); return CTK_LBRACKET;
                case ')': c_token_lexer_readc(self); return CTK_RBRACKET;
                case ':': c_token_lexer_readc(self); return CTK_COLON;
                case '?': c_token_lexer_readc(self); return CTK_QUESTION;
                case ',': c_token_lexer_readc(self); return CTK_COMMA;
                case '~': c_token_lexer_readc(self); return CTK_TILDE;
                case ';': c_token_lexer_readc(self); return CTK_SEMICOLON;

                case '.':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '.')
                        {
                                if (c_token_lexer_get_nextc(self) == '.')
                                {
                                        c_token_lexer_readc(self);
                                        c_token_lexer_readc(self);
                                        return CTK_ELLIPSIS;
                                }
                        }
                        return CTK_DOT;

                case '-':
                        c_token_lexer_readc(self);
                        switch (c_token_lexer_getc(self))
                        {
                                case '>': c_token_lexer_readc(self); return CTK_ARROW;
                                case '=': c_token_lexer_readc(self); return CTK_MINUS_EQ;
                                case '-': c_token_lexer_readc(self); return CTK_MINUS2;
                                default: return CTK_MINUS;
                        }

                case '<':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '<')
                        {
                                c_token_lexer_readc(self);
                                if (c_token_lexer_getc(self) == '=')
                                {
                                        c_token_lexer_readc(self);
                                        return CTK_LE2_EQ;
                                }
                                return CTK_LE2;
                        }
                        else if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_LEQ;
                        }
                        return CTK_LE;

                case '>':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '>')
                        {
                                c_token_lexer_readc(self);
                                if (c_token_lexer_getc(self) == '=')
                                {
                                        c_token_lexer_readc(self);
                                        return CTK_GR2_EQ;
                                }
                                return CTK_GR2;
                        }
                        else if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_GREQ;
                        }
                        return CTK_GR;

                case '!':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_EXCLAIM_EQ;
                        }
                        return CTK_EXCLAIM;

                case '=':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_EQ2;
                        }
                        return CTK_EQ;

                case '&':
                        c_token_lexer_readc(self);
                        switch (c_token_lexer_getc(self))
                        {
                                case '&': c_token_lexer_readc(self); return CTK_AMP2;
                                case '=': c_token_lexer_readc(self); return CTK_AMP_EQ;
                                default: return CTK_AMP;
                        }

                case '|':
                        c_token_lexer_readc(self);
                        switch (c_token_lexer_getc(self))
                        {
                                case '|': c_token_lexer_readc(self); return CTK_VBAR2;
                                case '=': c_token_lexer_readc(self); return CTK_VBAR_EQ;
                                default: return CTK_VBAR;
                        }

                case '^':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_CARET_EQ;
                        }
                        return CTK_CARET;

                case '+':
                        c_token_lexer_readc(self);
                        switch (c_token_lexer_getc(self))
                        {
                                case '+': c_token_lexer_readc(self); return CTK_PLUS2;
                                case '=': c_token_lexer_readc(self); return CTK_PLUS_EQ;
                                default: return CTK_PLUS;
                        }

                case '*':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_STAR_EQ;
                        }
                        return CTK_STAR;

                case '/':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_SLASH_EQ;
                        }
                        return CTK_SLASH;

                case '%':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '=')
                        {
                                c_token_lexer_readc(self);
                                return CTK_PERCENT_EQ;
                        }
                        return CTK_PERCENT;

                case '#':
                        c_token_lexer_readc(self);
                        if (c_token_lexer_getc(self) == '#')
                        {
                                c_token_lexer_readc(self);
                                return CTK_HASH2;
                        }
                        return CTK_HASH;

                default:
                        return CTK_UNKNOWN;
        }
}

static c_token* c_token_lexer_lex_punctuator(c_token_lexer* self)
{
        tree_location loc = c_token_lexer_get_loc(self);
        c_token_kind k = _c_token_lexer_lex_punctuator(self);
        if (k == CTK_UNKNOWN)
        {
                c_error_unknown_punctuator(self->logger, loc, c_token_lexer_getc(self));
                return NULL;
        }
        return c_token_new(self->context, k, loc);
}

static c_token* c_token_lexer_lex_symbols(c_token_lexer* self)
{
        int c = c_token_lexer_getc(self);
        if (c == '\"')
                return c_token_lexer_lex_string_literal(self);
        else if (c == '\'')
                return c_token_lexer_lex_character_constant(self);
        else if (c == '<' && self->angle_string_expected)
                return c_token_lexer_lex_angle_string_literal(self);

        int next = c_token_lexer_get_nextc(self);
        if (c == '.' && char_is_digit(next))
                return c_token_lexer_lex_number(self);
        else if (c == '/' && (next == '/' || next == '*'))
                return c_token_lexer_lex_comment(self);

        return c_token_lexer_lex_punctuator(self);
}

static c_token* c_token_lexer_lex_wspace(c_token_lexer* self)
{
        tree_location loc = c_token_lexer_get_loc(self);
        int spaces = 0;
        int c = c_token_lexer_getc(self);
        while (char_is_wspace(c) && !c_token_lexer_at_eof(self))
        {
                spaces += c == '\t' ? self->tab_to_space : 1;
                c = c_token_lexer_readc(self);
        }
        return c_token_new_wspace(self->context, loc, spaces);
}

static inline c_token* _c_token_lexer_lex_token(c_token_lexer* self)
{
        int c = c_token_lexer_getc(self);
        int i = get_char_info(c);

        if (i & ACK_ALPHA)
                return c_token_lexer_lex_identifier(self);
        else if (i & ACK_SYMBOL)
                return c_token_lexer_lex_symbols(self);
        else if (i & ACK_DIGIT)
                return c_token_lexer_lex_number(self);
        else if (i & ACK_WSPACE)
                return c_token_lexer_lex_wspace(self);
        else if (i & ACK_NEWLINE)
        {
                tree_location loc = c_token_lexer_get_loc(self);
                c_token_lexer_readc(self);
                return c_token_new(self->context, self->in_directive ? CTK_EOD : CTK_EOL, loc);

        }
        else if (c_token_lexer_at_eof(self))
                return c_token_new(self->context, CTK_EOF, c_token_lexer_get_loc(self));

        c_error_unknown_symbol(self->logger, c_token_lexer_get_loc(self), c);
        return NULL;
}

extern c_token* c_token_lexer_lex_token(c_token_lexer* self)
{
        c_token* t = _c_token_lexer_lex_token(self);
        if (!t)
                return NULL;

        if (c_token_is(t, CTK_EOL) || c_token_is(t, CTK_EOD))
                self->hash_expected = true;
        else if (!c_token_is(t, CTK_WSPACE) && !c_token_is(t, CTK_COMMENT))
        {
                if (c_token_is(t, CTK_HASH) && !self->hash_expected && !self->in_directive)
                {
                        c_error_unexpected_hash(self->logger, c_token_get_loc(t));
                        return NULL;
                }
                self->hash_expected = false;
        }
        return t;
}