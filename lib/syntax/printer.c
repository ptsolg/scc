#include "scc/syntax/printer.h"
#include "scc/c-common/context.h"
#include "scc/c-common/source.h"
#include "scc/c-common/limits.h"
#include "scc/core/buf-io.h"
#include "scc/core/list.h"
#include "scc/core/num.h"
#include "scc/lex/charset.h"
#include "scc/lex/token.h"
#include "scc/lex/reswords-info.h"
#include "scc/lex/misc.h"
#include "scc/tree/tree.h"
#include <stdarg.h>
#include <math.h>
#include <stdio.h>

extern void c_printer_opts_init(c_printer_opts* self)
{
        self->print_expr_type = false;
        self->print_expr_value = false;
        self->print_impl_casts = false;
        self->print_eval_result = false;
        self->print_semantic_init = false;
        self->double_precision = 4;
        self->float_precision = 4;
        self->force_brackets = false;
}

extern void c_printer_init(c_printer* self, const c_context* context, FILE* fout)
{
        self->context = context->tree;
        self->ccontext = context;
        self->source_manager = &context->source_manager;
        self->target = self->context->target;
        self->indent_level = 0;
        init_buf_writer(&self->buf, fout);
        c_printer_opts_init(&self->opts);
}

extern void c_printer_dispose(c_printer* self)
{
        drop_buf_writer(&self->buf);
}

static inline void c_prints(c_printer* self, const char* s)
{
        if (s)
                buf_write_str(&self->buf, s);
}

static inline void c_printf(c_printer* self, const char* f, ...)
{
        char buf[1024];

        va_list args;
        va_start(args, f);
        vsnprintf(buf, 1024, f, args);

        c_prints(self, buf);
}

static inline void c_print_float(c_printer* self, float v)
{
        c_printf(self, "%.*ff", self->opts.float_precision, v);
}

static inline void c_print_double(c_printer* self, double v)
{
        c_printf(self, "%.*f", self->opts.double_precision, v);
}

static inline void c_print_d(c_printer* self, int v)
{
        c_printf(self, "%d", v);
}

static inline void c_print_lld(c_printer* self, int64_t v)
{
        c_printf(self, "%lld", v);
        c_prints(self, "LL");
}

static inline void c_print_u(c_printer* self, uint v)
{
        c_printf(self, "%u", v);
        c_prints(self, "U");
}

static inline void c_print_llu(c_printer* self, uint64_t v)
{
        c_printf(self, "%llu", v);
        c_prints(self, "ULL");
}

static inline void c_printc(c_printer* self, int c)
{
        buf_write_char(&self->buf, c);
}

static inline void c_printcn(c_printer* self, int c, int n)
{
        while (n--)
                c_printc(self, c);
}

static inline void c_print_id(c_printer* self, tree_id id)
{
        c_prints(self, tree_get_id_string(self->context, id));
}

static inline void c_print_space(c_printer* self)
{
        c_printc(self, ' ');
}

static inline void c_print_indent(c_printer* self)
{
        c_printcn(self, '\t', self->indent_level);
}

static inline void c_printer_add_indent(c_printer* self)
{
        self->indent_level++;
}

static inline void c_printer_sub_indent(c_printer* self)
{
        assert(self->indent_level);
        self->indent_level--;
}

static inline void c_print_endl(c_printer* self)
{
        c_printc(self, '\n');
        c_print_indent(self);
}

static inline void c_printrw(c_printer* self, c_token_kind k)
{
        c_prints(self, c_get_token_kind_info(k)->string);
}

extern void c_token_print_info_init(c_token_print_info* self)
{
        self->max_col = 0;
        self->max_file_len = 0;
        self->max_kind_len = 0;
        self->max_line = 0;
}

static void _c_print_string_literal(c_printer* self, tree_id id)
{
        struct strentry* entry = tree_get_id_strentry(self->context, id);
        char unescaped[C_MAX_LINE_LENGTH * 2];
        c_get_unescaped_string(unescaped, ARRAY_SIZE(unescaped), (const char*)entry->data, entry->size);
        c_printf(self, "\"%s\"", unescaped);
}

static void _c_print_char_literal(c_printer* self, int c)
{
        if (c_char_is_escape(c))
                c_printf(self, "'\\%c'", c_char_from_escape(c));
        else
                c_printf(self, "'%c'", c);
}

static void c_print_token_value(c_printer* self, const c_token* token)
{
        c_token_kind k = c_token_get_kind(token);

        if (k == CTK_ID)
                c_print_id(self, c_token_get_string(token));
        else if (k == CTK_CONST_INT)
        {
                c_printf(self, "%llu", c_token_get_int(token));
                int ls = c_token_get_int_ls(token);
                while (ls--)
                        c_printc(self, 'L');
                if (!c_token_int_is_signed(token))
                        c_printc(self, 'U');
        }
        else if (k == CTK_CONST_FLOAT)
                c_printf(self, "%ff", c_token_get_float(token));
        else if (k == CTK_CONST_DOUBLE)
                c_printf(self, "%f", (double)c_token_get_double(token));
        else if (k == CTK_CONST_STRING)
                _c_print_string_literal(self, c_token_get_string(token));
        else if (k == CTK_CONST_CHAR)
                _c_print_char_literal(self, c_token_get_char(token));
        c_print_endl(self);
}

static char* strfill(char* string, int v, size_t n)
{
        memset(string, v, n);
        string[n] = '\0';
        return string;
}

static char* append_chars(char* string, int v, size_t n)
{
        return strfill(string + strlen(string), v, n);
}

static inline int ndigits(int n)
{
        return n ? (int)log10(n) + 1 : 1;
}

extern void c_print_token(c_printer* self, const c_token* token, const c_token_print_info* info)
{
        c_location loc;
        c_source_find_loc(self->source_manager, &loc, c_token_get_loc(token));

        char file[MAX_PATH_LEN];
        file[0] = '\0';

        char col[32];
        sprintf(col, "%d", loc.column);
        append_chars(col, ' ', 1 + info->max_col - ndigits(loc.column));

        char line[32];
        sprintf(line, "%d", loc.line);
        append_chars(line, ' ', 1 + info->max_line - ndigits(loc.line));

        const char* kind = c_get_token_kind_string(c_token_get_kind(token));
        char trail[32];
        strfill(trail, ' ', 1 + info->max_kind_len - strlen(kind));

        c_printf(self, "%s%s%s%s%s", file, line, col, kind, trail);
        c_print_token_value(self, token);
}

extern void c_print_tokens(c_printer* self, const struct vec* tokens)
{
        c_token_print_info info;
        c_token_print_info_init(&info);

        for (size_t i = 0; i < tokens->size; i++)
        {
                const c_token* token = vec_get(tokens, i);
                c_location loc;
                c_source_find_loc(self->source_manager, &loc, c_token_get_loc(token));

                if (ndigits(loc.line) > info.max_line)
                        info.max_line = ndigits(loc.line);
                if (ndigits(loc.column) > info.max_col)
                        info.max_col = ndigits(loc.column);

                int len = (int)strlen(pathfile(loc.file));
                if (len > info.max_file_len)
                        info.max_file_len = len;

                len = (int)strlen(c_get_token_kind_string(c_token_get_kind(token)));
                if (len > info.max_kind_len)
                        info.max_kind_len = len;
        }
        for (size_t i = 0; i < tokens->size; i++)
                c_print_token(self, vec_get(tokens, i), &info);
}

static inline void c_print_lbracket(c_printer* self)
{
        c_printrw(self, CTK_LBRACKET);
}

static inline void c_print_rbracket(c_printer* self)
{
        c_printrw(self, CTK_RBRACKET);
}

static inline void c_print_lsbracket(c_printer* self)
{
        c_printrw(self, CTK_LSBRACKET);
}

static inline void c_print_rsbracket(c_printer* self)
{
        c_printrw(self, CTK_RSBRACKET);
}

static inline void c_print_lbrace(c_printer* self, bool newline)
{
        if (newline)
        {
                c_print_endl(self);
                c_printer_add_indent(self);
        }
        c_printrw(self, CTK_LBRACE);
}

static inline void c_print_rbrace(c_printer* self, bool newline)
{
        if (newline)
        {
                c_printer_sub_indent(self);
                c_print_endl(self);
        }
        c_printrw(self, CTK_RBRACE);
}

static inline void c_print_comma(c_printer* self)
{
        c_printrw(self, CTK_COMMA);
        c_print_space(self);
}

static inline void c_print_decl_name(c_printer* self, const tree_decl* d)
{
        c_print_id(self, tree_get_decl_name(d));
}

static const char* binop2str[] = {
        "" , // TBK_UNKNOWN
        "*" , // TBK_MUL
        "/" , // TBK_DIV
        "%" , // TBK_MOD
        "+" , // TBK_ADD
        "-" , // TBK_SUB
        "<<" , // TBK_SHL
        ">>" , // TBK_SHR
        "<" , // TBK_LE
        ">" , // TBK_GR
        "<=" , // TBK_LEQ
        ">=" , // TBK_GEQ
        "==" , // TBK_EQ
        "!=" , // TBK_NEQ
        "&" , // TBK_AND
        "^" , // TBK_XOR
        "|" , // TBK_OR
        "&&" , // TBK_LOG_AND
        "||" , // TBK_LOG_OR
        "=" , // TBK_ASSIGN
        "+=" , // TBK_ADD_ASSIGN
        "-=" , // TBK_SUB_ASSIGN
        "*=" , // TBK_MUL_ASSIGN
        "/=" , // TBK_DIV_ASSIGN
        "%=" , // TBK_MOD_ASSIGN
        "<<=" , // TBK_SHL_ASSIGN
        ">>=" , // TBK_SHR_ASSIGN
        "&=" , // TBK_AND_ASSIGN
        "^=" , // TBK_XOR_ASSIGN
        "|=" , // TBK_OR_ASSIGN
        "," , // TBK_COMMA
};

static void c_print_binop(c_printer* self, const tree_expr* expr)
{
        c_print_expr(self, tree_get_binop_lhs(expr));
        c_print_space(self);
        c_prints(self, binop2str[tree_get_binop_kind(expr)]);
        c_print_space(self);
        c_print_expr(self, tree_get_binop_rhs(expr));
}

typedef struct
{
        const char* string;
} c_unop_info;

static_assert(11 == TUK_SIZE, "cunop_info_table needs update.");
static const c_unop_info c_unop_info_table[TBK_SIZE] =
{
        { "" }, //TUK_UNKNOWN
        { "++" }, //TUK_POST_INC
        { "--" }, //TUK_POST_DEC
        { "++" }, //TUK_PRE_INC
        { "--" }, //TUK_PRE_DEC
        { "+" }, //TUK_PLUS
        { "-" }, //TUK_MINUS
        { "~" }, //TUK_NOT
        { "!" }, //TUK_LOG_NOT
        { "*" }, //TUK_DEREFERENCE
        { "&" }, //TUK_ADDRESS
};

static void c_print_unop(c_printer* self, const tree_expr* expr)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        const tree_expr* e = tree_get_unop_operand(expr);
        const char* unop = c_unop_info_table[k].string;

        if (k == TUK_POST_INC || k == TUK_POST_DEC)
        {
                c_print_expr(self, e);
                c_prints(self, unop);
        }
        else
        {
                c_prints(self, unop);
                c_print_expr(self, e);
        }
}

static inline bool c_printer_postfix_expr_needs_wrapping(const c_printer* self, const tree_expr* e)
{
        return tree_get_expr_kind(e) == TEK_CAST
                && tree_cast_is_implicit(e) && self->opts.print_impl_casts;
}

static void _c_print_expr(c_printer*, const tree_expr*, bool);

static void c_print_call_expr(c_printer* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_call_lhs(expr);
        _c_print_expr(self, lhs, c_printer_postfix_expr_needs_wrapping(self, lhs));

        c_print_lbracket(self);
        TREE_FOREACH_CALL_ARG(expr, arg)
        {
                c_print_expr(self, *arg);
                if (arg + 1 != tree_get_call_args_end(expr))
                        c_print_comma(self);
        }
        c_print_rbracket(self);
}

static void c_print_subscript_expr(c_printer* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_subscript_lhs(expr);
        _c_print_expr(self, lhs, c_printer_postfix_expr_needs_wrapping(self, lhs));
        c_print_lsbracket(self);
        c_print_expr(self, tree_get_subscript_rhs(expr));
        c_print_rsbracket(self);
}

static void c_print_conditional_expr(c_printer* self, const tree_expr* expr)
{
        c_print_expr(self, tree_get_conditional_condition(expr));
        c_print_space(self);
        c_printrw(self, CTK_QUESTION);
        c_print_space(self);
        c_print_expr(self, tree_get_conditional_lhs(expr));
        c_print_space(self);
        c_printrw(self, CTK_COLON);
        c_print_space(self);
        c_print_expr(self, tree_get_conditional_rhs(expr));
}

static void c_print_decl_expr(c_printer* self, const tree_expr* expr)
{
        c_print_decl(self, tree_get_decl_expr_entity(expr),
                CPRINT_DECL_NAME | CPRINTER_IGNORE_DECL_ENDING);
}

static void c_print_floating_literal(c_printer* self, const tree_expr* expr)
{
        const struct num* v = tree_get_floating_literal_cvalue(expr);
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                c_print_float(self, num_f32(v));
        else
                c_print_double(self, num_f64(v));
}

static void c_print_integer(c_printer* self, const tree_type* t, uint64_t v)
{
        assert(tree_type_is_integer(t));
        bool sign = !tree_type_is_unsigned_integer(t);
        bool ext = tree_builtin_type_is(t, TBTK_UINT64)
                || tree_builtin_type_is(t, TBTK_INT64);

        if (sign && ext)
                c_print_lld(self, (int64_t)v);
        else if (!sign && ext)
                c_print_llu(self, v);
        else if (!sign)
                c_print_u(self, (uint)v);
        else
                c_print_d(self, (int)v);
}

static void c_print_integer_literal(c_printer* self, const tree_expr* expr)
{
        c_print_integer(self, tree_get_expr_type(expr), tree_get_integer_literal(expr));
}

static void c_print_char_literal(c_printer* self, const tree_expr* expr)
{
        _c_print_char_literal(self, tree_get_character_literal(expr));
}

static void c_print_string_literal(c_printer* self, const tree_expr* expr)
{
        _c_print_string_literal(self, tree_get_string_literal(expr));
}

static void c_print_member_expr(c_printer* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_member_expr_lhs(expr);
        _c_print_expr(self, lhs, c_printer_postfix_expr_needs_wrapping(self, lhs));

        const tree_decl* decl = tree_get_member_expr_decl(expr);
        if (tree_decl_is_anon(decl))
                return;

        c_printrw(self, (tree_member_expr_is_arrow(expr) ? CTK_ARROW : CTK_DOT));
        c_print_decl(self, decl, CPRINT_DECL_NAME);
}

static void c_print_cast_expr(c_printer* self, const tree_expr* expr)
{
        const tree_expr* e = tree_get_cast_operand(expr);
        bool implicit = tree_cast_is_implicit(expr);
        if (!implicit || self->opts.print_impl_casts)
        {
                c_print_lbracket(self);
                int opts = implicit ? CPRINT_IMPL_TYPE_BRACKETS : CPRINT_OPTS_NONE;
                c_print_type_name(self, tree_get_expr_type(expr), opts);
                c_print_rbracket(self);
        }

        bool brackets = false;
        if (implicit)
        {
                brackets = (self->opts.print_impl_casts && tree_get_expr_kind(e) == TEK_BINARY)
                        || self->opts.force_brackets;
        }
        _c_print_expr(self, e, brackets);
}

static void c_print_sizeof_expr(c_printer* self, const tree_expr* expr)
{
        c_printrw(self, CTK_SIZEOF);
        if (tree_sizeof_contains_type(expr))
        {
                c_print_lbracket(self);
                c_print_type_name(self, tree_get_sizeof_type(expr), CPRINT_OPTS_NONE);
                c_print_rbracket(self);
        }
        else
        {
                c_print_space(self);
                c_print_expr(self, tree_get_sizeof_expr(expr));
        }
}

static void c_print_offsetof_expr(c_printer* self, const tree_expr* expr)
{
        c_printrw(self, CTK_OFFSETOF);
        c_print_lbracket(self);
        c_print_type_name(self, tree_get_offsetof_record(expr), CPRINT_OPTS_NONE);
        c_print_comma(self);
        c_print_id(self, tree_get_decl_name(tree_get_offsetof_field(expr)));
        c_print_rbracket(self);
}

static void c_print_paren_expr(c_printer* self, const tree_expr* expr)
{
        c_print_lbracket(self);
        _c_print_expr(self, tree_get_paren_expr(expr), false);
        c_print_rbracket(self);
}

static void c_print_designation(c_printer* self, const tree_expr* d)
{
        TREE_FOREACH_DESIGNATION_DESIGNATOR(d, it, end)
        {
                tree_designator_kind dk = tree_get_designator_kind(*it);
                if (dk == TREE_DESIGNATOR_INDEX)
                {
                        c_print_lsbracket(self);
                        c_print_expr(self, tree_get_designator_index(*it));
                        c_print_rsbracket(self);
                        continue;
                }

                tree_id name = dk == TREE_DESIGNATOR_FIELD_NAME
                        ? tree_get_designator_field_name(*it)
                        : tree_get_decl_name(tree_get_designator_field(*it));
                c_printrw(self, CTK_DOT);
                c_print_id(self, name);
        }

        c_print_space(self);
        c_printrw(self, CTK_EQ);
        c_print_space(self);
        c_print_expr(self, tree_get_designation_init(d));
}

static void c_print_initializer(c_printer* self, const tree_expr* expr)
{
        c_print_lbrace(self, false);
        TREE_FOREACH_INIT_LIST_EXPR(expr, it, end)
        {
                c_print_expr(self, *it);
                if (it + 1 != end )
                        c_print_comma(self);
        }
        if (tree_init_list_has_trailing_comma(expr))
                c_print_comma(self);
        c_print_rbrace(self, false);
}

static void _c_print_expr(c_printer* self, const tree_expr* e, bool print_brackets)
{
        if (!e)
                return;

        if (print_brackets)
                c_print_lbracket(self);
        switch (tree_get_expr_kind(e))
        {
                case TEK_BINARY: c_print_binop(self, e); break;
                case TEK_UNARY: c_print_unop(self, e); break;
                case TEK_CALL: c_print_call_expr(self, e); break;
                case TEK_SUBSCRIPT: c_print_subscript_expr(self, e); break;
                case TEK_CONDITIONAL: c_print_conditional_expr(self, e); break;
                case TEK_CHARACTER_LITERAL: c_print_char_literal(self, e); break;
                case TEK_FLOATING_LITERAL: c_print_floating_literal(self, e); break;
                case TEK_INTEGER_LITERAL: c_print_integer_literal(self, e); break;
                case TEK_STRING_LITERAL: c_print_string_literal(self, e); break;
                case TEK_DECL: c_print_decl_expr(self, e); break;
                case TEK_MEMBER: c_print_member_expr(self, e); break;
                case TEK_CAST: c_print_cast_expr(self, e); break;
                case TEK_SIZEOF: c_print_sizeof_expr(self, e); break;
                case TEK_OFFSETOF: c_print_offsetof_expr(self, e); break;
                case TEK_PAREN: c_print_paren_expr(self, e); break;
                case TEK_INIT_LIST: c_print_initializer(self, e); break;
                case TEK_DESIGNATION: c_print_designation(self, e); break;

                default:
                        ; // unknown exp kind
        }
        if (print_brackets)
                c_print_rbracket(self);
}

extern void c_print_expr(c_printer* self, const tree_expr* expr)
{
        if (!expr)
                return;

        tree_expr_kind k = tree_get_expr_kind(expr);
        bool is_primitive = k == TEK_DECL
                || k == TEK_PAREN
                || tree_expr_is_literal(expr)
                || k == TEK_CAST;

        bool brackets = self->opts.force_brackets && !is_primitive;
        _c_print_expr(self, expr, brackets);
}

static void c_print_type_quals(c_printer* self, tree_type_quals q)
{
        if (q == TTQ_UNQUALIFIED)
                return;

        if (q & TTQ_CONST)
        {
                c_printrw(self, CTK_CONST);
                c_print_space(self);
        }
        if (q & TTQ_RESTRICT)
        {
                c_printrw(self, CTK_RESTRICT);
                c_print_space(self);
        }
        if (q & TTQ_VOLATILE)
        {
                c_printrw(self, CTK_VOLATILE);
                c_print_space(self);
        }
}

typedef struct
{
        const tree_decl* decl;
        int n_type_parts;
        int n_suffixes;
        bool name_printed;
        bool has_brackets;
        bool att_emited;
        const tree_type* type_parts[100]; // todo: size
        const tree_type* suffixes[100];
} c_type_name_info;

static void c_type_name_info_init(
        c_type_name_info* self,
        const tree_type* type,
        const tree_decl* decl)
{
        self->decl = decl;
        self->n_type_parts = 0;
        self->n_suffixes = 0;
        self->name_printed = false;
        self->att_emited = false;
        self->has_brackets = false;

        const tree_type* it = type;
        if (decl && tree_decl_is(decl, TDK_FUNCTION))
                it = tree_get_func_type_result(tree_get_decl_type(decl));

        while (it)
        {
                if (tree_type_is(it, TTK_ADJUSTED))
                        it = tree_get_original_type(it);

                self->type_parts[self->n_type_parts++] = it;
                if (tree_type_is(it, TTK_PAREN))
                        self->has_brackets = true;
                it = tree_get_type_next(it);
        }
}

typedef struct
{
        const char* string;
} c_builtin_type_info;

static_assert(12 == TBTK_SIZE, "cbuiltin_type_info_table needs update.");
static const c_builtin_type_info c_builtin_type_info_table[TBTK_SIZE] =
{
        { "" }, // TBTK_INVALID
        { "void" }, // TBTK_VOID
        { "char" }, // TBTK_INT8
        { "unsigned char" }, // TBTK_UINT8
        { "short" }, // TBTK_INT16
        { "unsigned short" }, // TBTK_UINT16
        { "int" }, // TBTK_INT32
        { "unsigned" }, // TBTK_UINT32
        { "long long" }, // TBTK_INT64
        { "unsigned long long" }, // TBTK_UINT64
        { "float" }, // TBTK_FLOAT
        { "double" }, // TBTK_DOUBLE
};

static void c_print_builtin_type(c_printer* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;

        c_print_type_quals(self, tree_get_type_quals(t));
        c_prints(self, c_builtin_type_info_table[tree_get_builtin_type_kind(t)].string);
}

static void c_print_decl_type(c_printer* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;
        if (tree_decl_type_is_referenced(t))
                opts |= CPRINT_DECL_NAME;

        c_print_type_quals(self, tree_get_type_quals(t));
        c_print_decl(self, tree_get_decl_type_entity(t), opts);
}

static void c_print_function_type_attributes(c_printer* self, const tree_type* t)
{
        if (tree_func_type_is_transaction_safe(t))
        {
                c_print_space(self);
                c_printrw(self, CTK_TRANSACTION_SAFE);
        }
        tree_calling_convention cc = tree_get_func_type_cc(t);
        if (cc == TCC_STDCALL)
        {
                c_print_space(self);
                c_printrw(self, CTK_STDCALL);
        }
}

static void c_print_type_parts(c_printer* self, c_type_name_info* info, int opts)
{
        int part_it = info->n_type_parts;
        while (part_it)
        {
                const tree_type* t = info->type_parts[--part_it];
                tree_type_kind k = tree_get_type_kind(t);
                const tree_type* next = NULL;
                if (part_it)
                        next = info->type_parts[part_it - 1];

                if (k == TTK_BUILTIN)
                        c_print_builtin_type(self, t, opts);
                else if (k == TTK_DECL)
                        c_print_decl_type(self, t, opts);
                else if (k == TTK_ARRAY || k == TTK_FUNCTION || k == TTK_PAREN)
                {
                        if (k == TTK_PAREN || opts & CPRINT_IMPL_TYPE_BRACKETS)
                                c_print_lbracket(self);
                        info->suffixes[info->n_suffixes++] = t;
                }
                else if (k == TTK_POINTER)
                {
                        c_printrw(self, CTK_STAR);
                        c_print_type_quals(self, tree_get_type_quals(t));
                }
                else
                        ; // unknown type kind

                if (next && tree_get_type_next(next))
                        continue;

                if (!info->name_printed && info->decl)
                {
                        c_print_space(self);
                        c_print_id(self, tree_get_decl_name(info->decl));

                        if (!tree_decl_is(info->decl, TDK_FUNCTION))
                                continue;

                        const tree_decl_scope* params = tree_get_func_cparams(info->decl);

                        c_print_lbracket(self);
                        TREE_FOREACH_DECL_IN_SCOPE(params, param)
                        {
                                c_print_decl(self, param, CPRINTER_IGNORE_DECL_ENDING);
                                const tree_decl* next = tree_get_next_decl(param);
                                if (next != tree_get_decl_scope_decls_cend(params))
                                        c_print_comma(self);
                        }

                        tree_type* func_type = tree_get_decl_type(info->decl);
                        if (tree_func_type_is_vararg(func_type))
                        {
                                c_print_comma(self);
                                c_printrw(self, CTK_ELLIPSIS);
                        }
                        c_print_rbracket(self);
                        c_print_function_type_attributes(self, func_type);
                }
        }
}

static void c_print_suffix_endings(c_printer* self, c_type_name_info* info, int opts)
{
        int suffix_it = info->n_suffixes;
        while (suffix_it)
        {
                const tree_type* t = info->suffixes[--suffix_it];
                tree_type_kind k = tree_get_type_kind(t);
                if (opts & CPRINT_IMPL_TYPE_BRACKETS)
                        c_print_rbracket(self);

                if (k == TTK_FUNCTION)
                {
                        c_print_lbracket(self);
                        TREE_FOREACH_FUNC_TYPE_PARAM(t, param)
                        {
                                c_print_type_name(self, *param, CPRINT_OPTS_NONE);
                                if (param + 1 != tree_get_func_type_params_end(t))
                                        c_print_comma(self);
                        }
                        if (tree_func_type_is_vararg(t))
                        {
                                c_print_comma(self);
                                c_printrw(self, CTK_ELLIPSIS);
                        }
                        c_print_rbracket(self);
                        c_print_function_type_attributes(self, t);
                }
                else if (k == TTK_ARRAY)
                {
                        c_print_lsbracket(self);
                        if (tree_array_is(t, TAK_CONSTANT))
                                c_print_expr(self, tree_get_array_size_expr(t));
                        c_print_rsbracket(self);
                }
                else if (k == TTK_PAREN)
                        c_print_rbracket(self);
        }
}

static void _c_print_type_name(
        c_printer* self,
        const tree_type* type,
        const tree_decl* decl,
        int opts)
{
        if (!type)
                return;

        c_type_name_info info;
        c_type_name_info_init(&info, type, decl);
        c_print_type_parts(self, &info, opts);
        c_print_suffix_endings(self, &info, opts);
}

extern void c_print_type_name(c_printer* self, const tree_type* type, int opts)
{
        _c_print_type_name(self, type, NULL, opts);
}

static void c_print_decl_scope(c_printer* self, const tree_decl_scope* scope, bool braces, int opts)
{
        if (!scope)
                return;

        if (braces)
                c_print_lbrace(self, true);
        TREE_FOREACH_DECL_IN_SCOPE(scope, decl)
                if (!tree_decl_is_implicit(decl))
                {
                        c_print_endl(self);
                        c_print_decl(self, decl, CPRINT_OPTS_NONE);
                }
        if (braces)
                c_print_rbrace(self, true);
        c_print_endl(self);
}

static void c_print_typedef(c_printer* self, const tree_decl* decl, int opts)
{
        tree_id name = tree_get_decl_name(decl);
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_id(self, name);
                return;
        }

        if (!(opts & CPRINTER_IGNORE_STORAGE_SPECS))
        {
                c_printrw(self, CTK_TYPEDEF);
                c_print_space(self);
        }

        _c_print_type_name(self, tree_get_decl_type(decl), decl, opts);
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_printrw(self, CTK_SEMICOLON);
}

static void c_print_struct_or_union_specifier(c_printer* self, const tree_decl* record, int opts)
{
        c_printrw(self, (tree_record_is_union(record) ? CTK_UNION : CTK_STRUCT));
        c_print_space(self);
        tree_expr* alignment = tree_get_record_alignment(record);
        bool is_ref = opts & CPRINT_DECL_NAME;

        if (alignment && !is_ref)
        {
                c_printrw(self, CTK_ALIGNED);
                c_printrw(self, CTK_LBRACKET);
                c_print_expr(self, alignment);
                c_printrw(self, CTK_RBRACKET);
                c_print_space(self);
        }
        c_print_decl_name(self, record);
        if (is_ref)
                return;

        c_print_decl_scope(self, tree_get_record_cfields(record), true, CPRINT_OPTS_NONE);
}

static void c_print_enum_specifier(c_printer* self, const tree_decl* enum_, int opts)
{
        c_printrw(self, CTK_ENUM);
        c_print_space(self);
        c_print_decl_name(self, enum_);
        if (opts & CPRINT_DECL_NAME)
                return;

        c_print_decl_scope(self, tree_get_enum_cvalues(enum_), true, CPRINT_OPTS_NONE);
}

static void c_print_decl_storage_specs(c_printer* self, const tree_decl* d)
{
        tree_storage_class sc = tree_get_decl_storage_class(d);
        if (sc != TSC_NONE || sc != TSC_IMPL_EXTERN)
        {
                if (sc == TSC_AUTO)
                        c_printrw(self, CTK_AUTO);
                else if (sc == TSC_REGISTER)
                        c_printrw(self, CTK_REGISTER);
                else if (sc == TSC_EXTERN)
                        c_printrw(self, CTK_EXTERN);
                else if (sc == TSC_STATIC)
                        c_printrw(self, CTK_STATIC);
                c_print_space(self);
        }

        tree_dll_storage_class dsc = tree_get_decl_dll_storage_class(d);
        if (dsc == TDSC_IMPORT)
        {
                c_printrw(self, CTK_DLLIMPORT);
                c_print_space(self);
        }

        tree_storage_duration sd = tree_get_decl_storage_duration(d);
        if (sd == TSD_THREAD)
        {
                c_printrw(self, CTK_THREAD_LOCAL);
                c_print_space(self);
        }
}

static void c_print_function(c_printer* self, const tree_decl* f, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_decl_name(self, f);
                return;
        }

        if (!(opts & CPRINTER_IGNORE_STORAGE_SPECS))
                c_print_decl_storage_specs(self, f);

        if (tree_func_is_inlined(f))
        {
                c_printrw(self, CTK_INLINE);
                c_print_space(self);
        }

        _c_print_type_name(self, tree_get_decl_type(f), f, opts);

        const tree_stmt* body = tree_get_func_body(f);
        if (body)
                c_print_stmt(self, body);
        else if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_printrw(self, CTK_SEMICOLON);
}

static void c_print_field(c_printer* self, const tree_decl* m, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_decl_name(self, m);
                return;
        }

        _c_print_type_name(self, tree_get_decl_type(m), m, opts);
        const tree_expr* bits = tree_get_field_bit_width(m);
        if (bits)
        {
                c_print_space(self);
                c_printrw(self, CTK_COLON);
                c_print_space(self);
                c_print_expr(self, bits);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_printrw(self, CTK_SEMICOLON);
}

static void c_print_var_decl(c_printer* self, const tree_decl* v, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_decl_name(self, v);
                return;
        }

        c_print_decl_storage_specs(self, v);
        _c_print_type_name(self, tree_get_decl_type(v), v, opts);

        const tree_expr* init = tree_get_var_init(v);
        if (self->opts.print_semantic_init)
                init = tree_get_var_semantic_init(v);
        if (init)
        {
                c_print_space(self);
                c_printrw(self, CTK_EQ);
                c_print_space(self);
                c_print_expr(self, init);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_printrw(self, CTK_SEMICOLON);
}

static void c_print_param_decl(c_printer* self, const tree_decl* p, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_decl_name(self, p);
                return;
        }
        _c_print_type_name(self, tree_get_decl_type(p), p, opts);
}

static void c_print_enumerator(c_printer* self, const tree_decl* e, int opts)
{
        c_print_decl_name(self, e);

        const tree_expr* value = tree_get_enumerator_expr(e);
        if (value && !tree_expr_is(value, TEK_IMPL_INIT) && !(opts & CPRINT_DECL_NAME))
        {
                c_print_space(self);
                c_printrw(self, CTK_EQ);
                c_print_space(self);
                c_print_expr(self, value);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_print_comma(self);
}

static void c_print_decl_group(c_printer* self, const tree_decl* g, int opts)
{
        bool first_printed = false;
        TREE_FOREACH_DECL_IN_GROUP(g, it)
        {
                int o = first_printed ? CPRINTER_IGNORE_SPECS : CPRINT_OPTS_NONE;
                c_print_decl(self, *it, o | CPRINTER_IGNORE_DECL_ENDING);
                first_printed = true;

                if (it + 1 != tree_get_decl_group_end(g))
                        c_print_comma(self);
                else
                        c_printrw(self, CTK_SEMICOLON);
        }
}

extern void c_print_decl(c_printer* self, const tree_decl* d, int opts)
{
        if (!d)
                return;

        switch (tree_get_decl_kind(d))
        {
                case TDK_TYPEDEF: c_print_typedef(self, d, opts); break;
                case TDK_RECORD: c_print_struct_or_union_specifier(self, d, opts); break;
                case TDK_ENUM: c_print_enum_specifier(self, d, opts); break;
                case TDK_FUNCTION: c_print_function(self, d, opts); break;
                case TDK_FIELD: c_print_field(self, d, opts); break;
                case TDK_VAR: c_print_var_decl(self, d, opts); break;
                case TDK_PARAM: c_print_param_decl(self, d, opts); break;
                case TDK_ENUMERATOR: c_print_enumerator(self, d, opts); break;
                case TDK_GROUP: c_print_decl_group(self, d, opts); break;
                default: break;
        }
}

static void c_print_stmt_with_indent(c_printer*self, const tree_stmt* s)
{
        if (!s)
                return;

        if (tree_get_stmt_kind(s) != TSK_COMPOUND)
        {
                c_printer_add_indent(self);
                c_print_endl(self);
                c_print_stmt(self, s);
                c_printer_sub_indent(self);
        }
        else
                c_print_stmt(self, s);
}

static void c_print_labeled_stmt(c_printer* self, const tree_stmt* s)
{
        bool restore_indent = false;
        if (self->indent_level)
        {
                c_printer_sub_indent(self);
                restore_indent = true;
        }

        c_print_endl(self);
        const tree_decl* label = tree_get_label_stmt_decl(s);
        c_print_decl_name(self, label);
        c_printrw(self, CTK_COLON);

        if (restore_indent)
                c_printer_add_indent(self);
        c_print_endl(self);
        c_print_stmt(self, tree_get_label_decl_stmt(label));
}

static void c_print_case_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_CASE);
        c_print_space(self);
        c_print_expr(self, tree_get_case_expr(s));
        c_printrw(self, CTK_COLON);
        c_printer_add_indent(self);
        c_print_endl(self);
        c_print_stmt(self, tree_get_case_body(s));
        c_printer_sub_indent(self);
}

static void c_print_default_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_DEFAULT);
        c_printrw(self, CTK_COLON);
        c_printer_add_indent(self);
        c_print_endl(self);
        c_print_stmt(self, tree_get_default_body(s));
        c_printer_sub_indent(self);
}

static void c_print_compound_stmt(c_printer* self, const tree_stmt* s)
{
        c_print_lbrace(self, true);
        const tree_scope* scope = tree_get_compound_cscope(s);
        TREE_FOREACH_STMT(scope, stmt)
        {
                c_print_endl(self);
                c_print_stmt(self, stmt);
        }
        c_print_rbrace(self, true);
}

static void c_print_atomic_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_ATOMIC);
        c_print_stmt_with_indent(self, tree_get_atomic_body(s));
}

static void c_print_expr_stmt(c_printer* self, const tree_stmt* s)
{
        const tree_expr* expr = tree_get_expr_stmt_root(s);
        c_print_expr(self, expr);
        c_printrw(self, CTK_SEMICOLON);
        if (!expr)
                return;

        bool print_type = self->opts.print_expr_type;
        bool print_val = self->opts.print_expr_value;
        bool print_eval_result = self->opts.print_eval_result;
        if (print_eval_result || print_val || print_type)
                c_prints(self, " // ");

        if (print_val)
                c_prints(self, (tree_expr_is_lvalue(expr) ? "lvalue " : "rvalue "));
        if (print_type)
                c_print_type_name(self, tree_get_expr_type(expr),
                        CPRINT_TYPE_REF | CPRINT_IMPL_TYPE_BRACKETS);
        if (print_eval_result)
        {
                tree_eval_result r;
                tree_eval_expr(self->context->target, expr, &r);
                if (r.kind == TERK_INVALID)
                        c_prints(self, "not-a-constant ");
                else if (r.kind == TERK_ADDRESS_CONSTANT)
                        c_prints(self, "address constant ");
                else
                {
                        tree_type* t = tree_get_expr_type(expr);
                        if (tree_type_is_integer(t))
                                c_print_integer(self, t, num_as_u64(&r.value));
                        else if (tree_builtin_type_is(t, TBTK_FLOAT))
                                c_print_float(self, num_f32(&r.value));
                        else if (tree_builtin_type_is(t, TBTK_DOUBLE))
                                c_print_double(self, num_f64(&r.value));
                }
        }
}

static void c_print_if_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_IF);
        c_print_space(self);
        _c_print_expr(self, tree_get_if_condition(s), true);
        c_print_stmt_with_indent(self, tree_get_if_body(s));

        const tree_stmt* else_ = tree_get_if_else(s);
        if (else_)
        {
                c_print_endl(self);
                c_printrw(self, CTK_ELSE);
                c_print_space(self);
                if (tree_get_stmt_kind(else_) != TSK_IF)
                        c_print_stmt_with_indent(self, else_);
                else
                        c_print_stmt(self, else_);
        }
}

static void c_print_switch_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_SWITCH);
        c_print_space(self);
        _c_print_expr(self, tree_get_switch_expr(s), true);
        c_print_stmt_with_indent(self, tree_get_switch_body(s));
}

static void c_print_while_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_WHILE);
        c_print_space(self);
        _c_print_expr(self, tree_get_while_condition(s), true);
        c_print_stmt_with_indent(self, tree_get_while_body(s));
}

static void c_print_do_while_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_DO);
        c_print_space(self);
        c_print_stmt_with_indent(self, tree_get_do_while_body(s));
        c_print_endl(self);
        c_printrw(self, CTK_WHILE);
        c_print_space(self);
        _c_print_expr(self, tree_get_do_while_condition(s), true);
        c_printrw(self, CTK_SEMICOLON);
}

static void c_print_for_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_FOR);
        c_print_space(self);
        c_print_lbracket(self);
        c_print_stmt(self, tree_get_for_init(s));
        c_print_space(self);
        c_print_expr(self, tree_get_for_condition(s));
        c_printrw(self, CTK_SEMICOLON);
        c_print_space(self);
        c_print_expr(self, tree_get_for_step(s));
        c_print_rbracket(self);
        c_print_stmt_with_indent(self, tree_get_for_body(s));
}

static void c_print_goto_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_GOTO);
        c_print_space(self);
        c_print_decl_name(self, tree_get_goto_label(s));
        c_printrw(self, CTK_SEMICOLON);
}

static void c_print_continue_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_CONTINUE);
        c_printrw(self, CTK_SEMICOLON);
}

static void c_print_break_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_BREAK);
        c_printrw(self, CTK_SEMICOLON);
}

static void c_print_decl_stmt(c_printer* self, const tree_stmt* s)
{
        c_print_decl(self, tree_get_decl_stmt_entity(s), CPRINT_OPTS_NONE);
}

static void c_print_return_stmt(c_printer* self, const tree_stmt* s)
{
        c_printrw(self, CTK_RETURN);
        c_print_space(self);
        c_print_expr(self, tree_get_return_value(s));
        c_printrw(self, CTK_SEMICOLON);
}

extern void c_print_stmt(c_printer* self, const tree_stmt* s)
{
        if (!s)
                return;

        switch (tree_get_stmt_kind(s))
        {
                case TSK_LABELED: c_print_labeled_stmt(self, s); break;
                case TSK_CASE: c_print_case_stmt(self, s); break;
                case TSK_DEFAULT: c_print_default_stmt(self, s); break;
                case TSK_COMPOUND: c_print_compound_stmt(self, s); break;
                case TSK_ATOMIC: c_print_atomic_stmt(self, s); break;
                case TSK_EXPR: c_print_expr_stmt(self, s); break;
                case TSK_IF: c_print_if_stmt(self, s); break;
                case TSK_SWITCH: c_print_switch_stmt(self, s); break;
                case TSK_WHILE: c_print_while_stmt(self, s); break;
                case TSK_DO_WHILE: c_print_do_while_stmt(self, s); break;
                case TSK_FOR: c_print_for_stmt(self, s); break;
                case TSK_GOTO: c_print_goto_stmt(self, s); break;
                case TSK_CONTINUE: c_print_continue_stmt(self, s); break;
                case TSK_BREAK: c_print_break_stmt(self, s); break;
                case TSK_DECL: c_print_decl_stmt(self, s); break;
                case TSK_RETURN: c_print_return_stmt(self, s); break;
                default: break;
        }
}

extern void c_print_module(c_printer* self, const tree_module* module)
{
        c_print_decl_scope(self, tree_get_module_cglobals(module), false, CPRINT_OPTS_NONE);
}
