#include "scc/c/c-parse-decl.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-parse-stmt.h"
#include "scc/c/c-parse-expr.h" // cparse_const_expr
#include "scc/c/c-info.h"
#include "scc/c/c-context.h"
#include "scc/c/c-errors.h"
#include "c-builtin-type.h"

static tree_decl* c_parse_function_or_init_declarator(
        c_parser* self, c_decl_specs* specs, bool func_def_expected, bool* func_has_def)
{
        c_declarator d;
        c_declarator_init(&d, self->sema, CDK_UNKNOWN);
        if (!c_parse_declarator(self, &d))
        {
                c_declarator_dispose(&d);
                return NULL;
        }

        tree_decl* decl = c_sema_declare_external_decl(self->sema, specs, &d);
        c_declarator_dispose(&d);
        if (!decl)
                return NULL;

        tree_decl_kind dk = tree_get_decl_kind(decl);
        if (c_parser_at(self, CTK_EQ))
        {
                if (dk != TDK_VAR)
                {
                        if (dk == TDK_FUNCTION)
                                c_error_function_initialized_like_a_variable(self->logger, decl);
                        return NULL;
                }

                c_parser_consume_token(self);
                tree_expr* init = c_parse_initializer(self, tree_get_decl_type(decl));
                if (!init)
                        return NULL;

                return c_sema_define_var_decl(self->sema, decl, init);
        }
        else if (func_def_expected && dk == TDK_FUNCTION && c_parser_at(self, CTK_LBRACE))
        {
                if (!c_sema_check_func_def_loc(self->sema, decl))
                        return NULL;

                c_sema_enter_function(self->sema, decl);
                tree_stmt* body = c_parse_stmt(self);
                c_sema_exit_function(self->sema);

                if (!body)
                        return NULL;

                *func_has_def = true;
                return c_sema_define_func_decl(self->sema, decl, body);
        }
        return decl;
}

static const c_token_kind ctk_lbracket_or_id[] =
{
        CTK_ID,
        CTK_LBRACKET,
        CTK_UNKNOWN,
};

static const c_token_kind ctk_comma_eq_semicolon_lbracket[] =
{
        CTK_COMMA,
        CTK_EQ,
        CTK_SEMICOLON,
        CTK_UNKNOWN,
};

static tree_decl* c_parse_function_or_init_declarator_list(c_parser* self, c_decl_specs* specs)
{
        tree_decl* group = NULL;
        bool func_def_expected = true;
        while (1)
        {
                bool func_has_def = false;
                tree_decl* d = c_parse_function_or_init_declarator(
                        self, specs, func_def_expected, &func_has_def);
                if (!d)
                        return NULL;

                if (!(group = c_sema_add_decl_to_group(self->sema, group, d)))
                        return NULL;

                if (func_has_def)
                        return group;
                else if (c_parser_at(self, CTK_SEMICOLON))
                {
                        c_parser_consume_token(self);
                        return group;
                }
                else if (c_parser_at(self, CTK_COMMA))
                        c_parser_consume_token(self);
                else
                {
                        c_parser_require_ex(self, CTK_UNKNOWN,
                                tree_decl_is_anon(d)
                                        ? ctk_lbracket_or_id
                                        : ctk_comma_eq_semicolon_lbracket);
                        return NULL;
                }

                func_def_expected = false;
        }
}

extern tree_decl* c_parse_decl(c_parser* self)
{
        c_decl_specs specs;
        c_decl_specs_init(&specs);
        if (!c_parse_decl_specs(self, &specs))
                return NULL;

        if (c_token_starts_declarator(c_parser_get_token(self)))
                return c_parse_function_or_init_declarator_list(self, &specs);

        if (!c_parser_require(self, CTK_SEMICOLON))
                return NULL;

        return c_sema_handle_unused_decl_specs(self->sema, &specs);
}

static inline bool c_parser_token_is_type_specifier(const c_parser* self, const c_token* t)
{
        if (!c_token_is_type_specifier(t))
                return false;

        if (c_token_is(t, CTK_ID))
                return c_sema_typedef_name_exists(self->sema, c_token_get_string(t));

        return true;
}

static inline bool c_parser_at_type_specifier(const c_parser* self)
{
        return c_parser_token_is_type_specifier(self, c_parser_get_token(self));
}

extern bool c_parser_at_declaration(c_parser* self)
{
        c_token* t = c_parser_get_token(self);
        return c_parser_token_starts_type_name(self, t)
                || c_token_is_decl_storage_class(t)
                || c_token_is(t, CTK_TYPEDEF);
}

extern bool c_parse_decl_specs(c_parser* self, c_decl_specs* result)
{
        c_decl_specs_set_loc_begin(result, c_parser_get_loc(self));
        tree_type_quals quals = TTQ_UNQUALIFIED;
        while (1)
        {
                const c_token* t = c_parser_get_token(self);
                if (c_parser_at_type_specifier(self) && !result->typespec)
                {
                        tree_type* typespec = c_parse_type_specifier(self);
                        if (!typespec)
                                return false;
                        if (!c_sema_set_type_specifier(self->sema, result, typespec))
                                return false;
                }
                else if (c_token_is_type_qualifier(t))
                        quals |= c_parse_type_qualifier_list_opt(self);
                else if (c_token_is_decl_storage_class(t))
                {
                        tree_decl_storage_class c = c_token_to_decl_storage_class(t);
                        if (!c_sema_set_decl_storage_class(self->sema, result, c))
                                return false;
                        c_parser_consume_token(self);
                }
                else if (c_parser_at(self, CTK_TYPEDEF))
                {
                        if (!c_sema_set_typedef_specifier(self->sema, result))
                                return false;
                        c_parser_consume_token(self);
                }
                else if (c_parser_at(self, CTK_INLINE))
                {
                        if (!c_sema_set_inline_specifier(self->sema, result))
                                return false;
                        c_parser_consume_token(self);
                }
                else
                        break;
        }

        if (!result->typespec)
        {
                if (c_parser_at(self, CTK_ID))
                        c_error_unknown_type_name(self->logger, c_parser_get_token(self));
                else
                        c_error_expected_type_specifier(self->logger,
                                c_decl_specs_get_loc_begin(result));
                return false;
        }

        c_decl_specs_set_loc_end(result, c_parser_get_loc(self));
        tree_set_type_quals(result->typespec, quals);
        return true;
}

static tree_type* c_parse_builtin_type_specifier(c_parser* self)
{
        tree_location begin = c_parser_get_loc(self);
        c_builtin_type type;
        c_builtin_type_init(&type);
        while (1)
        {
                bool correct = true;
                c_token_kind k = c_token_get_kind(c_parser_get_token(self));
                if (k == CTK_VOID)
                        correct = c_builtin_type_set_kind(&type, TBTK_VOID);
                else if (k == CTK_CHAR)
                        correct = c_builtin_type_set_kind(&type, TBTK_INT8);
                else if (k == CTK_INT)
                        correct = c_builtin_type_set_kind(&type, TBTK_INT32);
                else if (k == CTK_FLOAT)
                        correct = c_builtin_type_set_kind(&type, TBTK_FLOAT);
                else if (k == CTK_DOUBLE)
                        correct = c_builtin_type_set_kind(&type, TBTK_DOUBLE);
                else if (k == CTK_SIGNED)
                        correct = c_builtin_type_add_signed_specifier(&type);
                else if (k == CTK_UNSIGNED)
                        correct = c_builtin_type_add_unsigned_specifier(&type);
                else if (k == CTK_SHORT)
                        correct = c_builtin_type_add_short_specifier(&type);
                else if (k == CTK_LONG)
                        correct = c_builtin_type_add_long_specifier(&type);
                else
                        break;

                if (!correct)
                {
                        c_error_invalid_type_specifier(self->logger, begin);
                        return NULL;
                }
                c_parser_consume_token(self);
        }

        tree_builtin_type_kind k = c_builtin_type_get_kind(&type);
        if (k == TBTK_INVALID)
        {
                c_error_expected_type_specifier(self->logger, c_parser_get_loc(self));
                return NULL;
        }
        return c_sema_get_builtin_type(self->sema, TTQ_UNQUALIFIED, k);
}

extern tree_type* c_parse_type_specifier(c_parser* self)
{
        c_token_kind k = c_token_get_kind(c_parser_get_token(self));
        if (k == CTK_STRUCT || k == CTK_UNION || k == CTK_ENUM)
        {
                bool referenced = false;
                tree_decl* specifier = k == CTK_ENUM
                        ? c_parse_enum_specifier(self, &referenced)
                        : c_parse_struct_or_union_specifier(self, &referenced);

                return c_sema_new_decl_type(self->sema, specifier, referenced);
        }
        else if (k == CTK_ID)
                return c_parse_typedef_name(self);

        return c_parse_builtin_type_specifier(self);
}

extern int c_parse_type_qualifier_list_opt(c_parser* self)
{
        tree_type_quals quals = TTQ_UNQUALIFIED;
        while (!c_parser_at(self, CTK_EOF))
        {
                tree_type_quals cur = TTQ_UNQUALIFIED;
                if ((cur |= c_token_to_type_qualifier(c_parser_get_token(self))) == TTQ_UNQUALIFIED)
                        return quals;

                quals |= cur;
                c_parser_consume_token(self);
        }
        return quals;
}

extern tree_type* c_parse_specifier_qualifier_list(c_parser* self)
{
        tree_type_quals quals = c_parse_type_qualifier_list_opt(self);
        tree_type* typespec = c_parse_type_specifier(self);
        if (!typespec)
                return NULL;

        tree_set_type_quals(typespec, quals | c_parse_type_qualifier_list_opt(self));
        return typespec;
}

static const c_token_kind ctk_semicolon_or_comma[] =
{
        CTK_SEMICOLON,
        CTK_COMMA,
        CTK_UNKNOWN,
};

static bool c_parse_struct_declaration(c_parser* self)
{
        c_decl_specs ds;
        c_decl_specs_init(&ds);
        if (!c_parse_decl_specs(self, &ds))
                return false;

        tree_decl* group = NULL;
        while (1)
        {
                c_declarator sd;
                c_declarator_init(&sd, self->sema, CDK_MEMBER);

                if (!c_parse_declarator(self, &sd))
                {
                        c_declarator_dispose(&sd);
                        return false;
                }

                tree_expr* bits = NULL;
                if (c_parser_at(self, CTK_COLON))
                {
                        c_parser_consume_token(self);
                        bits = c_parse_const_expr(self);
                }

                tree_decl* field = c_sema_define_field_decl(self->sema, &ds, &sd, bits);
                c_declarator_dispose(&sd);
                if (!field)
                        return false;
                if (!(group = c_sema_add_decl_to_group(self->sema, group, field)))
                        return false;

                if (c_parser_at(self, CTK_SEMICOLON))
                {
                        c_parser_consume_token(self);
                        return true;
                }
                else if (!c_parser_require_ex(self, CTK_COMMA, ctk_semicolon_or_comma))
                        return false;
        }
}

static bool c_parse_struct_declaration_list(c_parser* self, tree_decl* record)
{
        if (c_parser_at(self, CTK_RBRACE))
        {
                c_error_empty_struct(self->logger, c_parser_get_loc(self));
                return false;
        }

        bool res = true;
        c_sema_enter_decl_scope(self->sema, tree_get_record_fields(record));
        while (!c_parser_at(self, CTK_RBRACE))
                if (!c_parse_struct_declaration(self))
                {
                        res = false;
                        break;
                }
        c_sema_exit_decl_scope(self->sema);
        return res;
}

static const c_token_kind ctk_struct_or_union[] =
{
        CTK_STRUCT,
        CTK_UNION,
        CTK_UNKNOWN,
};

extern tree_decl* c_parse_struct_or_union_specifier(c_parser* self, bool* referenced)
{
        tree_location kw_loc = c_parser_get_loc(self);

        bool is_union = false;
        if (c_parser_at(self, CTK_UNION))
        {
                is_union = true;
                c_parser_consume_token(self);
        }
        else if (!c_parser_require_ex(self, CTK_STRUCT, ctk_struct_or_union))
                return NULL;

        tree_id name = TREE_EMPTY_ID;
        if (c_parser_at(self, CTK_ID))
        {
                name = c_token_get_string(c_parser_get_token(self));
                c_parser_consume_token(self);
        }

        if (!c_parser_at(self, CTK_LBRACE))
        {
                if (referenced)
                        *referenced = true;
                return c_sema_declare_record_decl(self->sema, kw_loc, name, is_union);
        }

        tree_decl* record = c_sema_define_record_decl(self->sema, kw_loc, name, is_union);
        if (!record)
                return NULL;

        c_parser_consume_token(self);
        if (!c_parse_struct_declaration_list(self, record))
                return NULL;
        tree_location rbrace_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACE))
                return NULL;

        return c_sema_complete_record_decl(self->sema, record, rbrace_loc);
}

static tree_decl* c_parse_enumerator(c_parser* self, tree_decl* enum_)
{
        tree_id id_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_ID))
                return NULL;

        tree_id id = c_token_get_string(c_parser_get_prev(self));
        tree_expr* value = NULL;

        if (c_parser_at(self, CTK_EQ))
        {
                c_parser_consume_token(self);
                if (!(value = c_parse_const_expr(self)))
                        return NULL;
        }

        return c_sema_define_enumerator_decl(self->sema, enum_, id_loc, id, value);
}

static c_token_kind ctk_rbrace_or_comma[] =
{
        CTK_RBRACE,
        CTK_COMMA,
        CTK_UNKNOWN,
};

static bool c_parse_enumerator_list(c_parser* self, tree_decl* enum_)
{
        if (c_parser_at(self, CTK_RBRACE))
        {
                c_error_empty_enum(self->logger, c_parser_get_loc(self));
                return false;
        }
        bool res = false;
        c_sema_enter_decl_scope(self->sema, tree_get_enum_values(enum_));
        while (1)
        {
                tree_decl* enumerator = c_parse_enumerator(self, enum_);
                if (!enumerator)
                        break;

                if (c_parser_at(self, CTK_COMMA))
                        c_parser_consume_token(self);

                if (c_parser_at(self, CTK_RBRACE))
                {
                        res = true;
                        break;
                }
                else if (!c_parser_at(self, CTK_ID))
                {
                        c_parser_require_ex(self, CTK_UNKNOWN, ctk_rbrace_or_comma);
                        break;
                }
        }
        c_sema_exit_decl_scope(self->sema);
        return res;
}

extern tree_decl* c_parse_enum_specifier(c_parser* self, bool* referenced)
{
        tree_location kw_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_ENUM))
                return NULL;

        tree_id name = TREE_EMPTY_ID;
        if (c_parser_at(self, CTK_ID))
        {
                name = c_token_get_string(c_parser_get_token(self));
                c_parser_consume_token(self);
        }

        if (!c_parser_at(self, CTK_LBRACE))
        {
                if (referenced)
                        *referenced = true;
                return c_sema_declare_enum_decl(self->sema, kw_loc, name);
        }

        tree_decl* enum_ = c_sema_define_enum_decl(self->sema, kw_loc, name);
        if (!enum_)
                return NULL;

        c_parser_consume_token(self);
        if (!c_parse_enumerator_list(self, enum_))
                return NULL;

        tree_location rbrace_loc = c_parser_get_loc(self);
        if (!c_parser_require(self, CTK_RBRACE))
                return NULL;

        return c_sema_complete_enum_decl(self->sema, enum_, rbrace_loc);
}

static bool c_parse_pointer_opt(c_parser* self, c_type_chain* result)
{
        while (c_parser_at(self, CTK_STAR))
        {
                c_parser_consume_token(self);
                tree_type_quals quals = c_parse_type_qualifier_list_opt(self);

                if (!(result->head = c_sema_new_pointer_type(self->sema, quals, result->head)))
                        return false;
                if (!result->tail)
                        result->tail = result->head;
        }
        return true;
}

static c_param* c_parse_param_declaration(c_parser* self)
{
        c_param* p = c_sema_new_param(self->sema);
        if (!c_parse_decl_specs(self, &p->specs))
                return NULL;

        if (c_token_starts_declarator(c_parser_get_token(self)))
                if (!c_parse_declarator(self, &p->declarator))
                        return NULL;

        return c_sema_finish_param(self->sema, p);
}

static bool c_parse_parameter_type_list_opt(c_parser* self, c_declarator* result)
{
        if (c_parser_at(self, CTK_RBRACKET))
        {
                c_declarator_set_initialized(result);
                return true;
        }

        while (1)
        {
                if (c_parser_at(self, CTK_ELLIPSIS))
                {
                        tree_location loc = c_parser_get_loc(self);
                        if (!c_sema_set_declarator_has_vararg(self->sema, result, loc))
                                return false;
                        
                        c_parser_consume_token(self);
                        c_declarator_set_initialized(result);
                        return true;
                }

                c_param* p = c_parse_param_declaration(self);
                if (!p)
                        return false;

                c_sema_add_declarator_param(self->sema, result, p);
                if (c_parser_at(self, CTK_RBRACKET))
                {
                        c_declarator_set_initialized(result);
                        return true;
                }
                else if (!c_parser_require_ex(self, CTK_COMMA, ctk_rbracket_or_comma))
                        return false;
        }
        UNREACHABLE();
        return false;
}

static bool c_parse_direct_declarator_suffix_opt(c_parser* self, c_declarator* result)
{
        if (c_parser_at(self, CTK_LBRACKET))
        {
                c_parser_consume_token(self);
                if (!c_sema_add_direct_declarator_function_suffix(self->sema, result))
                        return false;
                if (!c_parse_parameter_type_list_opt(self, result))
                        return false;
                if (!c_parser_require(self, CTK_RBRACKET))
                        return false;
        }
        else if (c_parser_at(self, CTK_LSBRACKET))
        {
                c_parser_consume_token(self);
                tree_expr* size = NULL;
                if (!c_parser_at(self, CTK_RSBRACKET))
                        if (!(size = c_parse_const_expr(self)))
                                return false;

                if (!c_sema_add_direct_declarator_array_suffix(
                        self->sema, result, TTQ_UNQUALIFIED, size))
                {
                        return false;
                }
                if (!c_parser_require(self, CTK_RSBRACKET))
                        return false;
        }
        else
                return true;

        return c_parse_direct_declarator_suffix_opt(self, result);
}

static bool c_parse_direct_declarator(c_parser* self, c_declarator* result)
{
        if (c_parser_at(self, CTK_ID))
        {
                tree_id id = c_token_get_string(c_parser_get_token(self));
                tree_location id_loc = c_parser_get_loc(self);
                c_declarator_set_name(result, id_loc, id);
                c_parser_consume_token(self);
        }
        else if (c_parser_require(self, CTK_LBRACKET))
        {
                if (!c_parse_declarator(self, result))
                        return false;
                if (!c_parser_require(self, CTK_RBRACKET))
                        return false;
                if (!c_sema_add_direct_declarator_parens(self->sema, result))
                        return false;
        }
        else
                return false;

        return c_parse_direct_declarator_suffix_opt(self, result);
}

extern bool c_parse_declarator(c_parser* self, c_declarator* result)
{
        c_declarator_set_loc_begin(result, c_parser_get_loc(self));

        c_type_chain pointer;
        c_type_chain_init(&pointer);
        if (!c_parse_pointer_opt(self, &pointer))
                return false;

        if (c_parser_at(self, CTK_LBRACKET) || c_parser_at(self, CTK_ID))
                if (!c_parse_direct_declarator(self, result))
                        return false;

        c_declarator_set_loc_end(result, c_parser_get_loc(self));
        return c_sema_finish_declarator(self->sema, result, &pointer);
}

extern tree_type* c_parse_type_name(c_parser* self)
{
        tree_type* t = c_parse_specifier_qualifier_list(self);
        if (!t)
                return NULL;

        if (c_token_starts_declarator(c_parser_get_token(self)))
        {
                c_declarator d;
                c_declarator_init(&d, self->sema, CDK_TYPE_NAME);
                if (!c_parse_declarator(self, &d))
                {
                        c_declarator_dispose(&d);
                        return NULL;
                }

                t = c_sema_new_type_name(self->sema, &d, t);
                c_declarator_dispose(&d);
        }
        return t;
}

extern tree_type* c_parse_paren_type_name(c_parser* self)
{
        if (!c_parser_require(self, CTK_LBRACKET))
                return NULL;

        tree_type* t = c_parse_type_name(self);
        if (!t)
                return NULL;

        return c_parser_require(self, CTK_RBRACKET)
                ? t : NULL;
}

extern bool c_parser_token_starts_type_name(const c_parser* self, const c_token* t)
{
        return c_token_is_type_qualifier(t) || c_parser_token_is_type_specifier(self, t);
}

extern bool c_parser_next_token_starts_type_name(const c_parser* self)
{
        return c_parser_token_starts_type_name(self, c_parser_get_next(self));
}

extern tree_type* c_parse_typedef_name(c_parser* self)
{
        c_token* id = c_parser_get_token(self);
        if (!c_parser_require(self, CTK_ID))
                return NULL;

        return c_sema_new_typedef_name(self->sema,
                c_token_get_loc(id), c_token_get_string(id));
}