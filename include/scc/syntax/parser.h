#ifndef C_PARSER_H
#define C_PARSER_H

#include "scc/lex/lexer.h"

typedef struct _c_sema c_sema;
typedef struct _tree_scope tree_scope;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_module tree_module;
typedef struct _tree_decl tree_decl;
typedef struct _tree_type tree_type;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_expr tree_expr;
typedef struct _c_decl_specs c_decl_specs;
typedef struct _c_declarator c_declarator;

typedef struct _c_parser
{
        // prev, current and next
        c_token* buffer[3];
        c_lexer* lexer;
        c_sema* sema;
        c_context* context;
        void* on_error;
} c_parser;

extern void c_parser_init(c_parser* self, c_context* context, c_lexer* lexer, c_sema* sema);

extern void c_parser_dispose(c_parser* self);
extern void c_parser_set_on_error(c_parser* self, void* b);
extern void c_parser_die(const c_parser* self, int code);

extern void c_parser_enter_token_stream(c_parser* self);

// tries to resolve error, never returns NULL
extern c_token* c_parser_handle_lex_error(const c_parser* self);

extern c_token* c_parser_consume_token(c_parser* self);
extern c_token* c_parser_get_token(const c_parser* self);
extern c_token* c_parser_get_next(const c_parser* self);
extern c_token* c_parser_get_prev(const c_parser* self);
extern bool c_parser_at(const c_parser* self, c_token_kind k);
extern bool c_parser_next_token_is(c_parser* self, c_token_kind k);
extern bool c_parser_require(c_parser* self, c_token_kind k);
extern bool c_parser_require_ex(c_parser* self, c_token_kind k, const c_token_kind expected[]);
extern tree_location c_parser_get_loc(const c_parser* self);

// c99 6.9 translation-unit:
//      external-declaration
//      translation-unit external-declaration
extern tree_module* c_parse_module(c_parser* self);

// c99 6.9 external-declaration:
//      function-definition
//      declaration
//
// function-definition:
//       declaration-specifiers declarator declaration-list-opt compound-statement
//
// declaration-list:
//      declaration
//      declaration-list declaration
//
// c99 6.7 declaration:
//      declaration-specifiers init-declarator-list-opt ;
//
// init-declarator-list:
//      init-declarator
//      init-declarator-list , init-declarator
//
// init-declarator:
//      declarator
//      declarator = initializer
extern tree_decl* c_parse_decl(c_parser* self);

//c99 6.7 declaration-specifiers:
//      storage_class_specifier declaration-specifiers-opt
//      type-specifier declaration-specifiers-opt
//      type-qualifier declaration-specifiers-opt
//      function-specifier declaration-specifiers-opt
//
// function-specifier:
//      inline
//
// storage-class-specifier:
//      typedef
//      extern
//      static
//      auto
//      register
//      _Thread_local
//      _Dllimport
extern bool c_parser_at_declaration(c_parser* self);
extern bool c_parse_decl_specs(c_parser* self, c_decl_specs* result);

// c99 6.7.2 type-specifier:
//      void
//      char
//      short
//      int
//      long
//      float
//      double
//      signed
//      unsigned
//      _Bool
//      _Complex
//      struct-or-union specifier
//      enum-specifier
//      typedef-name
extern tree_type* c_parse_type_specifier(c_parser* self);

// c99 6.7.3 type-qualifier:
//      const
//      restrict
//      volatile
//
// type-qualifier-list:
//      type-qualifier
//      type-qualifier-list type-qualifier
extern int c_parse_type_qualifier_list_opt(c_parser* self);

// c99 6.7.2.1 specifier-qualifier-list:
//      type-specifier specifier-qualifier-list-opt
//      type-qualifier specifier-qualifier-list-opt
extern tree_type* c_parse_specifier_qualifier_list(c_parser* self);

// c99 6.7.2.1 struct-or-union specifier
//      struct-or-union alignment-specifier-opt identifier-opt { struct-declaration-list }
//      struct-or-union alignment-specifier-opt identifier
//
// struct-or-union:
//      struct
//      union
//
// alignment-specifier:
//      _Aligned ( constant-expression )
//
// struct-declaration-list:
//      struct-declaration
//      struct-declaration-list struct-declaration
//
// struct-declaration:
//      specifier-qualifier-list struct-declarator-list ;
//
// struct-declarator-list:
//      struct-declarator
//      struct-declarator-list , struct-declarator
//
// struct-declarator:
//      declarator
//      declarator : constant-expression
extern tree_decl* c_parse_struct_or_union_specifier(c_parser* self, bool* referenced);

// c99 6.7.2.2 enum-specifier:
//      enum identifier-opt { enumerator-list }
//      enum identifier-opt { enumerator-list , }
//      enum identifier
//
// enumerator_list:
//      enumerator
//      enumerator_list , enumerator
//
// enumerator:
//      enumeration-constant
//      enumeration-constant = constant-expression
extern tree_decl* c_parse_enum_specifier(c_parser* self, bool* referenced);

// c99 6.7.5 declarator:
//      pointer-opt direct-declarator
//
// we diverge from c99 grammar when parsing direct-declarator.
// direct-declarator:
//      identifier
//      ( declarator )
//      direct-declarator ( parameter-type-list-opt ) attribute-list-opt
//      direct-declarator [ const-expression-opt ]
//
// attribute-list:
//      attribute-list
//      _Transaction_safe
//      _Stdcall
//
// pointer:
//      * type-qualifier-list-opt
//      * type-qualifier-list-opt pointer
//
// parameter-list:
//      parameter-declaration
//      parameter-list , parameter-declaration
//
// parameter-declaration:
//      declaration-specifiers declarator-opt
extern bool c_parse_declarator(c_parser* self, c_declarator* result);

// we diverge from c99 grammar when parsing type-name.
// c99 6.7.6 type-name:
//      specifier-qualifier-list declarator
extern tree_type* c_parse_type_name(c_parser* self);
extern tree_type* c_parse_paren_type_name(c_parser* self);
extern bool c_parser_token_starts_type_name(const c_parser* self, const c_token* t);
extern bool c_parser_next_token_starts_type_name(const c_parser* self);

// c99 6.7.7 typedef-name:
//      identifier
extern tree_type* c_parse_typedef_name(c_parser* self);

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

extern const c_token_kind ctk_rbracket_or_comma[];

// c99 6.5.1 primary-expression:
//      identifier
//      constant
//      string-literal
//      paren-expression
//
// paren-expression:
//      ( expression )
extern tree_expr* c_parse_paren_expr(c_parser* self);
extern tree_expr* c_parse_primary_expr(c_parser* self);

// c99 6.5.2 postfix-expression:
//       primary-expression
//       postfix-expression [ expression ]
//       postfix-expression ( argument-expression-list-opt )
//       postfix-expression . identifier
//       postfix-expression -> identifier
//       postfix-expression ++
//       postfix-expression --
//      
// argument-expression-list:
//       assignment-expression
//       argument-expression-list , assignment-expression
extern tree_expr* c_parse_postfix_expr(c_parser* self);

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
extern tree_expr* c_parse_unary_expr(c_parser* self);

// c99 6.5.4 cast-expression:
//      unary-expression
//      ( type-name ) cast-expression
extern tree_expr* c_parse_cast_expr(c_parser* self);

// c99 6.5.5 multiplicative-expression:
//       cast-expression
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
//      and-expression
//      xor-expression ^ and-expression
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
//      logical-or-expression
//      logical-or-expression ? expression : conditional-expression
//
// c99 6.5.16 assignment-expression:
//      conditional-expression
//      unary-expression assignment-operator assignment-expression
//
// asignment-operator: one of
//      = *= /= %= += -= <<= >>= &= ^= |=
extern tree_expr* c_parse_assignment_expr(c_parser* self);

enum
{
        PRECEDENCE_UNKNOWN,
        PRECEDENCE_COMMA,
        PRECEDENCE_ASSIGN,
        PRECEDENCE_CONDITIONAL,
        PRECEDENCE_LOG_OR,
        PRECEDENCE_LOG_AND,
        PRECEDENCE_OR,
        PRECEDENCE_XOR,
        PRECEDENCE_AND,
        PRECEDENCE_EQUALITY,
        PRECEDENCE_RELATIONAL,
        PRECEDENCE_SHIFT,
        PRECEDENCE_ADDITIVE,
        PRECEDENCE_MULTIPLICATIVE,
};

// c99 6.5.17 expression:
//      expression , assignment-expression
//      assignment-expression
extern tree_expr* c_parse_expr(c_parser* self);
extern tree_expr* c_parse_expr_ex(c_parser* self, int min_prec);

// c99 6.6 constant-expression:
//       conditional-expression
extern tree_expr* c_parse_const_expr(c_parser* self);

// c99 6.7.8 initializer:
//      assignment-expression
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
extern tree_expr* c_parse_initializer(c_parser* self);

extern tree_module* c_parse_source(
        c_context* context,
        file_entry* source,
        c_pragma_handlers handlers,
        FILE* error);

#endif
