#include "scc/c/c-macro.h"
#include "scc/c/c-context.h"

extern c_macro* c_macro_new(c_context* context,
        bool builtin, bool function_like, tree_location loc, tree_id name)
{
        c_macro* m = c_context_allocate_node(context, sizeof(c_macro));
        m->builtin = builtin;
        m->function_like = function_like;
        m->used = false;
        m->loc = loc;
        m->name = name;
        u32vec_init_ex(&m->params, c_context_get_allocator(context));
        ptrvec_init_ex(&m->tokens, c_context_get_allocator(context));
        return m;
}

extern void c_macro_delete(c_context* context, c_macro* macro)
{
        // todo
}

extern void c_macro_add_param(c_macro* self, c_context* context, tree_id param)
{
        assert(self->function_like);
        u32vec_push(&self->params, param);
}

extern void c_macro_add_token(c_macro* self, c_context* context, c_token* token)
{
        ptrvec_push(&self->tokens, token);
}

extern c_token* c_macro_get_token(const c_macro* self, size_t i)
{
        return c_macro_get_tokens_begin(self)[i];
}

extern c_token** c_macro_get_tokens_begin(const c_macro* self)
{
        return (c_token**)ptrvec_begin(&self->tokens);
}

extern c_token** c_macro_get_tokens_end(const c_macro* self)
{
        return (c_token**)ptrvec_end(&self->tokens);
}

extern size_t c_macro_get_tokens_size(const c_macro* self)
{
        return c_macro_get_tokens_end(self) - c_macro_get_tokens_begin(self);
}

extern tree_id c_macro_get_param(const c_macro* self, size_t i)
{
        return u32vec_get(&self->params, i);
}

extern tree_id* c_macro_get_params_begin(const c_macro* self)
{
        return u32vec_begin(&self->params);
}

extern tree_id* c_macro_get_params_end(const c_macro* self)
{
        return u32vec_end(&self->params);
}

extern size_t c_macro_get_params_size(const c_macro* self)
{
        return self->params.size;
}

extern void c_macro_args_init(c_macro_args* self, c_context* context)
{
        self->context = context;
        strmap_init_ex(&self->args, c_context_get_allocator(context));
}

extern void c_macro_args_dispose(c_macro_args* self)
{
        STRMAP_FOREACH(&self->args, it)
        {
                ptrvec* args = it->value;
                ptrvec_dispose(args);
                c_context_deallocate(self->context, args);
        }
        strmap_dispose(&self->args);
}

extern void c_macro_args_add(c_macro_args* self, tree_id arg, c_token* token)
{
        ptrvec* tokens;
        strmap_entry* entry = strmap_lookup(&self->args, arg);
        if (!entry)
        {
                tokens = c_context_allocate(self->context, sizeof(ptrvec));
                ptrvec_init_ex(tokens, c_context_get_allocator(self->context));
                strmap_update(&self->args, arg, tokens);
        }
        else
                tokens = entry->value;

        ptrvec_push(tokens, token);
}

extern void c_macro_args_set_empty(c_macro_args* self, tree_id arg)
{
        strmap_entry* entry = strmap_lookup(&self->args, arg);
        assert(!entry);
        ptrvec* tokens = c_context_allocate(self->context, sizeof(ptrvec));
        ptrvec_init_ex(tokens, c_context_get_allocator(self->context));
        strmap_update(&self->args, arg, tokens);
}

extern ptrvec* c_macro_args_get(c_macro_args* self, tree_id arg)
{
        strmap_entry* entry = strmap_lookup(&self->args, arg);
        return entry ? entry->value : NULL;
}