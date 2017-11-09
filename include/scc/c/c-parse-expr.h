#ifndef CPARSE_EXPR_H
#define CPARSE_EXPR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"

typedef struct _tree_expr tree_expr;
typedef struct _tree_decl tree_decl;

extern const ctoken_kind ctk_rbracket_or_comma[];

// c99 6.5.1 primary-expression:
//      identifier
//      constant
//      string-literal
//      paren-expression
//
// paren-expression:
//      ( expression )
extern tree_expr* cparse_paren_expr(cparser* self);
extern tree_expr* cparse_primary_expr(cparser* self);

// c99 6.5.2 postfix-expression:
//       primary-expression
//       postfix-expression [ expression ]
//       postfix-expression ( argument-expression-list-opt )
//       postfix-expression . identifier
//       postfix-expression -> identifier
//       postfix-expression ++
//       postfix-expression --
//      
//       argument-expression-list:
//       assignment-expression
//       argument-expression-list , assignment-expression
extern tree_expr* cparse_postfix_expr(cparser* self);

// c99 6.5.3 unary-expression:
//      postfix-expression
//      ++ unary-expression
//      -- unary-expression
//      unary-operator cast-expression
//      sizeof unary-expression
//      sizeof ( type-name )
//
// unary-operator: one of
//       & * + - ~ !
extern tree_expr* cparse_unary_expr(cparser* self);

// c99 6.5.4 cast-expression:
//      unary-expression
//      ( type-name ) cast-expression
extern tree_expr* cparse_cast_expr(cparser* self);

// c99 6.5.5 multiplicative-expression:
// cast-expression
//       multiplicative-expression * cast-expression
//       multiplicative-expression / cast-expression
//       multiplicative-expression % cast-expression
//
// c99 6.5.6 additive-expression:
//       multiplicative-expression
//       additive-expression + multiplicative-expression
//       additive-expression - multiplicative-expression
//
// c99 6.5.7 shift-expression
//       additive-expression
//       shift-expression << additive-expression
//       shift-expression >> additive-expression
//
// c99 6.5.8 relational-expression:
//      shift-expression
//      relational-expression < shift-expression
//      relational-expression > shift-expression
//      relational-expression <= shift-expression
//      relational-expression >= shift-expression
//
// c99 6.5.9 equality-expression:
//      relational-expression
//      equality-expression == relational-expression
//      equality-expression != relational-expression
//
// c99 6.5.10 and-expression:
//      equality-expression
//      and-expression & equality-expression
//
// c99 6.5.11 xor-expression:
//       and-expression
//       xor-expression ^ and-expression
//
// c99 6.5.12 or-expression:
//      xor-expression
//      or-expression | xor-expression
//
// c99 6.5.13 logical-and-expression:
//      or-expression
//      logical-and-expression && or-expression
//
// c99 6.5.14 logical-or-expression:
//      logical-and-expression
//      logical-or-expression || logical-and-expression
//
// c99 6.5.15 conditional-expression
//       logical-or-expression
//       logical-or-expression ? expression : conditional-expression
//
// c99 6.5.16 assignment-expression:
//      conditional-expression
//      unary-expression assignment-operator assignment-expression
//
// asignment-operator: one of
//       = *= /= %= += -= <<= >>= &= ^= |=
extern tree_expr* cparse_assignment_expr(cparser* self);

// c99 6.5.17 expression:
//      expression , assignment-expression
//      assignment-expression
extern tree_expr* cparse_expr(cparser* self);
extern tree_expr* cparse_expr_ex(cparser* self, int min_prec);

// c99 6.6 constant-expression:
//       conditional-expression
extern tree_expr* cparse_const_expr(cparser* self);

// c99 6.7.8 initializer:
// assignment-expression
//      { initializer-list
//      { initializer-list , }
//
// initializer-list:
//      designation-opt initializer
//      initializer-list , designation-opt initializer
//
// designation:
//       designator-list =
//
// designator-list:
//      designator
//      designator-list designator
//
// designator:
//      [ constant-expression ]
//      . identifier
extern tree_expr* cparse_initializer(cparser* self, tree_decl* decl);

#ifdef __cplusplus
}
#endif

#endif // !CPARSE_EXPR_H