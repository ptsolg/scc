#ifndef CPRINTER_H
#define CPRINTER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "scc/scl/read-write.h"
#include "scc/scl/objgroup.h"

typedef struct _ctoken ctoken;
typedef struct _csource_manager csource_manager;
typedef struct _cident_policy cident_policy;
typedef struct _tree_expr tree_expr;
typedef struct _tree_type tree_type;
typedef struct _tree_decl tree_decl;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_scope tree_scope;
typedef struct _tree_module tree_module;
typedef struct _tree_context tree_context;
typedef struct _tree_target_info tree_target_info;

typedef struct
{
        bool print_expr_type;
        bool print_expr_value;
        bool print_impl_casts;
        bool print_eval_result;
        bool force_brackets;
        int float_precision;
        int double_precision;
} cprinter_opts;

extern void cprinter_opts_init(cprinter_opts* self);

typedef struct _cprinter
{
        writebuf buf;
        const tree_context* context;
        // used when we need to print constant expression result
        const tree_target_info* target;
        const csource_manager* source_manager;
        const cident_policy* id_policy;
        cprinter_opts opts;
        int indent_level;
} cprinter;

extern void cprinter_init(
        cprinter* self,
        write_cb* write,
        const tree_context* context,
        const cident_policy* id_policy,
        const csource_manager* source_manager,
        const tree_target_info* target);

extern void cprinter_dispose(cprinter* self);

typedef struct
{
        int max_line;
        int max_col;
        int max_kind_len;
        int max_file_len;
} ctoken_print_info;

extern void ctoken_print_info_init(ctoken_print_info* self);

extern void cprint_token(cprinter* self, const ctoken* token, const ctoken_print_info* info);
extern void cprint_tokens(cprinter* self, const objgroup* tokens);

extern void cprint_expr(cprinter* self, const tree_expr* expr);

enum
{
        CPRINT_OPTS_NONE = 0,
        // if type is struct A { ... } it will be printed like this: struct A
        CPRINT_TYPE_REF = 1,
        // prints brackets in implicit type
        CPRINT_IMPL_TYPE_BRACKETS = 2,
        CPRINTER_IGNORE_TYPESPEC = 4,

        // ignores typedef, extern, static
        CPRINTER_IGNORE_STORAGE_SPECS = 8,
        CPRINTER_IGNORE_SPECS = CPRINTER_IGNORE_TYPESPEC | CPRINTER_IGNORE_STORAGE_SPECS,

        // only name of the decl will be printed
        CPRINT_DECL_NAME = 16,
        // if decl ends with ';' or ',' the ending will not be printed
        CPRINTER_IGNORE_DECL_ENDING = 32,
};

extern void cprint_type_name(cprinter* self, const tree_type* type, int opts);
extern void cprint_decl(cprinter* self, const tree_decl* d, int opts);
extern void cprint_stmt(cprinter* self, const tree_stmt* s);
extern void cprint_module(cprinter* self, const tree_module* module);

#ifdef __cplusplus
}
#endif

#endif // !CPRINTER_H
