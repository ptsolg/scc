#ifndef C_PARSE_STMT_H
#define C_PARSE_STMT_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"
#include "c-context.h"

typedef struct _tree_stmt tree_stmt;

// c99 6.8 statement:
//      labeled-statement
//      compound-statement
//      expression-statement
//      selection-statement
//      iteration-statement
//      jump-statement
//      transaction-statement
extern tree_stmt* c_parse_stmt(c_parser* self);

// c99 6.8.1 labeled-statement:
//      identifier : statement
//      case constant-expression : statement
//      default : statement
extern tree_stmt* c_parse_case_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_default_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_labeled_stmt(c_parser* self, int scope_flags);

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
extern tree_stmt* c_parse_compound_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_decl_stmt(c_parser* self);

// c99 6.8.3 expression-statement:
//      expression-opt ;
extern tree_stmt* c_parse_expr_stmt(c_parser* self);

// c99 6.8.4 selection-statement:
//      if ( expression ) statement
//      if ( expression ) statement else statement
//      switch ( expression ) statement
extern tree_stmt* c_parse_if_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_switch_stmt(c_parser* self, int scope_flags);

// c99 6.8.5 iteration-statement:
//      while ( expression ) statement
//      do statement while ( expression ) ;
//      for ( expression-opt ; expression-opt ; expression-opt ) statement
//      for ( declaration expression-opt ; expression-opt ) statement
extern tree_stmt* c_parse_while_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_do_while_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_for_stmt(c_parser* self, int scope_flags);

// c99 6.8.6 jump-statement:
//      goto identifier ;
//      continue ;
//      break ;
//      return expression-opt ;
extern tree_stmt* c_parse_goto_stmt(c_parser* self, int scope_flags);
extern tree_stmt* c_parse_continue_stmt(c_parser* self);
extern tree_stmt* c_parse_break_stmt(c_parser* self);
extern tree_stmt* c_parse_return_stmt(c_parser* self);

// transaction-statement:
//      _Atomic statement
extern tree_stmt* c_parse_atomic_stmt(c_parser* self, int scope_flags);

#ifdef __cplusplus
}
#endif

#endif
