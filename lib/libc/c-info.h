#ifndef CINFO_H
#define CINFO_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libtree/tree-type.h>
#include <libtree/tree-expr.h>
#include <libtree/tree-decl.h>
#include "c-token.h"

typedef struct _cresword_info cresword_info;

extern const char* cget_builtin_type_string(tree_builtin_type_kind k);
extern const char* cget_token_kind_string(ctoken_kind k);
extern const cresword_info* cget_token_info(const ctoken* t);
extern const cresword_info* cget_token_kind_info(ctoken_kind k);

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
} cprecedence_level;

// returns precedence of binary or conditional operator
extern int cget_operator_precedence(const ctoken* self);
extern int cget_binop_precedence(tree_binop_kind k);
extern const char* cget_binop_string(tree_binop_kind k);
extern const char* cget_unop_string(tree_unop_kind k);
extern const char* cget_decl_storage_class_string(tree_decl_storage_class sc);
extern void cqet_qual_string(tree_type_quals q, char* buf);

extern tree_binop_kind ctoken_to_binop(const ctoken* self);
extern tree_unop_kind ctoken_to_prefix_unary_operator(const ctoken* self);
extern bool ctoken_is_builtin_type_specifier(const ctoken* self);
extern bool ctoken_is_type_specifier(const ctoken* self);
extern tree_type_quals ctoken_to_type_qualifier(const ctoken* self);
extern bool ctoken_is_type_qualifier(const ctoken* self);
extern tree_decl_storage_class ctoken_to_decl_storage_class(const ctoken* self);
extern bool ctoken_is_decl_storage_class(const ctoken* self);
extern bool ctoken_starts_declarator(const ctoken* self);

extern int cget_type_rank(const tree_type* t);

#ifdef __cplusplus
}
#endif

#endif // !CINFO_H
