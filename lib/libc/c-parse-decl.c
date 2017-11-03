#include "c-parse-decl.h"
#include "c-info.h"
#include "c-prog-decl.h"
#include "c-prog-type.h"
#include "c-parse-stmt.h"
#include "c-parse-exp.h" // cparse_const_expr
#include "c-reswords.h"

static tree_decl* cparse_function_or_init_declarator(
        cparser* self, cdecl_specs* specs, bool* is_function_with_body)
{
        cdeclarator d;
        cprog_init_declarator(self->prog, &d, CDK_UNKNOWN);
        if (!cparse_declarator(self, &d))
        {
                cdeclarator_dispose(&d);
                return NULL;
        }

        tree_decl* decl = cprog_build_external_decl(self->prog, specs, &d);
        cdeclarator_dispose(&d);

        if (!decl)
                return NULL;
        if (!cprog_finish_decl(self->prog, decl))
                return NULL;

        if (cparser_at(self, CTK_EQ))
        {
                cparser_consume_token(self);
                tree_expr* init = cparse_initializer(self, decl);
                if (!init)
                        return NULL;
                if (!cprog_set_var_initializer(self->prog, decl, init))
                        return NULL;
        }
        else if (cparser_at(self, CTK_LBRACE))
        {
                cprog_enter_function(self->prog, decl);
                tree_stmt* body = cparse_block_stmt(self, CSC_NONE);
                cprog_exit_function(self->prog);

                if (!body)
                        return NULL;
                if (!cprog_set_function_body(self->prog, decl, body))
                        return NULL;

                *is_function_with_body = true;
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
        tree_decl* list = NULL;
        while (1)
        {
                bool is_function_with_body = false;
                tree_decl* d = cparse_function_or_init_declarator(self, specs, &is_function_with_body);
                if (!d)
                        return NULL;

                if (!(list = cprog_add_init_declarator(self->prog, list, d)))
                        return NULL;

                if (is_function_with_body)
                        return list;
                else if (cparser_at(self, CTK_SEMICOLON))
                {
                        cparser_consume_token(self);
                        return list;
                }
                else if (cparser_at(self, CTK_COMMA))
                        cparser_consume_token(self);
                else
                {
                        cparser_require_ex(self, CTK_UNKNOWN,
                                tree_decl_is_unnamed(d)
                                        ? ctk_lbracket_or_id
                                        : ctk_comma_eq_semicolon_lbracket);
                        return NULL;
                }
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

        return cprog_handle_unused_decl_specs(self->prog, &specs);
}

static inline bool cparser_token_is_type_specifier(const cparser* self, const ctoken* t)
{
        if (!ctoken_is_type_specifier(t))
                return false;

        if (ctoken_is(t, CTK_ID))
                return cprog_typedef_name_exists(self->prog, ctoken_get_string(t));

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
        cdecl_specs_set_start_loc(result, cparser_get_loc(self));
        tree_type_quals quals = TTQ_UNQUALIFIED;
        while (1)
        {
                const ctoken* t = cparser_get_token(self);
                if (cparser_at_type_specifier(self))
                {
                        tree_type* typespec = cparse_type_specifier(self);
                        if (!typespec)
                                return false;
                        if (!cprog_set_typespec(self->prog, result, typespec))
                                return false;
                }
                else if (ctoken_is_type_qualifier(t))
                        quals |= cparse_type_qualifier_list_opt(self);
                else if (ctoken_is_decl_storage_class(t))
                {
                        tree_decl_storage_class c = ctoken_to_decl_storage_class(t);
                        if (!cprog_set_decl_storage_class(self->prog, result, c))
                                return false;
                        cparser_consume_token(self);
                }
                else if (cparser_at(self, CTK_TYPEDEF))
                {
                        if (!cprog_set_typedef_specifier(self->prog, result))
                                return false;
                        cparser_consume_token(self);
                }
                else if (cparser_at(self, CTK_INLINE))
                {
                        if (!cprog_set_inline_specifier(self->prog, result))
                                return false;
                        cparser_consume_token(self);
                }
                else
                        break;
        }

        if (!result->typespec)
        {
                if (cparser_at(self, CTK_ID))
                {
                        ctoken* id_tok = cparser_get_token(self);
                        tree_id id = ctoken_get_string(id_tok);
                        cerror(self->error_manager, CES_ERROR, ctoken_get_loc(id_tok),
                                "unknown type name '%s'", cprog_get_id(self->prog, id));
                        return false;
                }

                cerror(self->error_manager, CES_ERROR,
                        cdecl_specs_get_start_loc(result), "expected type specifier");
                return false;
        }

        result->typespec = cprog_set_type_quals(self->prog, result->typespec, quals);
        cdecl_specs_set_end_loc(result, cparser_get_loc(self));
        return true;
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

                return cprog_build_decl_type(self->prog, specifier, referenced);
        }
        else if (k == CTK_ID)
                return cparse_typedef_name(self);

        tree_location begin = cparser_get_loc(self);
        cbuiltin_type_info info;
        cbuiltin_type_info_init(&info);
        while (1)
        {
                k = ctoken_get_kind(cparser_get_token(self));
                bool res = true;
                if (k == CTK_VOID)
                        res = cbuiltin_type_info_set_base(&info, TBTK_VOID);
                else if (k == CTK_CHAR)
                        res = cbuiltin_type_info_set_base(&info, TBTK_INT8);
                else if (k == CTK_INT)
                        res = cbuiltin_type_info_set_base(&info, TBTK_INT32);
                else if (k == CTK_FLOAT)
                        res = cbuiltin_type_info_set_base(&info, TBTK_FLOAT);
                else if (k == CTK_DOUBLE)
                        res = cbuiltin_type_info_set_base(&info, TBTK_DOUBLE);
                else if (k == CTK_SIGNED)
                        res = cbuiltin_type_info_set_signed(&info);
                else if (k == CTK_UNSIGNED)
                        res = cbuiltin_type_info_set_unsigned(&info);
                else if (k == CTK_SHORT)
                        res = cbuiltin_type_info_set_short(&info);
                else if (k == CTK_LONG)
                        res = cbuiltin_type_info_set_long(&info);
                else
                        break;

                if (!res)
                {
                        cerror(self->error_manager, CES_ERROR, begin,
                                "invalid combination of type specifiers");
                        return NULL;
                }
                cparser_consume_token(self);
        }

        tree_builtin_type_kind btk = cbuiltin_type_info_get_type(&info);
        if (btk == TBTK_INVALID)
        {
                cerror(self->error_manager, CES_ERROR, cparser_get_loc(self),
                        "expected type specifier");
                return NULL;
        }
        return cprog_build_builtin_type(self->prog, TTQ_UNQUALIFIED, btk);
}

extern tree_type_quals cparse_type_qualifier_list_opt(cparser* self)
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

        quals |= cparse_type_qualifier_list_opt(self);
        return cprog_set_type_quals(self->prog, typespec, quals);
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

        while (1)
        {
                cdeclarator sd;
                cprog_init_declarator(self->prog, &sd, CDK_MEMBER);

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

                tree_decl* m = cprog_build_member_decl(self->prog, &ds, &sd, bits);
                cdeclarator_dispose(&sd);

                if (!m || !cprog_finish_decl(self->prog, m))
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
                cerror(self->error_manager, CES_ERROR, cparser_get_loc(self),
                        "empty struct/union is invalid in C99");
                return false;
        }

        bool res = true;
        cprog_enter_decl_scope(self->prog, tree_get_record_scope(record));
        while (!cparser_at(self, CTK_RBRACE))
                if (!cparse_struct_declaration(self))
                {
                        res = false;
                        break;
                }
        cprog_exit_decl_scope(self->prog);
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

        tree_id name = tree_get_empty_id();
        if (cparser_at(self, CTK_ID))
        {
                name = ctoken_get_string(cparser_get_token(self));
                cparser_consume_token(self);
        }

        bool has_body = cparser_at(self, CTK_LBRACE);
        tree_decl* record = cprog_build_record_decl(self->prog,
                kw_loc, name, is_union, has_body);
        if (!record)
                return NULL;

        if (!has_body)
        {
                if (referenced)
                        *referenced = true;
                return record;
        }

        cparser_consume_token(self);
        if (!cparse_struct_declaration_list(self, record))
                return NULL;
        tree_location rbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACE))
                return NULL;

        return cprog_finish_decl_ex(self->prog, rbrace_loc, record);
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

        tree_decl* e = cprog_build_enumerator(self->prog, enum_, id, id_loc, value);
        return e
                ? cprog_finish_decl(self->prog, e)
                : NULL;
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
                cerror(self->error_manager, CES_ERROR, cparser_get_loc(self),
                        "empty enum is invalid in C99");
                return false;
        }
        bool res = false;
        cprog_enter_decl_scope(self->prog, tree_get_enum_scope(enum_));
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
        cprog_exit_decl_scope(self->prog);
        return res;
}

extern tree_decl* cparse_enum_specifier(cparser* self, bool* referenced)
{
        tree_location kw_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_ENUM))
                return NULL;

        tree_id name = tree_get_empty_id();
        if (cparser_at(self, CTK_ID))
        {
                name = ctoken_get_string(cparser_get_token(self));
                cparser_consume_token(self);
        }

        bool has_body = cparser_at(self, CTK_LBRACE);
        tree_decl* enum_ = cprog_build_enum_decl(self->prog, kw_loc, name, has_body);
        if (!enum_)
                return NULL;

        if (!has_body)
        {
                if (referenced)
                        *referenced = true;
                return enum_;
        }

        cparser_consume_token(self);
        if (!cparse_enumerator_list(self, enum_))
                return NULL;

        tree_location rbrace_loc = cparser_get_loc(self);
        if (!cparser_require(self, CTK_RBRACE))
                return NULL;

        return cprog_finish_decl_ex(self->prog, rbrace_loc, enum_);
}

static bool cparse_pointer_opt(cparser* self, ctype_chain* result)
{
        while (cparser_at(self, CTK_STAR))
        {
                cparser_consume_token(self);
                tree_type_quals quals = cparse_type_qualifier_list_opt(self);

                if (!(result->head = cprog_build_pointer(self->prog, quals, result->head)))
                        return false;
                if (!result->tail)
                        result->tail = result->head;
        }
        return true;
}

static cparam* cparse_param_declaration(cparser* self)
{
        cparam* p = cprog_build_param(self->prog);
        if (!cparse_decl_specs(self, &p->specs))
                return NULL;

        if (ctoken_starts_declarator(cparser_get_token(self)))
                if (!cparse_declarator(self, &p->declarator))
                        return NULL;

        return cprog_finish_param(self->prog, p);
}

static bool cparse_parameter_type_list_opt(cparser* self, cdeclarator* result)
{
        if (cparser_at(self, CTK_RBRACKET))
        {
                cdeclarator_set_initialized(result);
                return true;
        }

        cparam* p;
        while ((p = cparse_param_declaration(self)))
        {
                if (!cprog_add_declarator_param(self->prog, result, p))
                        return false;

                if (cparser_at(self, CTK_RBRACKET))
                {
                        cdeclarator_set_initialized(result);
                        return true;
                }
                else if (!cparser_require_ex(self, CTK_COMMA, ctk_rbracket_or_comma))
                        return false;
        }
        return false;
}

static bool cparse_direct_declarator_suffix_opt(cparser* self, cdeclarator* result)
{
        if (cparser_at(self, CTK_LBRACKET))
        {
                cparser_consume_token(self);
                if (!cprog_build_direct_declarator_function_suffix(self->prog, result))
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

                if (!cprog_build_direct_declarator_array_suffix(
                        self->prog, result, TTQ_UNQUALIFIED, size))
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
                if (!cprog_set_declarator_name(self->prog, id_loc, result, id))
                        return false;

                cparser_consume_token(self);
        }
        else if (cparser_require(self, CTK_LBRACKET))
        {
                if (!cparse_declarator(self, result))
                        return false;
                if (!cparser_require(self, CTK_RBRACKET))
                        return false;
                if (!cprog_add_direct_declarator_parens(self->prog, result))
                        return false;
        }
        else
                return false;

        return cparse_direct_declarator_suffix_opt(self, result);
}

extern bool cparse_declarator(cparser* self, cdeclarator* result)
{
        cdeclarator_set_start_loc(result, cparser_get_loc(self));

        ctype_chain pointer;
        ctype_chain_init(&pointer);
        if (!cparse_pointer_opt(self, &pointer))
                return false;

        if (cparser_at(self, CTK_LBRACKET) || cparser_at(self, CTK_ID))
                if (!cparse_direct_declarator(self, result))
                        return false;

        cdeclarator_set_end_loc(result, cparser_get_loc(self));
        return cprog_finish_declarator(self->prog, result, &pointer);
}

extern tree_type* cparse_type_name(cparser* self)
{
        tree_type* t = cparse_specifier_qualifier_list(self);
        if (!t)
                return NULL;

        if (ctoken_starts_declarator(cparser_get_token(self)))
        {
                cdeclarator d;
                cprog_init_declarator(self->prog, &d, CDK_TYPE_NAME);
                if (!cparse_declarator(self, &d))
                {
                        cdeclarator_dispose(&d);
                        return NULL;
                }

                t = cprog_build_type_name(self->prog, &d, t);
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

        return cprog_build_typedef_name(self->prog,
                ctoken_get_loc(id), ctoken_get_string(id));
}