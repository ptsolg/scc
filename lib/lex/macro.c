#include "macro.h"
#include "scc/c-common/context.h"
#include "scc/core/hashmap.h"

#define VEC u32vec
#define VEC_T unsigned
#include "scc/core/vec.inc"

extern c_macro* c_macro_new(c_context* context,
        bool builtin, bool function_like, tree_location loc, tree_id name)
{
        c_macro* m = c_context_allocate_node(context, sizeof(c_macro));
        m->builtin = builtin;
        m->function_like = function_like;
        m->used = false;
        m->loc = loc;
        m->name = name;
        m->params = u32vec_new();
        vec_init(&m->tokens);
        return m;
}

extern void c_macro_delete(c_context* context, c_macro* macro)
{
        // todo
}

extern void c_macro_add_param(c_macro* self, c_context* context, tree_id param)
{
        assert(self->function_like);
        u32vec_push(self->params, param);
}

extern void c_macro_add_token(c_macro* self, c_context* context, c_token* token)
{
        vec_push(&self->tokens, token);
}

extern c_token* c_macro_get_token(const c_macro* self, size_t i)
{
        return c_macro_get_tokens_begin(self)[i];
}

extern c_token** c_macro_get_tokens_begin(const c_macro* self)
{
        return (c_token**)self->tokens.items;
}

extern c_token** c_macro_get_tokens_end(const c_macro* self)
{
        return (c_token**)self->tokens.items + self->tokens.size;
}

extern size_t c_macro_get_tokens_size(const c_macro* self)
{
        return c_macro_get_tokens_end(self) - c_macro_get_tokens_begin(self);
}

extern tree_id c_macro_get_param(const c_macro* self, size_t i)
{
        return u32vec_get(self->params, i);
}

extern tree_id* c_macro_get_params_begin(const c_macro* self)
{
        return u32vec_begin(self->params);
}

extern tree_id* c_macro_get_params_end(const c_macro* self)
{
        return u32vec_end(self->params);
}

extern size_t c_macro_get_params_size(const c_macro* self)
{
        return self->params->size;
}

extern void c_macro_args_init(c_macro_args* self, c_context* context)
{
        self->context = context;
        hashmap_init(&self->args);
}

extern void c_macro_args_dispose(c_macro_args* self)
{
        for (struct hashmap_iter it = hashmap_begin(&self->args);
                it.pos != it.end; hashmap_next(&it))
        {
                vec_del(it.pos->value);
        }
        hashmap_drop(&self->args);
}

extern void c_macro_args_add(c_macro_args* self, tree_id arg, c_token* token)
{
        struct vec* tokens;
        struct hashmap_entry* entry = hashmap_lookup(&self->args, arg);
        if (!entry)
        {
                tokens = vec_new();
                hashmap_update(&self->args, arg, tokens);
        }
        else
                tokens = entry->value;

        vec_push(tokens, token);
}

extern void c_macro_args_set_empty(c_macro_args* self, tree_id arg)
{
        struct hashmap_entry* entry = hashmap_lookup(&self->args, arg);
        assert(!entry);
        struct vec* tokens = vec_new();
        hashmap_update(&self->args, arg, tokens);
}

extern struct vec* c_macro_args_get(c_macro_args* self, tree_id arg)
{
        struct hashmap_entry* entry = hashmap_lookup(&self->args, arg);
        return entry ? entry->value : NULL;
}
