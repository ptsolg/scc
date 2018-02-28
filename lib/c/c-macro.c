#include "scc/c/c-macro.h"
#include "scc/c/c-context.h"
#include "scc/scl/dseq-instance.h"

extern c_macro* c_macro_new(c_context* context,
        bool builtin, bool function_like, tree_location loc, tree_id name)
{
        c_macro* m = c_context_allocate_node(context, sizeof(c_macro));
        m->builtin = builtin;
        m->function_like = function_like;
        m->loc = loc;
        m->name = name;
        dseq_u32_init_alloc(&m->params, c_context_get_allocator(context));
        dseq_init_alloc(&m->tokens, c_context_get_allocator(context));
        return m;
}

extern void c_macro_delete(c_context* context, c_macro* macro)
{
        // todo
}

extern void c_macro_add_param(c_macro* self, c_context* context, tree_id param)
{
        S_ASSERT(self->function_like);
        dseq_u32_append(&self->params, param);
}

extern void c_macro_add_token(c_macro* self, c_context* context, c_token* token)
{
        dseq_append(&self->tokens, token);
}

extern c_token* c_macro_get_token(const c_macro* self, ssize i)
{
        return c_macro_get_tokens_begin(self)[i];
}

extern c_token** c_macro_get_tokens_begin(const c_macro* self)
{
        return (c_token**)dseq_begin(&self->tokens);
}

extern c_token** c_macro_get_tokens_end(const c_macro* self)
{
        return (c_token**)dseq_end(&self->tokens);
}

extern ssize c_macro_get_tokens_size(const c_macro* self)
{
        return c_macro_get_tokens_end(self) - c_macro_get_tokens_begin(self);
}

extern tree_id* c_macro_get_params_begin(const c_macro* self)
{
        return dseq_u32_begin(&self->params);
}

extern tree_id* c_macro_get_params_end(const c_macro* self)
{
        return dseq_u32_end(&self->params);
}