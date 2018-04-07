#ifndef C_INFO_H
#define C_INFO_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree-type.h"
#include "scc/tree/tree-expr.h"
#include "scc/tree/tree-decl.h"
#include "scc/c/c-token.h"

extern void c_get_unescaped_string(char* dst, size_t dst_size, const char* string, size_t string_size);
// returns bytes written (including trailing zero)
extern size_t c_get_escaped_string(char* dst, size_t dst_size, const char* string, size_t string_size);

typedef struct _c_resword_info c_resword_info;

extern const char* c_get_builtin_type_string(tree_builtin_type_kind k);
extern const char* c_get_token_kind_string(c_token_kind k);
extern const c_resword_info* c_get_token_info(const c_token* t);
extern const c_resword_info* c_get_token_kind_info(c_token_kind k);
extern int c_token_to_string(const tree_context* context, const c_token* tok, char* buf, size_t n);

typedef enum
{
        CSF_NONE = 0,
        CSF_BREAK = 1,
        CSF_CONTINUE = 2,
        CSF_SWITCH = 4 | CSF_BREAK,
        CSF_DECL = 8,
        CSF_ATOMIC = 16 | CSF_BREAK,
        CSF_ITERATION = CSF_BREAK | CSF_CONTINUE,
} c_scope_flags;

typedef enum
{
        CPL_UNKNOWN,

        CPL_COMMA,
        CPL_ASSIGN,
        CPL_CONDITIONAL,
        CPL_LOG_OR,
        CPL_LOG_AND,
        CPL_OR,
        CPL_XOR,
        CPL_AND,
        CPL_EQUALITY,
        CPL_RELATIONAL,
        CPL_SHIFT,
        CPL_ADDITIVE,
        CPL_MULTIPLICATIVE,
} c_precedence_level;

// returns precedence of binary or conditional operator
extern int c_get_operator_precedence(const c_token* self);
extern int c_get_binop_precedence(tree_binop_kind k);
extern const char* c_get_binop_string(tree_binop_kind k);
extern const char* c_get_unop_string(tree_unop_kind k);
extern const char* c_get_decl_storage_class_string(tree_decl_storage_class sc);
extern void c_get_qual_string(tree_type_quals q, char* buf);

extern tree_binop_kind c_token_to_binop(const c_token* self);
extern tree_unop_kind c_token_to_prefix_unary_operator(const c_token* self);
extern bool c_token_is_builtin_type_specifier(const c_token* self);
extern bool c_token_is_type_specifier(const c_token* self);
extern tree_type_quals c_token_to_type_qualifier(const c_token* self);
extern bool c_token_is_type_qualifier(const c_token* self);
extern tree_decl_storage_class c_token_to_decl_storage_class(const c_token* self);
extern bool c_token_is_decl_storage_class(const c_token* self);
extern bool c_token_starts_declarator(const c_token* self);

extern int c_get_type_rank(const tree_type* t);

#ifdef __cplusplus
}
#endif

#endif
