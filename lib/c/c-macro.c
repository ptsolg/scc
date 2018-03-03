#include "scc/c/c-macro.h"
#include "scc/c/c-context.h"
#include "scc/core/dseq-instance.h"

extern c_macro* c_macro_new(c_context* context,
        bool builtin, bool function_like, tree_location loc, tree_id name)
{
        c_macro* m = c_context_allocate_node(context, sizeof(c_macro));
        m->builtin = builtin;
        m->function_like = function_like;
        m->used = false;
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

extern tree_id c_macro_get_param(const c_macro* self, ssize i)
{
        return dseq_u32_get(&self->params, i);
}

extern tree_id* c_macro_get_params_begin(const c_macro* self)
{
        return dseq_u32_begin(&self->params);
}

extern tree_id* c_macro_get_params_end(const c_macro* self)
{
        return dseq_u32_end(&self->params);
}

extern ssize c_macro_get_params_size(const c_macro* self)
{
        return dseq_u32_size(&self->params);
}

extern void c_macro_args_init(c_macro_args* self, c_context* context)
{
        self->context = context;
        strmap_init_alloc(&self->args, c_context_get_allocator(context));
}

extern void c_macro_args_dispose(c_macro_args* self)
{
        STRMAP_FOREACH(&self->args, it)
        {
                dseq* args = *strmap_iter_value(&it);
                dseq_dispose(args);
                c_context_deallocate(self->context, args);
        }
        strmap_dispose(&self->args);
}

extern void c_macro_args_add(c_macro_args* self, tree_id arg, c_token* token)
{
        dseq* tokens;
        strmap_iter it;
        if (!strmap_find(&self->args, arg, &it))
        {
                tokens = c_context_allocate(self->context, sizeof(dseq));
                dseq_init_alloc(tokens, c_context_get_allocator(self->context));
                strmap_insert(&self->args, arg, tokens);
        }
        else
                tokens = *strmap_iter_value(&it);

        dseq_append(tokens, token);
}

extern void c_macro_args_set_empty(c_macro_args* self, tree_id arg)
{
        strmap_iter it;
        bool already_set = strmap_find(&self->args, arg, &it);
        S_ASSERT(already_set == false);
        dseq* tokens = c_context_allocate(self->context, sizeof(dseq));
        dseq_init_alloc(tokens, c_context_get_allocator(self->context));
        strmap_insert(&self->args, arg, tokens);
}

extern dseq* c_macro_args_get(c_macro_args* self, tree_id arg)
{
        strmap_iter it;
        return strmap_find(&self->args, arg, &it) ? *strmap_iter_value(&it) : NULL;
}