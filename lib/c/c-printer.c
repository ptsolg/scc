#include "scc/c/c-printer.h"
#include "scc/c/c-token.h"
#include "scc/c/c-info.h"
#include "scc/c/c-reswords-info.h"
#include "scc/c/c-context.h"
#include "scc/c/c-source.h"
#include "scc/c/c-limits.h"
#include "scc/tree/tree-eval.h"
#include "scc/core/char-info.h"
#include <stdarg.h>

extern void c_printer_opts_init(c_printer_opts* self)
{
        self->print_expr_type = false;
        self->print_expr_value = false;
        self->print_impl_casts = false;
        self->print_eval_result = false;
        self->double_precision = 4;
        self->float_precision = 4;
        self->force_brackets = false;
}

extern void c_printer_init(
        c_printer* self,
        write_cb* write,
        const c_context* context,
        const c_source_manager* source_manager)
{
        self->context = c_context_get_tree_context(context);
        self->ccontext = context;
        self->source_manager = source_manager;
        self->target = self->context->target;
        self->indent_level = 0;
        writebuf_init(&self->buf, write);
        c_printer_opts_init(&self->opts);
}

extern void c_printer_dispose(c_printer* self)
{
        writebuf_flush(&self->buf);
}

static inline void c_prints(c_printer* self, const char* s)
{
        if (s)
                writebuf_writes(&self->buf, s);
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
        writebuf_writec(&self->buf, c);
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
        strentry entry;
        tree_get_id_strentry(self->context, id, &entry);
        char unescaped[C_MAX_LINE_LENGTH + 1];
        c_get_unescaped_string(unescaped, (const char*)entry.data, entry.size - 1);
        c_printf(self, "\"%s\"", unescaped);
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
        {
                int c = c_token_get_char(token);
                if (char_is_escape(c))
                        c_printf(self, "'\\%c'", escape_to_char(c));
                else
                        c_printf(self, "'%c'", c);
        }
        c_print_endl(self);
}

extern void c_print_token(c_printer* self, const c_token* token, const c_token_print_info* info)
{
        c_location loc;
        c_source_find_loc(self->source_manager, &loc, c_token_get_loc(token));

        char file[MAX_PATH_LEN];
        file[0] = '\0';

        if (false) // todo: -print-token-file
        {
                strncpy(file, path_get_cfile(loc.file), MAX_PATH_LEN);
                strfill(strend(file), ' ', 1 + info->max_file_len - strlen(file));
        }

        char col[32];
        sprintf(col, "%d", loc.column);
        strfill(strend(col), ' ', 1 + info->max_col - ndigits(loc.column));

        char line[32];
        sprintf(line, "%d", loc.line);
        strfill(strend(line), ' ', 1 + info->max_line - ndigits(loc.line));

        const char* kind = c_get_token_kind_string(c_token_get_kind(token));
        char trail[32];
        strfill(trail, ' ', 1 + info->max_kind_len - strlen(kind));

        c_printf(self, "%s%s%s%s%s", file, line, col, kind, trail);
        c_print_token_value(self, token);
}

extern void c_print_tokens(c_printer* self, const dseq* tokens)
{
        c_token_print_info info;
        c_token_print_info_init(&info);

        for (size_t i = 0; i < dseq_size(tokens); i++)
        {
                const c_token* token = dseq_get(tokens, i);
                c_location loc;
                c_source_find_loc(self->source_manager, &loc, c_token_get_loc(token));

                if (ndigits(loc.line) > info.max_line)
                        info.max_line = ndigits(loc.line);
                if (ndigits(loc.column) > info.max_col)
                        info.max_col = ndigits(loc.column);

                int len = (int)strlen(path_get_cfile(loc.file));
                if (len > info.max_file_len)
                        info.max_file_len = len;

                len = (int)strlen(c_get_token_kind_string(c_token_get_kind(token)));
                if (len > info.max_kind_len)
                        info.max_kind_len = len;
        }
        for (size_t i = 0; i < dseq_size(tokens); i++)
                c_print_token(self, dseq_get(tokens, i), &info);
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

static void c_print_binop(c_printer* self, const tree_expr* expr)
{
        c_print_expr(self, tree_get_binop_lhs(expr));
        c_print_space(self);
        c_prints(self, c_get_binop_string(tree_get_binop_kind(expr)));
        c_print_space(self);
        c_print_expr(self, tree_get_binop_rhs(expr));
}

static void c_print_unop(c_printer* self, const tree_expr* expr)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        const tree_expr* e = tree_get_unop_operand(expr);
        const char* unop = c_get_unop_string(k);

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
        const float_value* value = tree_get_floating_literal_cvalue(expr);
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                c_print_float(self, float_get_sp(value));
        else
                c_print_double(self, float_get_dp(value));
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
        c_printf(self, "'%c'", tree_get_character_literal(expr));
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
                if (tree_designator_is_field(*it))
                {
                        c_printrw(self, CTK_DOT);
                        c_print_id(self, tree_get_designator_field(*it));
                }
                else
                {
                        c_print_lsbracket(self);
                        c_print_expr(self, tree_get_designator_index(*it));
                        c_print_rsbracket(self);
                }
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
        int ntype_parts;
        int nsuffixes;
        tree_id name;
        bool name_printed;
        bool params_vararg;
        const tree_decl_scope* params;
        const tree_type* type_parts[100]; // todo: size
        const tree_type* suffixes[100];
} c_type_name_info;

static void c_type_name_info_init(
        c_type_name_info* self,
        const tree_type* type,
        tree_id name,
        bool params_vararg,
        const tree_decl_scope* params)
{
        self->ntype_parts = 0;
        self->nsuffixes = 0;
        self->name = name;
        self->name_printed = false;
        self->params = params;
        self->params_vararg = params_vararg;

        const tree_type* it = type;
        while (it)
        {
                self->type_parts[self->ntype_parts++] = it;
                it = tree_get_type_next(it);
        }
}

static void c_print_builtin_type(c_printer* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;

        c_print_type_quals(self, tree_get_type_quals(t));
        c_prints(self, c_get_builtin_type_string(tree_get_builtin_type_kind(t)));
}

static void c_print_decl_type(c_printer* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;
        if (tree_decl_type_is_referenced(t))
                opts |= CPRINT_DECL_NAME;

        c_print_decl(self, tree_get_decl_type_entity(t), opts);
}

static void c_print_type_parts(c_printer* self, c_type_name_info* info, int opts)
{
        int part_it = info->ntype_parts;
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
                        info->suffixes[info->nsuffixes++] = t;
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

                if (!info->name_printed && info->name != TREE_INVALID_ID)
                {
                        c_print_space(self);
                        c_print_id(self, info->name);

                        if (!info->params)
                                continue;

                        c_print_lbracket(self);
                        TREE_FOREACH_DECL_IN_SCOPE(info->params, param)
                        {
                                c_print_decl(self, param, CPRINTER_IGNORE_DECL_ENDING);
                                const tree_decl* next = tree_get_next_decl(param);
                                if (next != tree_get_decl_scope_decls_cend(info->params))
                                        c_print_comma(self);
                        }
                        if (info->params_vararg)
                        {
                                c_print_comma(self);
                                c_printrw(self, CTK_ELLIPSIS);
                        }
                        c_print_rbracket(self);
                }
        }
}

static void c_print_suffix_endings(c_printer* self, c_type_name_info* info, int opts)
{
        int suffix_it = info->nsuffixes;
        while (suffix_it)
        {
                const tree_type* t = info->suffixes[--suffix_it];
                tree_type_kind k = tree_get_type_kind(t);
                if (opts & CPRINT_IMPL_TYPE_BRACKETS)
                        c_print_rbracket(self);

                if (k == TTK_FUNCTION)
                {
                        c_print_lbracket(self);
                        TREE_FOREACH_FUNCTION_TYPE_PARAM(t, param)
                        {
                                c_print_type_name(self, *param, CPRINT_OPTS_NONE);
                                if (param + 1 != tree_get_function_type_params_end(t))
                                        c_print_comma(self);
                        }
                        if (tree_function_type_is_vararg(t))
                        {
                                c_print_comma(self);
                                c_printrw(self, CTK_ELLIPSIS);
                        }
                        c_print_rbracket(self);
                }
                else if (k == TTK_ARRAY)
                {
                        c_print_lsbracket(self);
                        if (tree_array_is(t, TAK_CONSTANT))
                                c_print_expr(self, tree_get_constant_array_size_expr(t));
                        c_print_rsbracket(self);
                }
                else if (k == TTK_PAREN)
                        c_print_rbracket(self);
        }
}

static void _c_print_type_name(
        c_printer* self,
        const tree_type* type,
        tree_id name,
        bool params_vararg,
        const tree_decl_scope* params,
        int opts)
{
        if (!type)
                return;

        c_type_name_info info;
        c_type_name_info_init(&info, type, name, params_vararg, params);
        c_print_type_parts(self, &info, opts);
        c_print_suffix_endings(self, &info, opts);
}

extern void c_print_type_name(c_printer* self, const tree_type* type, int opts)
{
        _c_print_type_name(self, type, TREE_INVALID_ID, false, NULL, opts);
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

        _c_print_type_name(self, tree_get_decl_type(decl), name, false, NULL, opts);
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                c_printrw(self, CTK_SEMICOLON);
}

static void c_print_struct_or_union_specifier(c_printer* self, const tree_decl* record, int opts)
{
        c_printrw(self, (tree_record_is_union(record) ? CTK_UNION : CTK_STRUCT));
        c_print_space(self);
        c_print_decl_name(self, record);
        if (opts & CPRINT_DECL_NAME)
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

static void c_print_decl_storage_class(c_printer* self, tree_decl_storage_class c)
{
        if (c == TDSC_AUTO)
                c_printrw(self, CTK_AUTO);
        else if (c == TDSC_REGISTER)
                c_printrw(self, CTK_REGISTER);
        else if (c == TDSC_EXTERN)
                c_printrw(self, CTK_EXTERN);
        else if (c == TDSC_STATIC)
                c_printrw(self, CTK_STATIC);
        else
                return;
        c_print_space(self);
}

static void c_print_function_specifier(c_printer* self, tree_function_specifier_kind k)
{
        if (k == TFSK_INLINE)
                c_printrw(self, CTK_INLINE);
        else
                return;
        c_print_space(self);
}

static void c_print_function(c_printer* self, const tree_decl* f, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                c_print_decl_name(self, f);
                return;
        }

        if (!(opts & CPRINTER_IGNORE_STORAGE_SPECS))
                c_print_decl_storage_class(self, tree_get_decl_storage_class(f));

        c_print_function_specifier(self, tree_get_function_specifier(f));

        const tree_type* func_type = tree_get_decl_type(f);
        // since we have param-list we skip first function type
        const tree_type* restype = tree_get_function_type_result(func_type);
        _c_print_type_name(self, restype, tree_get_decl_name(f),
                tree_function_type_is_vararg(func_type), tree_get_function_cparams(f), opts);

        const tree_stmt* body = tree_get_function_body(f);
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

        _c_print_type_name(self, tree_get_decl_type(m), tree_get_decl_name(m), false, NULL, opts);
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

        c_print_decl_storage_class(self, tree_get_decl_storage_class(v));
        _c_print_type_name(self, tree_get_decl_type(v), tree_get_decl_name(v), false, NULL, opts);

        const tree_expr* init = tree_get_var_init(v);
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
                tree_eval_expr(self->context, expr, &r);
                if (r.kind == TERK_INVALID)
                        c_prints(self, "not-a-constant ");
                else if (r.kind == TERK_ADDRESS_CONSTANT)
                        c_prints(self, "address constant ");
                else
                {
                        tree_type* t = tree_get_expr_type(expr);
                        if (tree_type_is_integer(t))
                                c_print_integer(self, t, avalue_get_u64(&r.value));
                        else if (tree_builtin_type_is(t, TBTK_FLOAT))
                                c_print_float(self, avalue_get_sp(&r.value));
                        else if (tree_builtin_type_is(t, TBTK_DOUBLE))
                                c_print_double(self, avalue_get_dp(&r.value));
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