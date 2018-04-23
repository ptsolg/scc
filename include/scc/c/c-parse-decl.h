#ifndef C_PARSE_DECL_H
#define C_PARSE_DECL_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"

typedef struct _c_declarator c_declarator;
typedef struct _c_decl_specs c_decl_specs;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;

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
//      struct-or-union identifier-opt { struct-declaration-list }
//      struct-or-union identifier
//
// struct-or-union:
//      struct
//      union
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

#ifdef __cplusplus
}
#endif

#endif
