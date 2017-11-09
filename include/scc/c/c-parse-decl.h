#ifndef CPARSE_DECL_H
#define CPARSE_DECL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-parser.h"
#include "c-tree.h"

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
extern tree_decl* cparse_decl(cparser* self);
//extern bool cparse_decl_ex(cparser* self, tree_decl** pfirst, tree_decl** plast);

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
extern bool cparser_at_declaration(cparser* self);
extern bool cparse_decl_specs(cparser* self, cdecl_specs* result);

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
extern tree_type* cparse_type_specifier(cparser* self);

// c99 6.7.3 type-qualifier:
//      const
//      restrict
//      volatile
//
// type-qualifier-list:
//      type-qualifier
//      type-qualifier-list type-qualifier
extern tree_type_quals cparse_type_qualifier_list_opt(cparser* self);

// c99 6.7.2.1 specifier-qualifier-list:
//      type-specifier specifier-qualifier-list-opt
//      type-qualifier specifier-qualifier-list-opt
extern tree_type* cparse_specifier_qualifier_list(cparser* self);

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
extern tree_decl* cparse_struct_or_union_specifier(cparser* self, bool* referenced);

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
extern tree_decl* cparse_enum_specifier(cparser* self, bool* referenced);

// c99 6.7.5 declarator:
//      pointer-opt direct-declarator
//
// we diverge from c99 grammar when parsing direct-declarator.
// direct-declarator:
//      identifier
//      ( declarator )
//      direct-declarator ( parameter-type-list-opt )
//      direct-declarator [ const-expression-opt ]
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
extern bool cparse_declarator(cparser* self, cdeclarator* result);

// we diverge from c99 grammar when parsing type-name.
// c99 6.7.6 type-name:
//      specifier-qualifier-list declarator
extern tree_type* cparse_type_name(cparser* self);
extern tree_type* cparse_paren_type_name(cparser* self);
extern bool cparser_token_starts_type_name(const cparser* self, const ctoken* t);
extern bool cparser_next_token_starts_type_name(const cparser* self);

// c99 6.7.7 typedef-name:
//      identifier
extern tree_type* cparse_typedef_name(cparser* self);

#ifdef __cplusplus
}
#endif

#endif // !CPARSE_DECL_H