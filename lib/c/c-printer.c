#include "scc/c/c-printer.h"
#include "scc/c/c-token.h"
#include "scc/c/c-info.h"
#include "scc/c/c-reswords.h"
#include "scc/c/c-tree.h"
#include "scc/c/c-source.h"
#include "scc/c/c-limits.h"
#include "scc/tree/tree-eval.h"
#include "scc/scl/char-info.h"
#include <stdarg.h>

extern void cprinter_opts_init(cprinter_opts* self)
{
        self->print_expr_type = false;
        self->print_expr_value = false;
        self->print_impl_casts = false;
        self->print_eval_result = false;
        self->double_precision = 4;
        self->float_precision = 4;
        self->force_brackets = false;
}

extern void cprinter_init(
        cprinter* self,
        write_cb* write,
        const ccontext* context,
        const csource_manager* source_manager)
{
        self->context = cget_tree(context);
        self->ccontext = context;
        self->source_manager = source_manager;
        self->target = tree_get_target(self->context);
        self->indent_level = 0;
        writebuf_init(&self->buf, write);
        cprinter_opts_init(&self->opts);
}

extern void cprinter_dispose(cprinter* self)
{
        writebuf_flush(&self->buf);
}

static inline void cprints(cprinter* self, const char* s)
{
        if (s)
                writebuf_writes(&self->buf, s);
}

static inline void cprintf(cprinter* self, const char* f, ...)
{
        char buf[1024];

        va_list args;
        va_start(args, f);
        vsnprintf(buf, 1024, f, args);

        cprints(self, buf);
}

static inline void cprint_float(cprinter* self, float v)
{
        cprintf(self, "%.*ff", self->opts.float_precision, v);
}

static inline void cprint_double(cprinter* self, double v)
{
        cprintf(self, "%.*f", self->opts.double_precision, v);
}

static inline void cprint_d(cprinter* self, int v)
{
        cprintf(self, "%d", v);
}

static inline void cprint_lld(cprinter* self, sint64 v)
{
        cprintf(self, "%lld", v);
        cprints(self, "LL");
}

static inline void cprint_u(cprinter* self, uint v)
{
        cprintf(self, "%u", v);
        cprints(self, "U");
}

static inline void cprint_llu(cprinter* self, suint64 v)
{
        cprintf(self, "%llu", v);
        cprints(self, "ULL");
}

static inline void cprintc(cprinter* self, int c)
{
        writebuf_writec(&self->buf, c);
}

static inline void cprintcn(cprinter* self, int c, int n)
{
        while (n--)
                cprintc(self, c);
}

static inline void cprint_id(cprinter* self, tree_id id)
{
        cprints(self, tree_get_id_cstr(self->context, id));
}

static inline void cprint_space(cprinter* self)
{
        cprintc(self, ' ');
}

static inline void cprint_indent(cprinter* self)
{
        cprintcn(self, '\t', self->indent_level);
}

static inline void cprinter_add_indent(cprinter* self)
{
        self->indent_level++;
}

static inline void cprinter_sub_indent(cprinter* self)
{
        S_ASSERT(self->indent_level);
        self->indent_level--;
}

static inline void cprint_endl(cprinter* self)
{
        cprintc(self, '\n');
        cprint_indent(self);
}

static inline void cprintrw(cprinter* self, ctoken_kind k)
{
        cprints(self, cget_token_kind_info(k)->string);
}

extern void ctoken_print_info_init(ctoken_print_info* self)
{
        self->max_col = 0;
        self->max_file_len = 0;
        self->max_kind_len = 0;
        self->max_line = 0;
}

static void _cprint_string_literal(cprinter* self, tree_id id)
{
        strentry entry;
        tree_get_id_strentry(self->context, id, &entry);
        char unescaped[CMAX_LINE_LENGTH + 1];
        cget_unescaped_string(unescaped, entry.data, entry.len - 1);
        cprintf(self, "\"%s\"", unescaped);
}

static void cprint_token_value(cprinter* self, const ctoken* token)
{
        ctoken_kind k = ctoken_get_kind(token);

        if (k == CTK_ID)
                cprint_id(self, ctoken_get_string(token));
        else if (k == CTK_CONST_INT)
        {
                cprintf(self, "%llu", ctoken_get_int(token));
                int ls = ctoken_get_int_ls(token);
                while (ls--)
                        cprintc(self, 'L');
                if (!ctoken_is_int_signed(token))
                        cprintc(self, 'U');
        }
        else if (k == CTK_CONST_FLOAT)
                cprintf(self, "%ff", ctoken_get_float(token));
        else if (k == CTK_CONST_DOUBLE)
                cprintf(self, "%f", (double)ctoken_get_double(token));
        else if (k == CTK_CONST_STRING)
                _cprint_string_literal(self, ctoken_get_string(token));
        else if (k == CTK_CONST_CHAR)
        {
                int c = ctoken_get_char(token);
                if (char_is_escape(c))
                        cprintf(self, "'\\%c'", escape_to_char(c));
                else
                        cprintf(self, "'%c'", c);
        }
        cprint_endl(self);
}

extern void cprint_token(cprinter* self, const ctoken* token, const ctoken_print_info* info)
{
        clocation loc;
        csource_find_loc(self->source_manager, &loc, ctoken_get_loc(token));

        char file[S_MAX_PATH_LEN];
        file[0] = '\0';

        if (false) // todo: -print-token-file
        {
                strncpy(file, path_get_cfile(loc.file), S_MAX_PATH_LEN);
                sstrfill(sstrend(file), ' ', 1 + info->max_file_len - strlen(file));
        }

        char col[32];
        sprintf(col, "%d", loc.column);
        sstrfill(sstrend(col), ' ', 1 + info->max_col - ndigits(loc.column));

        char line[32];
        sprintf(line, "%d", loc.line);
        sstrfill(sstrend(line), ' ', 1 + info->max_line - ndigits(loc.line));

        const char* kind = cget_token_kind_string(ctoken_get_kind(token));
        char trail[32];
        sstrfill(trail, ' ', 1 + info->max_kind_len - strlen(kind));

        cprintf(self, "%s%s%s%s%s", file, line, col, kind, trail);
        cprint_token_value(self, token);
}

extern void cprint_tokens(cprinter* self, const dseq* tokens)
{
        ctoken_print_info info;
        ctoken_print_info_init(&info);

        for (ssize i = 0; i < dseq_size(tokens); i++)
        {
                const ctoken* token = dseq_get_ptr(tokens, i);
                clocation loc;
                csource_find_loc(self->source_manager, &loc, ctoken_get_loc(token));

                if (ndigits(loc.line) > info.max_line)
                        info.max_line = ndigits(loc.line);
                if (ndigits(loc.column) > info.max_col)
                        info.max_col = ndigits(loc.column);

                int len = (int)strlen(path_get_cfile(loc.file));
                if (len > info.max_file_len)
                        info.max_file_len = len;

                len = (int)strlen(cget_token_kind_string(ctoken_get_kind(token)));
                if (len > info.max_kind_len)
                        info.max_kind_len = len;
        }
        for (ssize i = 0; i < dseq_size(tokens); i++)
                cprint_token(self, dseq_get_ptr(tokens, i), &info);
}

static inline void cprint_lbracket(cprinter* self)
{
        cprintrw(self, CTK_LBRACKET);
}

static inline void cprint_rbracket(cprinter* self)
{
        cprintrw(self, CTK_RBRACKET);
}

static inline void cprint_lsbracket(cprinter* self)
{
        cprintrw(self, CTK_LSBRACKET);
}

static inline void cprint_rsbracket(cprinter* self)
{
        cprintrw(self, CTK_RSBRACKET);
}

static inline void cprint_lbrace(cprinter* self, bool newline)
{
        if (newline)
        {
                cprint_endl(self);
                cprinter_add_indent(self);
        }
        cprintrw(self, CTK_LBRACE);
}

static inline void cprint_rbrace(cprinter* self, bool newline)
{
        if (newline)
        {
                cprinter_sub_indent(self);
                cprint_endl(self);
        }
        cprintrw(self, CTK_RBRACE);
}

static inline void cprint_comma(cprinter* self)
{
        cprintrw(self, CTK_COMMA);
        cprint_space(self);
}

static inline void cprint_decl_name(cprinter* self, const tree_decl* d)
{
        cprint_id(self, tree_get_decl_name(d));
}

static void cprint_binop(cprinter* self, const tree_expr* expr)
{
        cprint_expr(self, tree_get_binop_lhs(expr));
        cprint_space(self);
        cprints(self, cget_binop_string(tree_get_binop_kind(expr)));
        cprint_space(self);
        cprint_expr(self, tree_get_binop_rhs(expr));
}

static void cprint_unop(cprinter* self, const tree_expr* expr)
{
        tree_unop_kind k = tree_get_unop_kind(expr);
        const tree_expr* e = tree_get_unop_expr(expr);
        const char* unop = cget_unop_string(k);

        if (k == TUK_POST_INC || k == TUK_POST_DEC)
        {
                cprint_expr(self, e);
                cprints(self, unop);
        }
        else
        {
                cprints(self, unop);
                cprint_expr(self, e);
        }
}

static inline bool cprinter_postfix_expr_needs_wrapping(const cprinter* self, const tree_expr* e)
{
        return tree_get_expr_kind(e) == TEK_IMPLICIT_CAST && self->opts.print_impl_casts;
}

static void _cprint_expr(cprinter*, const tree_expr*, bool);

static void cprint_call_expr(cprinter* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_call_lhs(expr);
        _cprint_expr(self, lhs, cprinter_postfix_expr_needs_wrapping(self, lhs));

        cprint_lbracket(self);
        TREE_FOREACH_CALL_ARG(expr, arg)
        {
                cprint_expr(self, *arg);
                if (arg + 1 != tree_get_call_args_end(expr))
                        cprint_comma(self);
        }
        cprint_rbracket(self);
}

static void cprint_subscript_expr(cprinter* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_subscript_lhs(expr);
        _cprint_expr(self, lhs, cprinter_postfix_expr_needs_wrapping(self, lhs));
        cprint_lsbracket(self);
        cprint_expr(self, tree_get_subscript_rhs(expr));
        cprint_rsbracket(self);
}

static void cprint_conditional_expr(cprinter* self, const tree_expr* expr)
{
        cprint_expr(self, tree_get_conditional_condition(expr));
        cprint_space(self);
        cprintrw(self, CTK_QUESTION);
        cprint_space(self);
        cprint_expr(self, tree_get_conditional_lhs(expr));
        cprint_space(self);
        cprintrw(self, CTK_COLON);
        cprint_space(self);
        cprint_expr(self, tree_get_conditional_rhs(expr));
}

static void cprint_decl_expr(cprinter* self, const tree_expr* expr)
{
        cprint_decl(self, tree_get_decl_expr_entity(expr),
                CPRINT_DECL_NAME | CPRINTER_IGNORE_DECL_ENDING);
}

static void cprint_floating_literal(cprinter* self, const tree_expr* expr)
{
        const float_value* value = tree_get_floating_literal_cvalue(expr);
        if (tree_builtin_type_is(tree_get_expr_type(expr), TBTK_FLOAT))
                cprint_float(self, float_get_sp(value));
        else
                cprint_double(self, float_get_dp(value));
}

static void cprint_integer(cprinter* self, const tree_type* t, suint64 v)
{
        S_ASSERT(tree_type_is_integer(t));
        bool sign = !tree_type_is_unsigned_integer(t);
        bool ext = tree_builtin_type_is(t, TBTK_UINT64)
                || tree_builtin_type_is(t, TBTK_INT64);

        if (sign && ext)
                cprint_lld(self, (sint64)v);
        else if (!sign && ext)
                cprint_llu(self, v);
        else if (!sign)
                cprint_u(self, (uint)v);
        else
                cprint_d(self, (int)v);
}

static void cprint_integer_literal(cprinter* self, const tree_expr* expr)
{
        cprint_integer(self, tree_get_expr_type(expr), tree_get_integer_literal(expr));
}

static void cprint_char_literal(cprinter* self, const tree_expr* expr)
{
        cprintf(self, "'%c'", tree_get_character_literal(expr));
}

static void cprint_string_literal(cprinter* self, const tree_expr* expr)
{
        _cprint_string_literal(self, tree_get_string_literal(expr));
}

static void cprint_member_expr(cprinter* self, const tree_expr* expr)
{
        const tree_expr* lhs = tree_get_member_expr_lhs(expr);
        _cprint_expr(self, lhs, cprinter_postfix_expr_needs_wrapping(self, lhs));
        cprintrw(self, (tree_member_expr_is_arrow(expr) ? CTK_ARROW : CTK_DOT));
        cprint_decl(self, tree_get_member_expr_decl(expr), CPRINT_DECL_NAME);
}

static void cprint_cast_expr(cprinter* self, const tree_expr* expr)
{
        const tree_expr* e = tree_get_cast_expr(expr);
        tree_expr_kind k = tree_get_expr_kind(expr);
        if (k == TEK_EXPLICIT_CAST || self->opts.print_impl_casts)
        {
                cprint_lbracket(self);
                int opts = k == TEK_IMPLICIT_CAST ? CPRINT_IMPL_TYPE_BRACKETS : CPRINT_OPTS_NONE;
                cprint_type_name(self, tree_get_expr_type(expr), opts);
                cprint_rbracket(self);
        }

        bool brackets = false;
        if (k == TEK_IMPLICIT_CAST)
        {
                brackets = (self->opts.print_impl_casts && tree_get_expr_kind(e) == TEK_BINARY)
                        || self->opts.force_brackets;
        }
        _cprint_expr(self, e, brackets);
}

static void cprint_sizeof_expr(cprinter* self, const tree_expr* expr)
{
        cprintrw(self, CTK_SIZEOF);
        if (tree_sizeof_is_unary(expr))
        {
                cprint_space(self);
                cprint_expr(self, tree_get_sizeof_expr(expr));
        }
        else
        {
                cprint_lbracket(self);
                cprint_type_name(self, tree_get_sizeof_type(expr), CPRINT_OPTS_NONE);
                cprint_rbracket(self);
        }
}

static void cprint_paren_expr(cprinter* self, const tree_expr* expr)
{
        cprint_lbracket(self);
        _cprint_expr(self, tree_get_paren_expr(expr), false);
        cprint_rbracket(self);
}

static void cprint_designation(cprinter* self, const tree_designation* d)
{
        bool empty = true;
        TREE_FOREACH_DESIGNATOR(d, designator)
        {
                empty = false;
                tree_designator_kind k = tree_get_designator_kind(designator);
                if (k == TDK_DES_MEMBER)
                {
                        cprintrw(self, CTK_DOT);
                        cprint_decl_name(self, tree_get_member_designator_decl(designator));
                }
                else if (k == TDK_DES_ARRAY)
                {
                        cprint_lsbracket(self);
                        cprint_expr(self, tree_get_array_designator_index(designator));
                        cprint_rsbracket(self);
                }
        }
        if (!empty)
        {
                cprint_space(self);
                cprintrw(self, CTK_EQ);
                cprint_space(self);
        }

        cprint_expr(self, tree_get_designation_initializer(d));
}

static void cprint_initializer(cprinter* self, const tree_expr* expr)
{
        cprint_lbrace(self, false);
        TREE_FOREACH_DESIGNATION(expr, it)
        {
                cprint_designation(self, it);
                if (tree_get_next_designation(it) != tree_get_init_end(expr))
                        cprint_comma(self);
        }
        cprint_rbrace(self, false);
}

static void _cprint_expr(cprinter* self, const tree_expr* e, bool print_brackets)
{
        if (!e)
                return;

        if (print_brackets)
                cprint_lbracket(self);
        switch (tree_get_expr_kind(e))
        {
                case TEK_BINARY: cprint_binop(self, e); break;
                case TEK_UNARY: cprint_unop(self, e); break;
                case TEK_CALL: cprint_call_expr(self, e); break;
                case TEK_SUBSCRIPT: cprint_subscript_expr(self, e); break;
                case TEK_CONDITIONAL: cprint_conditional_expr(self, e); break;
                case TEK_CHARACTER_LITERAL: cprint_char_literal(self, e); break;
                case TEK_FLOATING_LITERAL: cprint_floating_literal(self, e); break;
                case TEK_INTEGER_LITERAL: cprint_integer_literal(self, e); break;
                case TEK_STRING_LITERAL: cprint_string_literal(self, e); break;
                case TEK_DECL: cprint_decl_expr(self, e); break;
                case TEK_MEMBER: cprint_member_expr(self, e); break;
                case TEK_IMPLICIT_CAST:
                case TEK_EXPLICIT_CAST: cprint_cast_expr(self, e); break;
                case TEK_SIZEOF: cprint_sizeof_expr(self, e); break;
                case TEK_PAREN: cprint_paren_expr(self, e); break;
                case TEK_INIT: cprint_initializer(self, e); break;

                default:
                        ; // unknown exp kind
        }
        if (print_brackets)
                cprint_rbracket(self);
}

extern void cprint_expr(cprinter* self, const tree_expr* expr)
{
        if (!expr)
                return;

        tree_expr_kind k = tree_get_expr_kind(expr);
        bool is_primitive = k == TEK_DECL
                || k == TEK_PAREN
                || tree_expr_is_literal(expr)
                || k == TEK_IMPLICIT_CAST;
        bool brackets = self->opts.force_brackets && !is_primitive;
        _cprint_expr(self, expr, brackets);
}

static void cprint_type_quals(cprinter* self, tree_type_quals q)
{
        if (q == TTQ_UNQUALIFIED)
                return;

        if (q & TTQ_CONST)
        {
                cprintrw(self, CTK_CONST);
                cprint_space(self);
        }
        if (q & TTQ_RESTRICT)
        {
                cprintrw(self, CTK_RESTRICT);
                cprint_space(self);
        }
        if (q & TTQ_VOLATILE)
        {
                cprintrw(self, CTK_VOLATILE);
                cprint_space(self);
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
} ctype_name_info;

static void ctype_name_info_init(
        ctype_name_info* self,
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

static void cprint_builtin_type(cprinter* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;

        cprint_type_quals(self, tree_get_type_quals(t));
        cprints(self, cget_builtin_type_string(tree_get_builtin_type_kind(t)));
}

static void cprint_decl_type(cprinter* self, const tree_type* t, int opts)
{
        if (opts & CPRINTER_IGNORE_TYPESPEC)
                return;
        if (tree_decl_type_is_referenced(t))
                opts |= CPRINT_DECL_NAME;

        cprint_decl(self, tree_get_decl_type_entity(t), opts);
}

static void cprint_type_parts(cprinter* self, ctype_name_info* info, int opts)
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
                        cprint_builtin_type(self, t, opts);
                else if (k == TTK_DECL)
                        cprint_decl_type(self, t, opts);
                else if (k == TTK_ARRAY || k == TTK_FUNCTION || k == TTK_PAREN)
                {
                        if (k == TTK_PAREN || opts & CPRINT_IMPL_TYPE_BRACKETS)
                                cprint_lbracket(self);
                        info->suffixes[info->nsuffixes++] = t;
                }
                else if (k == TTK_POINTER)
                {
                        cprintrw(self, CTK_STAR);
                        cprint_type_quals(self, tree_get_type_quals(t));
                }
                else
                        ; // unknown type kind

                if (next && tree_get_type_next(next))
                        continue;

                if (!info->name_printed && info->name != TREE_INVALID_ID)
                {
                        cprint_space(self);
                        cprint_id(self, info->name);

                        if (!info->params)
                                continue;

                        cprint_lbracket(self);
                        TREE_FOREACH_DECL_IN_SCOPE(info->params, param)
                        {
                                cprint_decl(self, param, CPRINTER_IGNORE_DECL_ENDING);
                                const tree_decl* next = tree_get_next_decl(param);
                                if (next != tree_get_decl_scope_decls_cend(info->params))
                                        cprint_comma(self);
                        }
                        if (info->params_vararg)
                        {
                                cprint_comma(self);
                                cprintrw(self, CTK_ELLIPSIS);
                        }
                        cprint_rbracket(self);
                }
        }
}

static void cprint_suffix_endings(cprinter* self, ctype_name_info* info, int opts)
{
        int suffix_it = info->nsuffixes;
        while (suffix_it)
        {
                const tree_type* t = info->suffixes[--suffix_it];
                tree_type_kind k = tree_get_type_kind(t);
                if (opts & CPRINT_IMPL_TYPE_BRACKETS)
                        cprint_rbracket(self);

                if (k == TTK_FUNCTION)
                {
                        cprint_lbracket(self);
                        TREE_FOREACH_FUNCTION_TYPE_PARAM(t, param)
                        {
                                cprint_type_name(self, *param, CPRINT_OPTS_NONE);
                                if (param + 1 != tree_get_function_type_params_end(t))
                                        cprint_comma(self);
                        }
                        if (tree_function_type_is_vararg(t))
                        {
                                cprint_comma(self);
                                cprintrw(self, CTK_ELLIPSIS);
                        }
                        cprint_rbracket(self);
                }
                else if (k == TTK_ARRAY)
                {
                        cprint_lsbracket(self);
                        if (tree_array_is(t, TAK_CONSTANT))
                                cprint_expr(self, tree_get_constant_array_size_expr(t));
                        cprint_rsbracket(self);
                }
                else if (k == TTK_PAREN)
                        cprint_rbracket(self);
        }
}

static void _cprint_type_name(
        cprinter* self,
        const tree_type* type,
        tree_id name,
        bool params_vararg,
        const tree_decl_scope* params,
        int opts)
{
        if (!type)
                return;

        ctype_name_info info;
        ctype_name_info_init(&info, type, name, params_vararg, params);
        cprint_type_parts(self, &info, opts);
        cprint_suffix_endings(self, &info, opts);
}

extern void cprint_type_name(cprinter* self, const tree_type* type, int opts)
{
        _cprint_type_name(self, type, TREE_INVALID_ID, false, NULL, opts);
}

static void cprint_decl_scope(cprinter* self, const tree_decl_scope* scope, bool braces, int opts)
{
        if (!scope)
                return;

        if (braces)
                cprint_lbrace(self, true);
        TREE_FOREACH_DECL_IN_SCOPE(scope, decl)
                if (!tree_decl_is_implicit(decl))
                {
                        cprint_endl(self);
                        cprint_decl(self, decl, CPRINT_OPTS_NONE);
                }
        if (braces)
                cprint_rbrace(self, true);
}

static void cprint_typedef(cprinter* self, const tree_decl* decl, int opts)
{
        tree_id name = tree_get_decl_name(decl);
        if (opts & CPRINT_DECL_NAME)
        {
                cprint_id(self, name);
                return;
        }

        if (!(opts & CPRINTER_IGNORE_STORAGE_SPECS))
        {
                cprintrw(self, CTK_TYPEDEF);
                cprint_space(self);
        }

        _cprint_type_name(self, tree_get_decl_type(decl), name, false, NULL, opts);
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                cprintrw(self, CTK_SEMICOLON);
}

static void cprint_struct_or_union_specifier(cprinter* self, const tree_decl* record, int opts)
{
        cprintrw(self, (tree_record_is_union(record) ? CTK_UNION : CTK_STRUCT));
        cprint_space(self);
        cprint_decl_name(self, record);
        if (opts & CPRINT_DECL_NAME)
                return;

        cprint_decl_scope(self, tree_get_record_cscope(record), true, CPRINT_OPTS_NONE);
}

static void cprint_enum_specifier(cprinter* self, const tree_decl* enum_, int opts)
{
        cprintrw(self, CTK_ENUM);
        cprint_space(self);
        cprint_decl_name(self, enum_);
        if (opts & CPRINT_DECL_NAME)
                return;

        cprint_decl_scope(self, tree_get_enum_cscope(enum_), true, CPRINT_OPTS_NONE);
}

static void cprint_decl_storage_class(cprinter* self, tree_decl_storage_class c)
{
        if (c == TDSC_NONE)
                return;

        if (c == TDSC_AUTO)
                cprintrw(self, CTK_AUTO);
        else if (c == TDSC_REGISTER)
                cprintrw(self, CTK_REGISTER);
        else if (c == TDSC_EXTERN)
                cprintrw(self, CTK_EXTERN);
        else if (c == TDSC_STATIC)
                cprintrw(self, CTK_STATIC);
        else
                return;
        cprint_space(self);
}

static void cprint_function_specifier(cprinter* self, tree_function_specifier_kind k)
{
        if (k == TFSK_INLINE)
                cprintrw(self, CTK_INLINE);
        else
                return;
        cprint_space(self);
}

static void cprint_function(cprinter* self, const tree_decl* f, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                cprint_decl_name(self, f);
                return;
        }

        if (!(opts & CPRINTER_IGNORE_STORAGE_SPECS))
                cprint_decl_storage_class(self, tree_get_decl_storage_class(f));

        cprint_function_specifier(self, tree_get_function_specifier(f));

        const tree_type* func_type = tree_get_decl_type(f);
        // since we have param-list we skip first function type
        const tree_type* restype = tree_get_function_type_result(func_type);
        _cprint_type_name(self, restype, tree_get_decl_name(f),
                tree_function_type_is_vararg(func_type), tree_get_function_cparams(f), opts);

        const tree_stmt* body = tree_get_function_body(f);
        if (body)
                cprint_stmt(self, body);
        else if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                cprintrw(self, CTK_SEMICOLON);
}

static void cprint_member(cprinter* self, const tree_decl* m, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                cprint_decl_name(self, m);
                return;
        }

        _cprint_type_name(self, tree_get_decl_type(m), tree_get_decl_name(m), false, NULL, opts);
        const tree_expr* bits = tree_get_member_bits(m);
        if (bits)
        {
                cprint_space(self);
                cprintrw(self, CTK_COLON);
                cprint_space(self);
                cprint_expr(self, bits);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                cprintrw(self, CTK_SEMICOLON);
}

static void cprint_var_decl(cprinter* self, const tree_decl* v, int opts)
{
        if (opts & CPRINT_DECL_NAME)
        {
                cprint_decl_name(self, v);
                return;
        }

        cprint_decl_storage_class(self, tree_get_decl_storage_class(v));
        _cprint_type_name(self, tree_get_decl_type(v), tree_get_decl_name(v), false, NULL, opts);

        const tree_expr* init = tree_get_var_init(v);
        if (init)
        {
                cprint_space(self);
                cprintrw(self, CTK_EQ);
                cprint_space(self);
                cprint_expr(self, init);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                cprintrw(self, CTK_SEMICOLON);
}

static void cprint_enumerator(cprinter* self, const tree_decl* e, int opts)
{
        cprint_decl_name(self, e);

        const tree_expr* value = tree_get_enumerator_expr(e);
        if (value && !tree_expr_is(value, TEK_IMPL_INIT) && !(opts & CPRINT_DECL_NAME))
        {
                cprint_space(self);
                cprintrw(self, CTK_EQ);
                cprint_space(self);
                cprint_expr(self, value);
        }
        if (!(opts & CPRINTER_IGNORE_DECL_ENDING))
                cprint_comma(self);
}

static void cprint_decl_group(cprinter* self, const tree_decl* g, int opts)
{
        bool first_printed = false;
        TREE_FOREACH_DECL_IN_GROUP(g, it)
        {
                int o = first_printed ? CPRINTER_IGNORE_SPECS : CPRINT_OPTS_NONE;
                cprint_decl(self, *it, o | CPRINTER_IGNORE_DECL_ENDING);
                first_printed = true;

                if (it + 1 != tree_get_decl_group_end(g))
                        cprint_comma(self);
                else
                        cprintrw(self, CTK_SEMICOLON);
        }
}

extern void cprint_decl(cprinter* self, const tree_decl* d, int opts)
{
        if (!d)
                return;

        switch (tree_get_decl_kind(d))
        {
                case TDK_TYPEDEF: cprint_typedef(self, d, opts); break;
                case TDK_RECORD: cprint_struct_or_union_specifier(self, d, opts); break;
                case TDK_ENUM: cprint_enum_specifier(self, d, opts); break;
                case TDK_FUNCTION: cprint_function(self, d, opts); break;
                case TDK_MEMBER: cprint_member(self, d, opts); break;
                case TDK_VAR: cprint_var_decl(self, d, opts); break;
                case TDK_ENUMERATOR: cprint_enumerator(self, d, opts); break;
                case TDK_GROUP: cprint_decl_group(self, d, opts); break;
                default: break;
        }
}

static void cprint_stmt_with_indent(cprinter*self, const tree_stmt* s)
{
        if (!s)
                return;

        if (tree_get_stmt_kind(s) != TSK_COMPOUND)
        {
                cprinter_add_indent(self);
                cprint_endl(self);
                cprint_stmt(self, s);
                cprinter_sub_indent(self);
        }
        else
                cprint_stmt(self, s);
}

static void cprint_labeled_stmt(cprinter* self, const tree_stmt* s)
{
        bool restore_indent = false;
        if (self->indent_level)
        {
                cprinter_sub_indent(self);
                restore_indent = true;
        }

        cprint_endl(self);
        const tree_decl* label = tree_get_label_stmt_decl(s);
        cprint_decl_name(self, label);
        cprintrw(self, CTK_COLON);

        if (restore_indent)
                cprinter_add_indent(self);
        cprint_endl(self);
        cprint_stmt(self, tree_get_label_decl_stmt(label));
}

static void cprint_case_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_CASE);
        cprint_space(self);
        cprint_expr(self, tree_get_case_expr(s));
        cprintrw(self, CTK_COLON);
        cprinter_add_indent(self);
        cprint_endl(self);
        cprint_stmt(self, tree_get_case_body(s));
        cprinter_sub_indent(self);
}

static void cprint_default_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_DEFAULT);
        cprintrw(self, CTK_COLON);
        cprinter_add_indent(self);
        cprint_endl(self);
        cprint_stmt(self, tree_get_default_body(s));
        cprinter_sub_indent(self);
}

static void cprint_compound_stmt(cprinter* self, const tree_stmt* s)
{
        cprint_lbrace(self, true);
        const tree_scope* scope = tree_get_compound_cscope(s);
        TREE_FOREACH_STMT(scope, stmt)
        {
                cprint_endl(self);
                cprint_stmt(self, stmt);
        }
        cprint_rbrace(self, true);
}

static void cprint_expr_stmt(cprinter* self, const tree_stmt* s)
{
        const tree_expr* expr = tree_get_expr_stmt_root(s);
        cprint_expr(self, expr);
        cprintrw(self, CTK_SEMICOLON);
        if (!expr)
                return;

        bool print_type = self->opts.print_expr_type;
        bool print_val = self->opts.print_expr_value;
        bool print_eval_result = self->opts.print_eval_result;
        if (print_eval_result || print_val || print_type)
                cprints(self, " // ");

        if (print_val)
                cprints(self, (tree_expr_is_lvalue(expr) ? "lvalue " : "rvalue "));
        if (print_type)
                cprint_type_name(self, tree_get_expr_type(expr),
                        CPRINT_TYPE_REF | CPRINT_IMPL_TYPE_BRACKETS);
        if (print_eval_result)
        {
                avalue v;
                tree_eval_info i;
                tree_init_eval_info(&i, self->target);
                if (!tree_eval_as_arithmetic(&i, expr, &v))
                        cprints(self, "not-a-constant ");
                else
                {
                        tree_type* t = tree_get_expr_type(expr);
                        if (tree_type_is_integer(t))
                                cprint_integer(self, t, avalue_get_u64(&v));
                        else if (tree_builtin_type_is(t, TBTK_FLOAT))
                                cprint_float(self, avalue_get_sp(&v));
                        else if (tree_builtin_type_is(t, TBTK_DOUBLE))
                                cprint_double(self, avalue_get_dp(&v));
                }
        }
}

static void cprint_if_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_IF);
        cprint_space(self);
        _cprint_expr(self, tree_get_if_condition(s), true);
        cprint_stmt_with_indent(self, tree_get_if_body(s));

        const tree_stmt* else_ = tree_get_if_else(s);
        if (else_)
        {
                cprint_endl(self);
                cprintrw(self, CTK_ELSE);
                cprint_space(self);
                if (tree_get_stmt_kind(else_) != TSK_IF)
                        cprint_stmt_with_indent(self, else_);
                else
                        cprint_stmt(self, else_);
        }
}

static void cprint_switch_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_SWITCH);
        cprint_space(self);
        _cprint_expr(self, tree_get_switch_expr(s), true);
        cprint_stmt_with_indent(self, tree_get_switch_body(s));
}

static void cprint_while_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_WHILE);
        cprint_space(self);
        _cprint_expr(self, tree_get_while_condition(s), true);
        cprint_stmt_with_indent(self, tree_get_while_body(s));
}

static void cprint_do_while_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_DO);
        cprint_space(self);
        cprint_stmt_with_indent(self, tree_get_do_while_body(s));
        cprint_endl(self);
        cprintrw(self, CTK_WHILE);
        cprint_space(self);
        _cprint_expr(self, tree_get_do_while_condition(s), true);
        cprintrw(self, CTK_SEMICOLON);
}

static void cprint_for_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_FOR);
        cprint_space(self);
        cprint_lbracket(self);
        cprint_stmt(self, tree_get_for_init(s));
        cprint_space(self);
        cprint_expr(self, tree_get_for_condition(s));
        cprintrw(self, CTK_SEMICOLON);
        cprint_space(self);
        cprint_expr(self, tree_get_for_step(s));
        cprint_rbracket(self);
        cprint_stmt_with_indent(self, tree_get_for_body(s));
}

static void cprint_goto_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_GOTO);
        cprint_space(self);
        cprint_decl_name(self, tree_get_goto_label(s));
        cprintrw(self, CTK_SEMICOLON);
}

static void cprint_continue_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_CONTINUE);
        cprintrw(self, CTK_SEMICOLON);
}

static void cprint_break_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_BREAK);
        cprintrw(self, CTK_SEMICOLON);
}

static void cprint_decl_stmt(cprinter* self, const tree_stmt* s)
{
        cprint_decl(self, tree_get_decl_stmt_entity(s), CPRINT_OPTS_NONE);
}

static void cprint_return_stmt(cprinter* self, const tree_stmt* s)
{
        cprintrw(self, CTK_RETURN);
        cprint_space(self);
        cprint_expr(self, tree_get_return_value(s));
        cprintrw(self, CTK_SEMICOLON);
}

extern void cprint_stmt(cprinter* self, const tree_stmt* s)
{
        if (!s)
                return;

        switch (tree_get_stmt_kind(s))
        {
                case TSK_LABELED: cprint_labeled_stmt(self, s); break;
                case TSK_CASE: cprint_case_stmt(self, s); break;
                case TSK_DEFAULT: cprint_default_stmt(self, s); break;
                case TSK_COMPOUND: cprint_compound_stmt(self, s); break;
                case TSK_EXPR: cprint_expr_stmt(self, s); break;
                case TSK_IF: cprint_if_stmt(self, s); break;
                case TSK_SWITCH: cprint_switch_stmt(self, s); break;
                case TSK_WHILE: cprint_while_stmt(self, s); break;
                case TSK_DO_WHILE: cprint_do_while_stmt(self, s); break;
                case TSK_FOR: cprint_for_stmt(self, s); break;
                case TSK_GOTO: cprint_goto_stmt(self, s); break;
                case TSK_CONTINUE: cprint_continue_stmt(self, s); break;
                case TSK_BREAK: cprint_break_stmt(self, s); break;
                case TSK_DECL: cprint_decl_stmt(self, s); break;
                case TSK_RETURN: cprint_return_stmt(self, s); break;
                default: break;
        }
}

extern void cprint_module(cprinter* self, const tree_module* module)
{
        cprint_decl_scope(self, tree_get_module_cglobals(module), false, CPRINT_OPTS_NONE);
}