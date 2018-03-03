#ifndef C_MACRO_H
#define C_MACRO_H

#include "scc/scl/dseq-common.h"
#include "scc/tree/tree-common.h"

typedef struct _dseq dseq;
typedef struct _dseq dseq_u32;
typedef struct _c_context c_context;
typedef struct _c_token c_token;

typedef struct _c_macro
{
        bool builtin;
        bool function_like;
        bool used;
        dseq tokens;
        dseq_u32 params;
        tree_location loc;
        tree_id name;
} c_macro;

extern c_macro* c_macro_new(c_context* context,
        bool builtin, bool function_like, tree_location loc, tree_id name);
extern void c_macro_delete(c_context* context, c_macro* macro);
extern void c_macro_add_param(c_macro* self, c_context* context, tree_id param);
extern void c_macro_add_token(c_macro* self, c_context* context, c_token* token);
extern c_token* c_macro_get_token(const c_macro* self, ssize i);
extern c_token** c_macro_get_tokens_begin(const c_macro* self);
extern c_token** c_macro_get_tokens_end(const c_macro* self);
extern ssize c_macro_get_tokens_size(const c_macro* self);
extern tree_id c_macro_get_param(const c_macro* self, ssize i);
extern tree_id* c_macro_get_params_begin(const c_macro* self);
extern tree_id* c_macro_get_params_end(const c_macro* self);
extern ssize c_macro_get_params_size(const c_macro* self);

#define C_FOREACH_MACRO_TOKEN(PMACRO, ITNAME, ENDNAME) \
        for (c_token** ITNAME = c_macro_get_tokens_begin(PMACRO), \
                **ENDNAME = c_macro_get_tokens_end(PMACRO); \
                ITNAME != ENDNAME; ITNAME++)

#define C_FOREACH_MACRO_TOKEN_REVERSED(PMACRO, ITNAME, ENDNAME) \
        for (c_token** ITNAME = c_macro_get_tokens_end(PMACRO) - 1, \
                **ENDNAME = c_macro_get_tokens_begin(PMACRO) - 1; \
                ITNAME != ENDNAME; ITNAME--)

typedef struct _c_macro_args
{
        strmap args;
        c_context* context;
} c_macro_args;

extern void c_macro_args_init(c_macro_args* self, c_context* context);
extern void c_macro_args_dispose(c_macro_args* self);
extern void c_macro_args_add(c_macro_args* self, tree_id arg, c_token* token);
extern void c_macro_args_set_empty(c_macro_args* self, tree_id arg);
extern dseq* c_macro_args_get(c_macro_args* self, tree_id arg);

#endif