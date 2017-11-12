#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-error.h"
#include "scc/c/c-tree.h"
#include "scc/scl/char-info.h"

extern void creswords_init(creswords* self)
{
        htab_init_i32(&self->reswords);
        htab_init_i32(&self->pp_reswords);
}

extern void creswords_dispose(creswords* self)
{
        htab_dispose(&self->reswords);
        htab_dispose(&self->pp_reswords);
}

extern serrcode creswords_add(creswords* self, const char* string, ctoken_kind k)
{
        return htab_insert_i32(&self->reswords, STRREF(string), k);
}

extern serrcode creswords_add_pp(creswords* self, const char* string, ctoken_kind k)
{
        return htab_insert_i32(&self->pp_reswords, STRREF(string), k);
}

extern ctoken_kind creswords_get(const creswords* self, const char* string, ssize len)
{
        return creswords_get_h(self, STRREFL(string, len));
}

extern ctoken_kind creswords_get_h(const creswords* self, hval h)
{
        hiter res;
        if (!htab_find(&self->reswords, h, &res))
                return CTK_UNKNOWN;

        return hiter_get_i32(&res);
}

extern ctoken_kind creswords_get_pp(const creswords* self, const char* string, ssize len)
{
        return creswords_get_pp_h(self, STRREFL(string, len));
}

extern ctoken_kind creswords_get_pp_h(const creswords* self, hval h)
{
        hiter res;
        if (!htab_find(&self->pp_reswords, h, &res))
                return CTK_UNKNOWN;

        return hiter_get_i32(&res);
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
        cerror_manager* error_manager,
        ctree_context* context)
{
        self->reswords = reswords;
        self->context = context;
        self->error_manager = error_manager;
        self->source = NULL;
        self->source_manager = source_manager;
        self->buf = NULL;
        self->tab_to_space = 4;
        self->chars[0] = RB_ENDC;
        self->chars[1] = RB_ENDC;
        self->chars[2] = RB_ENDC;
        self->loc = TREE_INVALID_LOC;
}

extern serrcode cpplexer_enter_source_file(cpplexer* self, csource* source)
{
        if (!source)
                return S_ERROR;
        
        if (!(self->buf = csource_open(source)))
        {
                cerror(self->error_manager, CES_ERROR, 0,
                        "cannot open source file %s", csource_get_name(source));
                return S_ERROR;
        }
        // save first line location
        if (S_FAILED(csource_save_line_loc(source, csource_begin(source))))
                return S_ERROR;

        // fill buffer
        cpplexer_readc(self);
        cpplexer_readc(self);
        self->loc = csource_begin(source);
        self->source = source;
        return S_NO_ERROR;
}

typedef struct 
{
        ssize size;
        tree_location loc;
        char val[CMAX_LINE_LENGTH + 1];
} csequence;

static inline void csequence_finish(csequence* self)
{
        self->val[self->size] = '\0';
}

static void csequence_init(csequence* self)
{
        self->size = 0;
        self->loc = 0;
        csequence_finish(self);
}

static inline bool csequence_append(csequence* self, const cpplexer* lexer, int c)
{
        if (self->size == CMAX_LINE_LENGTH)
        {
                cerror(lexer->error_manager, CES_ERROR, self->loc,
                        "token overflowed internal buffer");
                return false;
        }

        self->val[self->size++] = (char)c;
        return true;
}

static tree_id cpplexer_pool_seq(cpplexer* self, csequence* seq)
{
        tree_id id = ctree_context_add_string(self->context, seq->val, seq->size);
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
                        cerror(self->error_manager, CES_ERROR, seq->loc, "missing closing quote");
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

extern ctoken* cpplex_const_char(cpplexer* self)
{
        csequence cchar;
        csequence_init(&cchar);
        if (!cpplex_quoted_seq(self, &cchar, '\''))
                return NULL;

        bool escape = cchar.val[0] == '\\';
        if (cchar.size == 0)
        {
                cerror(self->error_manager, CES_ERROR, cchar.loc, "empty character constant");
                return NULL;
        }
        else if ((escape && cchar.size > 2) || (!escape && cchar.size > 1))
        {
                cerror(self->error_manager, CES_ERROR, cchar.loc, "invalid character constant");
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
                                cerror(self->error_manager, CES_ERROR, loc,
                                        "comment unclosed at end of file");
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
                cerror(self->error_manager, CES_ERROR, loc,
                        "unknown punctuator '%c'", cpplexer_getc(self));
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

        cerror(self->error_manager, CES_ERROR, cpplexer_loc(self),
                "unknown symbol '%c'", c);
        return NULL;
}

extern void cpproc_init(
        cpproc* self,
        const creswords* reswords,
        csource_manager* source_manager,
        cerror_manager* error_manager,
        ctree_context* context)
{
        self->reswords = reswords;
        self->state = self->files - 1;
        self->source_manager = source_manager;
        self->error_manager = error_manager;
        self->context = context;
        dseq_init_ex_ptr(&self->expansion,
                tree_get_context_allocator(ctree_context_base(context)));
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
                cerror(self->error_manager, CES_ERROR, 0,
                        "maximum include nesting is %d", CMAX_INCLUDE_NESTING);
                return S_ERROR;
        }

        cpproc_state* next = self->state + 1;
        cpplexer* pplexer = &next->lexer;
        cpplexer_init(
                pplexer,
                self->reswords,
                self->source_manager,
                self->error_manager, 
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
        ctoken* t = cpproc_lex_wspace(self, false);
        if (!t)
                return NULL;

        if (!ctoken_is(t, CTK_CONST_STRING) && !ctoken_is(t, CTK_ANGLE_STRING))
        {
                cerror(self->error_manager, CES_ERROR, ctoken_get_loc(t),
                        "expected a file name");
                return NULL;
        }

        tree_id ref = ctoken_get_string(t);
        const char* filename = tree_context_get_id(ctree_context_base(self->context), ref);
        S_ASSERT(filename);

        if (!*filename)
        {
                cerror(self->error_manager, CES_ERROR, ctoken_get_loc(t),
                        "empty file name in #include");
                return NULL;
        }

        csource* source = csource_find(self->source_manager, filename);
        if (!source)
        {
                cerror(self->error_manager, CES_ERROR, ctoken_get_loc(t),
                        "cannot open source file \"%s\"", filename);
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
                cerror(self->error_manager, CES_ERROR, ctoken_get_loc(t),
                        "'#' is not expected here");
                return NULL;
        }

        if (!(t = cpproc_lex_wspace(self, false)))
                return NULL;

        ctoken_kind directive = CTK_UNKNOWN;
        if (ctoken_is(t, CTK_ID))
                directive = creswords_get_pp_h(self->reswords, ctoken_get_string(t));

        if (directive == CTK_UNKNOWN)
        {
                cerror(self->error_manager, CES_ERROR, ctoken_get_loc(t),
                        "unkown preprocessing directive");
                return false;
        }

        switch (directive)
        {
                case CTK_PP_INCLUDE: return cpproc_lex_include(self);

                default:
                        S_UNREACHABLE(); // todo
                        return NULL;
        }
}

static ctoken* cpproc_lex_macro_id(cpproc* self)
{
        if (dseq_size(&self->expansion))
                return dseq_pop_ptr(&self->expansion);

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
        dseq_append_ptr(&self->expansion, t);
}

static ctoken* cpproc_lex_string(cpproc* self)
{
        ctoken* t = cpproc_lex_macro_id(self);
        if (!t)
                return NULL;

        if (!ctoken_is(t, CTK_CONST_STRING))
                return t;

        dseq buf;
        dseq_init_ex_ptr(&buf, tree_get_context_allocator(ctree_context_base(self->context)));
        while (ctoken_is(t, CTK_CONST_STRING))
        {
                dseq_append_ptr(&buf, t);
                if (!(t = cpproc_lex_macro_id(self)))
                        return NULL;
                if (!ctoken_is(t, CTK_CONST_STRING))
                {
                        cpproc_unget_macro_id(self, t);
                        break;
                }
        }

        //todo:
        char concat[4096];
        *concat = '\0';
        for (ssize i = 0; i < dseq_size(&buf); i++)
        {
                tree_id ref = ctoken_get_string(dseq_get_ptr(&buf, i));
                const char* s = tree_context_get_id(ctree_context_base(self->context), ref);
                strcat(concat, s);
        }

        tree_id ref = ctree_context_add_string(self->context, concat, strlen(concat));
        t = dseq_first_ptr(&buf);
        ctoken_set_string(t, ref);
        dseq_dispose(&buf);
        return t;
}

extern ctoken* cpreprocess(cpproc* self)
{
        return cpproc_lex_string(self);
}