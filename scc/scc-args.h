#ifndef SCC_ARGS_H
#define SCC_ARGS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libscl/arg-parser.h>

typedef struct _scc_env scc_env;

typedef struct
{
        aparser_cb base;
        scc_env* scc;
        const char* arg;
} scc_arg_handler;

extern scc_arg_handler* scc_new_arg_handler(
        allocator* alloc,
        void(*const fn)(scc_arg_handler*, aparser*),
        scc_env* scc,
        const char* arg);

extern void scc_syntax_only(scc_arg_handler* self, aparser* p);
extern void scc_lex_only(scc_arg_handler* self, aparser* p);
extern void scc_log(scc_arg_handler* self, aparser* p);
extern void scc_o(scc_arg_handler* self, aparser* p);
extern void scc_i(scc_arg_handler* self, aparser* p);
extern void scc_print_eval_result(scc_arg_handler* self, aparser* p);
extern void scc_print_expr_value(scc_arg_handler* self, aparser* p);
extern void scc_print_expr_type(scc_arg_handler* self, aparser* p);
extern void scc_print_impl_casts(scc_arg_handler* self, aparser* p);
extern void scc_force_brackets(scc_arg_handler* self, aparser* p);
extern void scc_float_precision(scc_arg_handler* self, aparser* p);
extern void scc_double_precision(scc_arg_handler* self, aparser* p);
extern void scc_x32(scc_arg_handler* self, aparser* p);
extern void scc_x64(scc_arg_handler* self, aparser* p);
extern void scc_default(scc_arg_handler* self, aparser* p);

#ifdef __cplusplus
}
#endif

#endif // !SCC_ARGS_H
