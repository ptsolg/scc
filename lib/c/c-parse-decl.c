#include "scc/c/c-parse-decl.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-sema-type.h"
#include "scc/c/c-parse-stmt.h"
#include "scc/c/c-parse-expr.h" // cparse_const_expr
#include "scc/c/c-info.h"
#include "scc/c/c-context.h"
#include "scc/c/c-errors.h"
#include "c-builtin-type.h"

static tree_decl* cparse_function_or_init_declarator(
        cparser* self, cdecl_specs* specs, bool func_def_expected, bool* func_has_def)
{
        cdeclarator d;
        cdeclarator_init(&d, self->sema, CDK_UNKNOWN);
        if (!cparse_declarator(self, &d))
        {
                cdeclarator_dispose(&d);
                return NULL;
        }

        tree_decl* decl = csema_declare_external_decl(self->sema, specs, &d);
        cdeclarator_dispose(&d);
        if (!decl)
                return NULL;

        tree_decl_kind dk = tree_get_decl_kind(decl);
        if (cparser_at(self, CTK_EQ))
        {
                if (dk != TDK_VAR)
                {
                        if (dk == TDK_FUNCTION)
                                cerror_function_initialized_like_a_variable(self->logger, decl);
                        return NULL;
                }

                cparser_consume_token(self);
                tree_expr* init = cparse_initializer(self, tree_get_decl_type(decl));
                if (!init)
                        return NULL;

                return csema_define_var_decl(self->sema, decl, init);
        }
        else if (func_def_expected && dk == TDK_FUNCTION && cparser_at(self, CTK_LBRACE))
        {
                if (!csema_check_func_def_loc(self->sema, decl))
                        return NULL;

                csema_enter_function(self->sema, decl);
                tree_stmt* body = cparse_stmt(self);
                csema_exit_function(self->sema);

                if (!body)
                        return NULL;

                *func_has_def = true;
                return csema_define_func_decl(self->sema, decl, body);
        }
        return decl;
}

static const ctoken_kind ctk_lbracket_or_id[] =
{
        CTK_ID,
        CTK_LBRACKET,
        CTK_UNKNOWN,
};

static const ctoken_kind ctk_comma_eq_semicolon_lbracket[] =
{
        CTK_COMMA,
        CTK_EQ,
        CTK_SEMICOLON,
        CTK_UNKNOWN,
};

static tree_decl* cparse_function_or_init_declarator_list(cparser* self, cdecl_specs* specs)
{
        tree_decl* group = NULL;
        bool func_def_expected = true;
        while (1)
        {
                bool func_has_def = false;
                tree_decl* d = cparse_function_or_init_declarator(
                        self, specs, func_def_expected, &func_has_def);
                if (!d)
                        return NULL;

                if (!(group = csema_add_decl_to_group(self->sema, group, d)))
                        return NULL;

                if (func_has_def)
                        return group;
                else if (cparser_at(self, CTK_SEMICOLON))
                {
                        cparser_consume_token(self);
                        return group;
                }
                else if (cparser_at(self, CTK_COMMA))
                        cparser_consume_token(self);
                else
                {
                        cparser_require_ex(self, CTK_UNKNOWN,
                                tree_decl_is_anon(d)
                                        ? ctk_lbracket_or_id
                                        : ctk_comma_eq_semicolon_lbracket);
                        return NULL;
                }

                func_def_expected = false;
        }
}

extern tree_decl* cparse_decl(cparser* self)
{
        cdecl_specs specs;
        cdecl_specs_init(&specs);
        if (!cparse_decl_specs(self, &specs))
                return NULL;

        if (ctoken_starts_declarator(cparser_get_token(self)))
                return cparse_function_or_init_declarator_list(self, &specs);

        if (!cparser_require(self, CTK_SEMICOLON))
                return NULL;

        return csema_handle_unused_decl_specs(self->sema, &specs);
}

static inline bool cparser_token_is_type_specifier(const cparser* self, const ctoken* t)
{
        if (!ctoken_is_type_specifier(t))
                return false;

        if (ctoken_is(t, CTK_ID))
                return csema_typedef_name_exists(self->sema, ctoken_get_string(t));

        return true;
}

static inline bool cparser_at_type_specifier(const cparser* self)
{
        return cparser_token_is_type_specifier(self, cparser_get_token(self));
}

extern bool cparser_at_declaration(cparser* self)
{
        ctoken* t = cparser_get_token(self);
        return cparser_token_starts_type_name(self, t)
                || ctoken_is_decl_storage_class(t)
                || ctoken_is(t, CTK_TYPEDEF);
}

extern bool cparse_decl_specs(cparser* self, cdecl_specs* result)
{
        cdecl_specs_set_loc_begin(result, cparser_get_loc(self));
        tree_type_quals quals = TTQ_UNQUALIFIED;
        while (1)
        {
                const ctoken* t = cparser_get_token(self);
                if (cparser_at_type_specifier(self) && !result->typespec)
                {
                        tree_type* typespec = cparse_type_specifier(self);
                        if (!typespec)
                                return false;
                        if (!csema_set_typespec(self->sema, result, typespec))
                                return false;
                }
                else if (ctoken_is_type_qualifier(t))
                        quals |= cparse_type_qualifier_list_opt(self);
                else if (ctoken_is_decl_storage_class(t))
                {
                        tree_decl_storage_class c = ctoken_to_decl_storage_class(t);
                        if (!csema_set_decl_storage_class(self->sema, result, c))
                                return false;
                        cparser_consume_token(self);
                }
                else if (cparser_at(self, CTK_TYPEDEF))
                {
                        if (!csema_set_typedef_spec(self->sema, result))
                                return false;
                        cparser_consume_token(self);
                }
                else if (cparser_at(self, CTK_INLINE))
                {
                        if (!csema_set_inline_spec(self->sema, result))
                                return false;
                        cparser_consume_token(self);
                }
                else
                        break;
        }

        if (!result->typespec)
        {
                if (cparser_at(self, CTK_ID))
                        cerror_unknown_type_name(self->logger, cparser_get_token(self));
                else
                        cerror_expected_type_specifier(self->logger,
                                cdecl_specs_get_loc_begin(result));
                return false;
        }

        cdecl_specs_set_loc_end(result, cparser_get_loc(self));
        tree_set_type_quals(result->typespec, quals);
        return true;
}

static tree_type* cparse_builtin_type_specifier(cparser* self)
{
        tree_location begin = cparser_get_loc(self);
        cbuiltin_type type;
        cbuiltin_type_init(&type);
        while (1)
        {
                bool correct = true;
                ctoken_kind k = ctoken_get_kind(cparser_get_token(self));
                if (k == CTK_VOID)
                        correct = cbuiltin_type_set_kind(&type, TBTK_VOID);
                else if (k == CTK_CHAR)
                        correct = cbuiltin_type_set_kind(&type, TBTK_INT8);
                else if (k == CTK_INT)
                        correct = cbuiltin_type_set_kind(&type, TBTK_INT32);
                else if (k == CTK_FLOAT)
                        correct = cbuiltin_type_set_kind(&type, TBTK_FLOAT);
                else if (k == CTK_DOUBLE)
                        correct = cbuiltin_type_set_kind(&type, TBTK_DOUBLE);
                else if (k == CTK_SIGNED)
                        correct = cbuiltin_type_add_signed_specifier(&type);
                else if (k == CTK_UNSIGNED)
                        correct = cbuiltin_type_add_unsigned_specifier(&type);
                else if (k == CTK_SHORT)
                        correct = cbuiltin_type_add_short_specifier(&type);
                else if (k == CTK_LONG)
                        correct = cbuiltin_type_add_long_specifier(&type);
                else
                        break;

                if (!correct)
                {
                        cerror_invalid_type_specifier(self->logger, begin);
                        return NULL;
                }
                cparser_consume_token(self);
        }

        tree_builtin_type_kind k = cbuiltin_type_get_kind(&type);
        if (k == TBTK_INVALID)
        {
                cerror_expected_type_specifier(self->logger, cparser_get_loc(self));
                return NULL;
        }
        return csema_new_builtin_type(self->sema, TTQ_UNQUALIFIED, k);
}

extern tree_type* cparse_type_specifier(cparser* self)
{
        ctoken_kind k = ctoken_get_kind(cparser_get_token(self));
        if (k == CTK_STRUCT || k == CTK_UNION || k == CTK_ENUM)
        {
                bool referenced = false;
                tree_decl* specifier = k == CTK_ENUM
                        ? cparse_enum_specifier(self, &referenced)
                        : cparse_struct_or_union_specifier(self, &referenced);

                return csema_new_decl_type(self->sema, specifier, referenced);
        }
        else if (k == CTK_ID)
                return cparse_typedef_name(self);

        return cparse_builtin_type_specifier(self);
}

extern int cparse_type_qualifier_list_opt(cparser* self)
{
        tree_type_quals quals = TTQ_UNQUALIFIED;
        while (!cparser_at(self, CTK_EOF))
        {
                tree_type_quals cur = TTQ_UNQUALIFIED;
                if ((cur |= ctoken_to_type_qualifier(cparser_get_token(self))) == TTQ_UNQUALIFIED)
                        return quals;

                quals |= cur;
                cparser_consume_token(self);
        }
        return quals;
}

extern tree_type* cparse_specifier_qualifier_list(cparser* self)
{
        tree_type_quals quals = cparse_type_qualifier_list_opt(self);
        tree_type* typespec = cparse_type_specifier(self);
        if (!typespec)
                return NULL;

        tree_set_type_quals(typespec, quals | cparse_type_qualifier_list_opt(self));
        return typespec;
}

static const ctoken_kind ctk_semicolon_or_comma[] =
{
        CTK_SEMICOLON,
        CTK_COMMA,
        CTK_UNKNOWN,
};

static bool cparse_struct_declaration(cparser* self)
{
        cdecl_specs ds;
        cdecl_specs_init(&ds);
        if (!cparse_decl_specs(self, &ds))
                return false;

        tree_decl* group = NULL;
        while (1)
        {
                cdeclarator sd;
                cdeclarator_init(&sd, self->sema, CDK_MEMBER);

                if (!cparse_declarator(self, &sd))
                {
                        cdeclarator_dispose(&sd);
                        return false;
                }

                tree_expr* bits = NULL;
                if (cparser_at(self, CTK_COLON))
                {
                        cparser_consume_token(self);
                        bits = cparse_const_expr(self);
                }

                tree_decl* field = csema_define_field_decl(self->sema, &ds, &sd, bits);
                cdeclarator_dispose(&sd);
                if (!field)
                        return false;
                if (!(group = csema_add_decl_to_group(self->sema, group, field)))
                        return false;

                if (cparser_at(self, CTK_SEMICOLON))
                {
                        cparser_consume_token(self);
                        return true;
                }
                else if (!cparser_require_ex(self, CTK_COMMA, ctk_semicolon_or_comma))
                        return false;
        }
}

static bool cparse_struct_declaration_list(cparser* self, tree_decl* record)
{
        if (cparser_at(self, CTK_RBRACE))
        {
                cerror_empty_struct(self->logger, cparser_get_loc(self));
                return false;
        }

        bool res = true;
        csema_enter_decl_scope(self->sema, tree_get_record_fields(record));
        while (!cparser_at(self, CTK_RBRACE))
                if (!cparse_struct_declaration(self))
                {
                        res = false;
                        break;
                }
        csema_exit_decl_scope(self->sema);
        return res;
}

static const ctoken_kind ctk_struct_or_union[] =
{
        CTK_STRUCT,
        CTK_UNION,
        CTK_UNKNOWN,
};

extern tree_decl* cparse_struct_or_union_specifier(cparser* self, bool* referenced)
{
        tree_location kw_loc = cparser_get_loc(self);

        bool is_union = false;
        if (cparser_at(self, CTK_UNION))
        {
                is_union = true;
                cparser_consume_token(self);
        }
        else if (!cparser_require_ex(self, CTK_STRUCT, ctk_struct_or_union))
                return NULL;

        tree_id name = TREE_EMPTY_ID;
        if (cparser_at(self, CTK_ID))
        {
                name = ctoken_get_string(cparser_get_token(self));
                cparser_consume_token(self);
        }

        if (!cparser_at(self, CTK_LBRACE))
        {
                if (referenced)
                        *referenced = true;
                return csema_declare_record_decl(self->sema, kw_loc, name, is_union);
        }

        tree_decl* record = csema_define_record_decl(self->sema, kw_loc, name, is_union);
        if (!record)
                return NULL;

        cparser_consume_token(self);
        if (!cparse_struct_declaration_list(self, record))
                return NULL;
        tree_location rbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACE))
                return NULL;

        return csema_complete_record_decl(self->sema, record, rbrace_loc);
}

static tree_decl* cparse_enumerator(cparser* self, tree_decl* enum_)
{
        tree_id id_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ID))
                return NULL;

        tree_id id = ctoken_get_string(cparser_get_prev(self));
        tree_expr* value = NULL;

        if (cparser_at(self, CTK_EQ))
        {
                cparser_consume_token(self);
                if (!(value = cparse_const_expr(self)))
                        return NULL;
        }

        return csema_define_enumerator_decl(self->sema, enum_, id_loc, id, value);
}

static ctoken_kind ctk_rbrace_or_comma[] =
{
        CTK_RBRACE,
        CTK_COMMA,
        CTK_UNKNOWN,
};

static bool cparse_enumerator_list(cparser* self, tree_decl* enum_)
{
        if (cparser_at(self, CTK_RBRACE))
        {
                cerror_empty_enum(self->logger, cparser_get_loc(self));
                return false;
        }
        bool res = false;
        csema_enter_decl_scope(self->sema, tree_get_enum_values(enum_));
        while (1)
        {
                tree_decl* enumerator = cparse_enumerator(self, enum_);
                if (!enumerator)
                        break;

                if (cparser_at(self, CTK_COMMA))
                        cparser_consume_token(self);

                if (cparser_at(self, CTK_RBRACE))
                {
                        res = true;
                        break;
                }
                else if (!cparser_at(self, CTK_ID))
                {
                        cparser_require_ex(self, CTK_UNKNOWN, ctk_rbrace_or_comma);
                        break;
                }
        }
        csema_exit_decl_scope(self->sema);
        return res;
}

extern tree_decl* cparse_enum_specifier(cparser* self, bool* referenced)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ENUM))
                return NULL;

        tree_id name = TREE_EMPTY_ID;
        if (cparser_at(self, CTK_ID))
        {
                name = ctoken_get_string(cparser_get_token(self));
                cparser_consume_token(self);
        }

        if (!cparser_at(self, CTK_LBRACE))
        {
                if (referenced)
                        *referenced = true;
                return csema_declare_enum_decl(self->sema, kw_loc, name);
        }

        tree_decl* enum_ = csema_define_enum_decl(self->sema, kw_loc, name);
        if (!enum_)
                return NULL;

        cparser_consume_token(self);
        if (!cparse_enumerator_list(self, enum_))
                return NULL;

        tree_location rbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACE))
                return NULL;

        return csema_complete_enum_decl(self->sema, enum_, rbrace_loc);
}

static bool cparse_pointer_opt(cparser* self, ctype_chain* result)
{
        while (cparser_at(self, CTK_STAR))
        {
                cparser_consume_token(self);
                tree_type_quals quals = cparse_type_qualifier_list_opt(self);

                if (!(result->head = csema_new_pointer(self->sema, quals, result->head)))
                        return false;
                if (!result->tail)
                        result->tail = result->head;
        }
        return true;
}

static cparam* cparse_param_declaration(cparser* self)
{
        cparam* p = csema_new_param(self->sema);
        if (!cparse_decl_specs(self, &p->specs))
                return NULL;

        if (ctoken_starts_declarator(cparser_get_token(self)))
                if (!cparse_declarator(self, &p->declarator))
                        return NULL;

        return csema_finish_param(self->sema, p);
}

static bool cparse_parameter_type_list_opt(cparser* self, cdeclarator* result)
{
        if (cparser_at(self, CTK_RBRACKET))
        {
                cdeclarator_set_initialized(result);
                return true;
        }

        while (1)
        {
                if (cparser_at(self, CTK_ELLIPSIS))
                {
                        tree_location loc = cparser_get_loc(self);
                        if (!csema_set_declarator_has_vararg(self->sema, result, loc))
                                return false;
                        
                        cparser_consume_token(self);
                        cdeclarator_set_initialized(result);
                        return true;
                }

                cparam* p = cparse_param_declaration(self);
                if (!p)
                        return false;

                csema_add_declarator_param(self->sema, result, p);
                if (cparser_at(self, CTK_RBRACKET))
                {
                        cdeclarator_set_initialized(result);
                        return true;
                }
                else if (!cparser_require_ex(self, CTK_COMMA, ctk_rbracket_or_comma))
                        return false;
        }
        S_UNREACHABLE();
        return false;
}

static bool cparse_direct_declarator_suffix_opt(cparser* self, cdeclarator* result)
{
        if (cparser_at(self, CTK_LBRACKET))
        {
                cparser_consume_token(self);
                if (!csema_add_direct_declarator_function_suffix(self->sema, result))
                        return false;
                if (!cparse_parameter_type_list_opt(self, result))
                        return false;
                if (!cparser_require(self, CTK_RBRACKET))
                        return false;
        }
        else if (cparser_at(self, CTK_LSBRACKET))
        {
                cparser_consume_token(self);
                tree_expr* size = NULL;
                if (!cparser_at(self, CTK_RSBRACKET))
                        if (!(size = cparse_const_expr(self)))
                                return false;

                if (!csema_add_direct_declarator_array_suffix(
                        self->sema, result, TTQ_UNQUALIFIED, size))
                {
                        return false;
                }
                if (!cparser_require(self, CTK_RSBRACKET))
                        return false;
        }
        else
                return true;

        return cparse_direct_declarator_suffix_opt(self, result);
}

static bool cparse_direct_declarator(cparser* self, cdeclarator* result)
{
        if (cparser_at(self, CTK_ID))
        {
                tree_id id = ctoken_get_string(cparser_get_token(self));
                tree_location id_loc = cparser_get_loc(self);
                cdeclarator_set_name(result, id_loc, id);
                cparser_consume_token(self);
        }
        else if (cparser_require(self, CTK_LBRACKET))
        {
                if (!cparse_declarator(self, result))
                        return false;
                if (!cparser_require(self, CTK_RBRACKET))
                        return false;
                if (!csema_add_direct_declarator_parens(self->sema, result))
                        return false;
        }
        else
                return false;

        return cparse_direct_declarator_suffix_opt(self, result);
}

extern bool cparse_declarator(cparser* self, cdeclarator* result)
{
        cdeclarator_set_loc_begin(result, cparser_get_loc(self));

        ctype_chain pointer;
        ctype_chain_init(&pointer);
        if (!cparse_pointer_opt(self, &pointer))
                return false;

        if (cparser_at(self, CTK_LBRACKET) || cparser_at(self, CTK_ID))
                if (!cparse_direct_declarator(self, result))
                        return false;

        cdeclarator_set_loc_end(result, cparser_get_loc(self));
        return csema_finish_declarator(self->sema, result, &pointer);
}

extern tree_type* cparse_type_name(cparser* self)
{
        tree_type* t = cparse_specifier_qualifier_list(self);
        if (!t)
                return NULL;

        if (ctoken_starts_declarator(cparser_get_token(self)))
        {
                cdeclarator d;
                cdeclarator_init(&d, self->sema, CDK_TYPE_NAME);
                if (!cparse_declarator(self, &d))
                {
                        cdeclarator_dispose(&d);
                        return NULL;
                }

                t = csema_new_type_name(self->sema, &d, t);
                cdeclarator_dispose(&d);
        }
        return t;
}

extern tree_type* cparse_paren_type_name(cparser* self)
{
        if (!cparser_require(self, CTK_LBRACKET))
                return NULL;

        tree_type* t = cparse_type_name(self);
        if (!t)
                return NULL;

        return cparser_require(self, CTK_RBRACKET)
                ? t : NULL;
}

extern bool cparser_token_starts_type_name(const cparser* self, const ctoken* t)
{
        return ctoken_is_type_qualifier(t) || cparser_token_is_type_specifier(self, t);
}

extern bool cparser_next_token_starts_type_name(const cparser* self)
{
        return cparser_token_starts_type_name(self, cparser_get_next(self));
}

extern tree_type* cparse_typedef_name(cparser* self)
{
        ctoken* id = cparser_get_token(self);
        if (!cparser_require(self, CTK_ID))
                return NULL;

        return csema_new_typedef_name(self->sema,
                ctoken_get_loc(id), ctoken_get_string(id));
}