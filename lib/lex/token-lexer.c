#include "scc/lex/token-lexer.h"
#include "scc/lex/token-kind.h"
#include "scc/c-common/source.h"
#include "scc/c-common/context.h"
#include "scc/tree/context.h"
#include "scc/lex/charset.h"
#include "errors.h"
#include <ctype.h> // toupper

typedef struct
{
        char buffer[C_MAX_LINE_LENGTH + 1];
        char* pos;
        tree_location loc;
        c_context* context;
} c_sequence;

static size_t c_sequence_length(const c_sequence* self)
{
        return (size_t)(self->pos - self->buffer);
}

static bool c_sequence_append(c_sequence* self, int c)
{
        if (c_sequence_length(self) >= C_MAX_LINE_LENGTH)
        {
                c_error_token_is_too_long(self->context, self->loc);
                return false;
        }
        *self->pos++ = (unsigned char)c;
        *self->pos = '\0';
        return true;
}

static void c_sequence_init(c_sequence* self, c_context* context, tree_location loc)
{
        self->context = context;
        self->loc = loc;
        self->pos = self->buffer;
        *self->pos = '\0';
}

static tree_id c_sequence_get_id(c_sequence* self)
{
        return tree_get_id_for_string(self->context->tree, self->buffer);
}

extern void c_token_lexer_init(c_token_lexer* self, c_context* context)
{
        self->c = RB_ENDC;
        self->nextc = RB_ENDC;

        self->input = NULL;

        self->angle_string_expected = false;
        self->hash_expected = true;
        self->in_directive = false;
        self->eod_before_eof_returned = false;

        self->source = NULL;
        self->context = context;

        self->loc = TREE_INVALID_LOC;
        self->line = -1;
        self->tab_to_space = 4;
}

static inline void _c_token_lexer_readc(c_token_lexer* self)
{
        self->c = self->nextc;
        self->nextc = readbuf_readc(self->input);
        self->loc++;

        if (self->c == '\r')
        {
                if (self->nextc == '\n')
                {
                        self->c = self->nextc;
                        self->nextc = readbuf_readc(self->input);
                }
                else
                        self->c = '\n';
        }
}

static inline int c_token_lexer_readc(c_token_lexer* self)
{
        _c_token_lexer_readc(self);

        while (self->c == '\\' && (self->nextc == '\n' || self->nextc == '\r'))
        {
                _c_token_lexer_readc(self);
                _c_token_lexer_readc(self);
        }

        if (self->c == '\n')
        {
                self->line++;
                if (self->source)
                        c_source_save_line_loc(self->source, self->loc + 1);
        }

        return self->c;
}

extern void c_token_lexer_enter_char_stream(
        c_token_lexer* self, readbuf* input, tree_location start_loc)
{
        self->input = input;
        c_token_lexer_readc(self);
        c_token_lexer_readc(self);

        self->loc = start_loc;
        self->hash_expected = true;
        self->in_directive = false;

        c_location loc;
        c_source_find_loc(&self->context->source_manager, &loc, start_loc);
        self->line = loc.line;
}

extern errcode c_token_lexer_enter(c_token_lexer* self, c_source* source)
{
        if (!source)
                return EC_ERROR;

        readbuf* buf = c_source_open(source);
        if (!buf)
        {
                c_error_cannot_open_source_file(self->context, 0, c_source_get_name(source));
                return EC_ERROR;
        }

        tree_location start_loc = c_source_get_loc_begin(source);
        // save first line location
        if (EC_FAILED(c_source_save_line_loc(source, start_loc)))
                return EC_ERROR;

        self->source = source;
        c_token_lexer_enter_char_stream(self, buf, start_loc);
        return EC_NO_ERROR;
}

extern bool c_token_lexer_at_eof(const c_token_lexer* self)
{
        return self->c == RB_ENDC;
}

static c_token* c_token_lexer_lex_identifier(c_token_lexer* self, c_sequence* seq)
{
        c_sequence_append(seq, self->c);
        while (1)
        {
                int c = c_token_lexer_readc(self);
                if (c_token_lexer_at_eof(self) || !(c_char_info_table[c] & (CCK_LETTER | CCK_DIGIT)))
                        break;
                if (!c_sequence_append(seq, c))
                        return NULL;
        }

        return c_token_new_id(self->context, seq->loc, c_sequence_get_id(seq));
}

static c_token* c_token_lexer_lex_spaces(c_token_lexer* self)
{
        tree_location loc = self->loc;
        int spaces = 0;
        int c = self->c;
        while (!c_token_lexer_at_eof(self) && c_char_is_space(c))
        {
                spaces += c == '\t' ? self->tab_to_space : 1;
                c = c_token_lexer_readc(self);
        }
        return c_token_new_wspace(self->context, loc, spaces);
}

static bool c_token_lexer_read_till_quote(c_token_lexer* self, c_sequence* seq, int quote)
{
        bool consume = false;
        while (1)
        {
                int c = c_token_lexer_readc(self);
                if (c_token_lexer_at_eof(self) || c == '\n')
                {
                        c_error_missing_closing_quote(self->context, seq->loc);
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

                if (!c_sequence_append(seq, c))
                        return false;
        }
        return true;
}

static c_token* c_token_lexer_lex_string_literal(c_token_lexer* self, c_sequence* seq)
{
        return c_token_lexer_read_till_quote(self, seq, '"')
                ? c_token_new_string(self->context, seq->loc, c_sequence_get_id(seq))
                : NULL;
}

static c_token* c_token_lexer_lex_angle_string_literal(c_token_lexer* self, c_sequence* seq)
{
        return c_token_lexer_read_till_quote(self, seq, '>')
                ? c_token_new_angle_string(self->context, seq->loc, c_sequence_get_id(seq))
                : NULL;
}

static c_token* c_token_lexer_lex_character_constant(c_token_lexer* self, c_sequence* seq)
{
        if (!c_token_lexer_read_till_quote(self, seq, '\''))
                return NULL;

        bool escape = seq->buffer[0] == '\\';
        size_t len = c_sequence_length(seq);
        if (len == 0)
        {
                c_error_empty_character_constant(self->context, seq->loc);
                return NULL;
        }
        else if ((escape && len > 2) || (!escape && len > 1))
        {
                c_error_invalid_character_constant(self->context, seq->loc);
                return NULL;
        }

        int c = escape ? c_char_to_escape(seq->buffer[1]) : seq->buffer[0];
        return c_token_new_char(self->context, seq->loc, c);
}

static c_token* c_token_lexer_lex_numeric_literal(c_token_lexer* self, c_sequence* seq)
{
        c_sequence_append(seq, self->c);
        while (1)
        {
                int c = c_token_lexer_readc(self);
                if (c_token_lexer_at_eof(self))
                        break;
                else if (c_char_is_digit(c))
                        ; // ok
                else if (c_char_is_letter(c))
                {
                        int u = toupper(c);
                        if ((u == 'E' || u == 'P') && (self->nextc == '+' || self->nextc == '-'))
                        {
                                if (!c_sequence_append(seq, c))
                                        return NULL;
                                c = c_token_lexer_readc(self);
                        }
                }
                else if (c == '.')
                        ; // ok
                else
                        break;

                if (!c_sequence_append(seq, c))
                        return NULL;
        }
        return c_token_new_pp_num(self->context, seq->loc, c_sequence_get_id(seq));
}

static c_token* c_token_lexer_lex_comment(c_token_lexer* self, tree_location start_loc)
{
        if (self->c == '/')
        {
                while (!c_token_lexer_at_eof(self) && self->c != '\n')
                        c_token_lexer_readc(self);
        }
        else if (self->c == '*')
        {
                while (1)
                {
                        c_token_lexer_readc(self);
                        if (c_token_lexer_at_eof(self))
                        {
                                c_error_unclosed_comment(self->context, start_loc);
                                return NULL;
                        }
                        else if (self->c == '*' && self->nextc == '/')
                        {
                                c_token_lexer_readc(self); // consume '*'
                                c_token_lexer_readc(self); // consume '/'
                                break;
                        }
                }
        }
        else
                UNREACHABLE();

        return c_token_new(self->context, CTK_COMMENT, start_loc);
}

static c_token* _c_token_lexer_lex_token(c_token_lexer* self)
{
        int c = self->c;

        c_sequence seq;
        c_sequence_init(&seq, self->context, self->loc);

        c_token_kind kind = CTK_UNKNOWN;
        tree_location loc = self->loc;

        switch (c)
        {
                case RB_ENDC:
                case '\0':
                        if (!self->eod_before_eof_returned && self->in_directive)
                        {
                                self->eod_before_eof_returned = true;
                                kind = CTK_EOD;
                        }
                        else
                                kind = CTK_EOF;
                        break;

                case ' ':
                case '\t':
                        return c_token_lexer_lex_spaces(self);

                case '\n':
                        c_token_lexer_readc(self);
                        kind = self->in_directive ? CTK_EOD : CTK_EOL;
                        break;

                case '_':
                case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
                case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
                case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
                case 'V': case 'W': case 'X': case 'Y': case 'Z':
                case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
                case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
                case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
                case 'v': case 'w': case 'x': case 'y': case 'z':
                        return c_token_lexer_lex_identifier(self, &seq);

                case '0': case '1': case '2': case '3': case '4': case '5':
                case '6': case '7': case '8': case '9':
                        return c_token_lexer_lex_numeric_literal(self, &seq);

                case '{': c_token_lexer_readc(self); kind = CTK_LBRACE; break;
                case '}': c_token_lexer_readc(self); kind = CTK_RBRACE; break;
                case '[': c_token_lexer_readc(self); kind = CTK_LSBRACKET; break;
                case ']': c_token_lexer_readc(self); kind = CTK_RSBRACKET; break;
                case '(': c_token_lexer_readc(self); kind = CTK_LBRACKET; break;
                case ')': c_token_lexer_readc(self); kind = CTK_RBRACKET; break;
                case ':': c_token_lexer_readc(self); kind = CTK_COLON; break;
                case '?': c_token_lexer_readc(self); kind = CTK_QUESTION; break;
                case ',': c_token_lexer_readc(self); kind = CTK_COMMA; break;
                case '~': c_token_lexer_readc(self); kind = CTK_TILDE; break;
                case ';': c_token_lexer_readc(self); kind = CTK_SEMICOLON; break;

                case '.':
                        if (self->nextc != RB_ENDC && c_char_is_digit(self->nextc))
                                return c_token_lexer_lex_numeric_literal(self, &seq);

                        c_token_lexer_readc(self);
                        if (self->c == '.')
                        {
                                if (self->nextc == '.')
                                {
                                        c_token_lexer_readc(self);
                                        c_token_lexer_readc(self);
                                        kind = CTK_ELLIPSIS;
                                }
                        }
                        else
                                kind = CTK_DOT;
                        break;

                case '-':
                        c_token_lexer_readc(self);
                        if (self->c == '>')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_ARROW;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_MINUS_EQ;
                        }
                        else if (self->c == '-')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_MINUS2;
                        }
                        else
                                kind = CTK_MINUS;
                        break;

                case '<':
                        if (self->angle_string_expected)
                                return c_token_lexer_lex_angle_string_literal(self, &seq);

                        c_token_lexer_readc(self);
                        if (self->c == '<')
                        {
                                c_token_lexer_readc(self);
                                if (self->c == '=')
                                {
                                        c_token_lexer_readc(self);
                                        kind = CTK_LE2_EQ;
                                }
                                else
                                        kind = CTK_LE2;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_LEQ;
                        }
                        else
                                kind = CTK_LE;
                        break;

                case '>':
                        c_token_lexer_readc(self);
                        if (self->c == '>')
                        {
                                c_token_lexer_readc(self);
                                if (self->c == '=')
                                {
                                        c_token_lexer_readc(self);
                                        kind = CTK_GR2_EQ;
                                }
                                else
                                        kind = CTK_GR2;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_GREQ;
                        }
                        else
                                kind = CTK_GR;
                        break;

                case '!':
                        c_token_lexer_readc(self);
                        if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_EXCLAIM_EQ;
                        }
                        else
                                kind = CTK_EXCLAIM;
                        break;

                case '=':
                        c_token_lexer_readc(self);
                        if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_EQ2;
                        }
                        else
                                kind = CTK_EQ;
                        break;

                case '&':
                        c_token_lexer_readc(self);
                        if (self->c == '&')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_AMP2;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_AMP_EQ;
                        }
                        else
                                kind = CTK_AMP;
                        break;

                case '|':
                        c_token_lexer_readc(self);
                        if (self->c == '|')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_VBAR2;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_VBAR_EQ;
                        }
                        else
                                kind = CTK_VBAR;
                        break;

                case '^':
                        c_token_lexer_readc(self);
                        if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_CARET_EQ;
                        }
                        else
                                kind = CTK_CARET;
                        break;

                case '+':
                        c_token_lexer_readc(self);
                        if (self->c == '+')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_PLUS2;
                        }
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_PLUS_EQ;
                        }
                        else
                                kind = CTK_PLUS;
                        break;

                case '*':
                        c_token_lexer_readc(self);
                        if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_STAR_EQ;
                        }
                        else
                                kind = CTK_STAR;
                        break;

                case '/':
                        c_token_lexer_readc(self);
                        if (self->c == '/' || self->c == '*')
                                return c_token_lexer_lex_comment(self, loc);
                        else if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_SLASH_EQ;
                        }
                        else
                                kind = CTK_SLASH;
                        break;

                case '%':
                        c_token_lexer_readc(self);
                        if (self->c == '=')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_PERCENT_EQ;
                        }
                        else
                                kind = CTK_PERCENT;
                        break;

                case '#':
                        c_token_lexer_readc(self);
                        if (self->c == '#')
                        {
                                c_token_lexer_readc(self);
                                kind = CTK_HASH2;
                        }
                        else if (self->hash_expected || self->in_directive)
                                kind = CTK_HASH;
                        break;

                case '\"':
                        return c_token_lexer_lex_string_literal(self, &seq);

                case '\'':
                        return c_token_lexer_lex_character_constant(self, &seq);

                default:
                        break;
        }

        if (kind == CTK_UNKNOWN)
        {
                c_error_stray_symbol(self->context, loc, c);
                return NULL;
        }

        return c_token_new(self->context, kind, loc);
}

extern c_token* c_token_lexer_lex_token(c_token_lexer* self)
{
        c_token* t = _c_token_lexer_lex_token(self);
        if (!t)
                return NULL;

        c_token_kind k = c_token_get_kind(t);
        if (k == CTK_EOL || k == CTK_EOD)
        {
                self->hash_expected = true;
        }
        else if (k != CTK_WSPACE && k != CTK_COMMENT)
        {
                if (k == CTK_HASH && !self->hash_expected && !self->in_directive)
                {
                        c_error_stray_symbol(self->context, c_token_get_loc(t), '#');
                        return NULL;
                }
                self->hash_expected = false;
        }
        return t;
}