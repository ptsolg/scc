#ifndef C_PRINTER_H
#define C_PRINTER_H

#include <stdio.h>
#include "scc/core/read-write.h"
#include "scc/core/vec.h"

typedef struct _c_token c_token;
typedef struct _c_context c_context;
typedef struct _c_source_manager c_source_manager;
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
} c_printer_opts;

extern void c_printer_opts_init(c_printer_opts* self);

typedef struct _c_printer
{
        writebuf buf;
        tree_context* context;
        const c_context* ccontext;
        // used when we need to print constant expression result
        const tree_target_info* target;
        const c_source_manager* source_manager;
        c_printer_opts opts;
        int indent_level;
} c_printer;

extern void c_printer_init(c_printer* self, write_cb* write, const c_context* context);
extern void c_printer_dispose(c_printer* self);

typedef struct
{
        int max_line;
        int max_col;
        int max_kind_len;
        int max_file_len;
} c_token_print_info;

extern void c_token_print_info_init(c_token_print_info* self);

extern void c_print_token(c_printer* self, const c_token* token, const c_token_print_info* info);
extern void c_print_tokens(c_printer* self, const struct vec* tokens);

extern void c_print_expr(c_printer* self, const tree_expr* expr);

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

extern void c_print_type_name(c_printer* self, const tree_type* type, int opts);
extern void c_print_decl(c_printer* self, const tree_decl* d, int opts);
extern void c_print_stmt(c_printer* self, const tree_stmt* s);
extern void c_print_module(c_printer* self, const tree_module* module);

#endif
