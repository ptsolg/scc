#ifndef CPARSE_STMT_H
#define CPARSE_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"
#include "c-tree.h"

typedef struct _tree_stmt tree_stmt;
typedef enum _tree_stmt_kind tree_stmt_kind;
typedef enum _cstmt_context cstmt_context;
// c99 6.8 statement:
//      labeled-statement
//      compound-statement
//      expression-statement
//      selection-statement
//      iteration-statement
//      jump-statement
extern tree_stmt* cparse_stmt(cparser* self);

// c99 6.8.1 labeled-statement:
//      identifier : statement
//      case constant-expression : statement
//      default : statement
extern tree_stmt* cparse_case_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_default_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_labeled_stmt(cparser* self, cstmt_context context);

// c99 6.8.2 compound-statement:
//      { block-item-list-opt }
//
// block-statement-list:
//      block-item
//      block-item-list block-item
//
// block-item:
//      declaration
//      statement
extern tree_stmt* cparse_block_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_decl_stmt(cparser* self);

// c99 6.8.3 expression-statement:
//      expression-opt ;
extern tree_stmt* cparse_expr_stmt(cparser* self);

// c99 6.8.4 selection-statement:
//      if ( expression ) statement
//      if ( expression ) statement else statement
//      switch ( expression ) statement
extern tree_stmt* cparse_if_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_switch_stmt(cparser* self, cstmt_context context);

// c99 6.8.5 iteration-statement:
//      while ( expression ) statement
//      do statement while ( expression ) ;
//      for ( expression-opt ; expression-opt ; expression-opt ) statement
//      for ( declaration expression-opt ; expression-opt ) statement
extern tree_stmt* cparse_while_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_do_while_stmt(cparser* self, cstmt_context context);
extern tree_stmt* cparse_for_stmt(cparser* self, cstmt_context context);

// c99 6.8.6 jump-statement:
//      goto identifier ;
//      continue ;
//      break ;
//      return expression-opt ;
extern tree_stmt* cparse_goto_stmt(cparser* self);
extern tree_stmt* cparse_continue_stmt(cparser* self);
extern tree_stmt* cparse_break_stmt(cparser* self);
extern tree_stmt* cparse_return_stmt(cparser* self);

#ifdef __cplusplus
}
#endif

#endif // !CPARSE_STMT_H
