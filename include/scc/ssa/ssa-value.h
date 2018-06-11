#ifndef SSA_VALUE_H
#define SSA_VALUE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <scc/tree/tree.h>
#include "ssa-common.h"

typedef struct _tree_type tree_type;
typedef struct _tree_expr tree_expr;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_block ssa_block;

typedef struct _ssa_value_use
{
        list_node node;
        ssa_instr* instr;
        ssa_value* value;
} ssa_value_use;

extern void _ssa_remove_value_use(ssa_value_use* self);
extern void _ssa_add_value_use(ssa_value* val, ssa_value_use* use);

extern ssa_block* ssa_get_value_use_block(const ssa_value_use* self);
extern ssa_value* ssa_get_value_use_label(const ssa_value_use* self);
extern void ssa_set_value_use_value(ssa_value_use* self, ssa_value* val);

static inline ssa_value* ssa_get_value_use_value(const ssa_value_use* self);
static inline ssa_instr* ssa_get_value_use_instr(const ssa_value_use* self);
static inline ssa_value_use* ssa_get_next_value_use(const ssa_value_use* self);

typedef enum
{
        SVK_INVALID,
        SVK_LOCAL_VAR,
        SVK_GLOBAL_VAR,
        SVK_CONSTANT,
        SVK_LABEL,
        SVK_STRING,
        SVK_FUNCTION,
        SVK_PARAM,
} ssa_value_kind;

struct _ssa_value_base
{
        ssa_value_kind kind;
        ssa_id id;
        tree_type* type;
        void* metadata;
        list_head use_list;
};

extern void ssa_init_value(ssa_value* self, ssa_value_kind kind, tree_type* type);

extern ssa_value* ssa_new_value(ssa_context* context,
        ssa_value_kind kind, tree_type* type, size_t size);

extern void ssa_replace_value_with(ssa_value* what, ssa_value* with);

static inline struct _ssa_value_base* _ssa_value_base(ssa_value* self);
static inline const struct _ssa_value_base* _ssa_value_cbase(const ssa_value* self);

static inline ssa_value_use* ssa_get_value_uses_begin(const ssa_value* self);
static inline ssa_value_use* ssa_get_value_uses_end(ssa_value* self);
static inline size_t ssa_get_value_uses_size(ssa_value* self);
static inline const ssa_value_use* ssa_get_value_uses_cend(const ssa_value* self);
static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self);
static inline ssa_id ssa_get_value_id(const ssa_value* self);
static inline tree_type* ssa_get_value_type(const ssa_value* self);
static inline bool ssa_value_is_used(const ssa_value* self);
static inline void* ssa_get_value_metadata(const ssa_value* self);
static inline void ssa_set_value_metadata(ssa_value* self, void* md);
static inline void* ssa_remove_value_metadata(ssa_value* self);

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind);
static inline void ssa_set_value_type(ssa_value* self, tree_type* type);

#define SSA_FOREACH_VALUE_USE(PVAL, ITNAME, ENDNAME) \
        for (ssa_value_use* ITNAME = ssa_get_value_uses_begin(PVAL),\
                *ENDNAME = ssa_get_value_uses_end(PVAL);\
                ITNAME != ENDNAME; ITNAME = ssa_get_next_value_use(ITNAME))

#define SSA_FOREACH_VALUE_USE_SAFE(PVAL, ITNAME, NEXTNAME, ENDNAME) \
        for (ssa_value_use* ITNAME = ssa_get_value_uses_begin(PVAL),\
                *ENDNAME = ssa_get_value_uses_end(PVAL), *NEXTNAME;\
                (NEXTNAME = ssa_get_next_value_use(ITNAME)), ITNAME != ENDNAME; ITNAME = NEXTNAME)

struct _ssa_local_var
{
        struct _ssa_value_base base;
};

extern void ssa_init_local_var(ssa_value* self, tree_type* type);

struct _ssa_global_var
{
        struct _ssa_value_base base;
        tree_decl* entity;
        // todo: ssa-initializer
};

extern ssa_value* ssa_new_global_var(ssa_context* self, tree_decl* var);

static inline struct _ssa_global_var* _ssa_global_var(ssa_value* self);
static inline const struct _ssa_global_var* _ssa_global_cvar(const ssa_value* self);

static inline tree_decl* ssa_get_global_var_entity(const ssa_value* self);
static inline void ssa_set_global_var_entity(ssa_value* self, tree_decl* var);

struct _ssa_constant
{
        struct _ssa_value_base base;
        avalue value;
};

extern ssa_value* ssa_new_constant(ssa_context* context, tree_type* type, const avalue* val);

static inline struct _ssa_constant* _ssa_constant(ssa_value* self);
static inline const struct _ssa_constant* _ssa_cconstant(const ssa_value* self);

static inline const avalue* ssa_get_constant_cvalue(const ssa_value* self);
static inline avalue* ssa_get_constant_value(ssa_value* self);

static inline void ssa_set_constant_value(ssa_value* self, const avalue* value);

struct _ssa_label
{
        struct _ssa_value_base base;
};

extern void ssa_init_label(ssa_value* self, tree_type* type);

struct _ssa_string
{
        struct _ssa_value_base base;
        tree_id id;
};

extern ssa_value* ssa_new_string(ssa_context* context, tree_type* type, tree_id ref);

static inline struct _ssa_string* _ssa_string(ssa_value* self);
static inline const struct _ssa_string* _ssa_cstring(const ssa_value* self);

static inline tree_id ssa_get_string_value(const ssa_value* self);

static inline void ssa_set_string_value(ssa_value* self, tree_id id);

struct _ssa_function
{
        struct _ssa_value_base base;
        tree_decl* entity;
        list_head blocks;
        ssa_array params;
};

extern ssa_value* ssa_new_function(ssa_context* context, tree_decl* func);
extern void ssa_add_function_block(ssa_value* self, ssa_block* block);
extern void ssa_add_function_param(ssa_value* self, ssa_context* context, ssa_value* param);
extern bool ssa_function_returns_void(const ssa_value* self);

extern void ssa_number_function_values(ssa_value* self);

static inline struct _ssa_function* _ssa_function(ssa_value* self);
static inline const struct _ssa_function* _ssa_cfunction(const ssa_value* self);

static inline tree_type* ssa_get_function_result_type(const ssa_value* self);
static inline tree_decl* ssa_get_function_entity(const ssa_value* self);
static inline ssa_block* ssa_get_function_blocks_begin(const ssa_value* self);
static inline ssa_block* ssa_get_function_blocks_end(ssa_value* self);
static inline ssa_block* ssa_get_function_blocks_cend(const ssa_value* self);
static inline ssa_value** ssa_get_function_params_begin(const ssa_value* self);
static inline ssa_value** ssa_get_function_params_end(const ssa_value* self);
static inline bool ssa_function_has_body(const ssa_value* self);

static inline void ssa_set_function_entity(ssa_value* self, tree_decl* func);

#define SSA_FOREACH_FUNCTION_BLOCK(PFUNC, ITNAME)\
        for (ssa_block* ITNAME = ssa_get_function_blocks_begin(PFUNC);\
                ITNAME != ssa_get_function_blocks_cend(PFUNC);\
                ITNAME = ssa_get_next_block(ITNAME))

#define SSA_FOREACH_FUNCTION_BLOCK_SAFE(PFUNC, ITNAME, NEXTNAME)\
        for (ssa_instr* ITNAME = ssa_get_function_blocks_begin(PFUNC), *NEXTNAME;\
                (NEXTNAME = ssa_get_next_block(ITNAME)), ITNAME != ssa_get_function_blocks_cend(PFUNC);\
                ITNAME = NEXTNAME)

#define SSA_FOREACH_FUNCTION_PARAM(PFUNC, ITNAME) \
        for (ssa_value** ITNAME = ssa_get_function_params_begin(PFUNC); \
                ITNAME != ssa_get_function_params_end(PFUNC); ITNAME++)

struct _ssa_param
{
        struct _ssa_value_base _base;
};

extern ssa_value* ssa_new_param(ssa_context* context, tree_type* type);

typedef struct _ssa_value
{
        union
        {
                struct _ssa_local_var local_var;
                struct _ssa_global_var global_var;
                struct _ssa_constant constant;
                struct _ssa_label label;
                struct _ssa_string string;
                struct _ssa_param param;
                struct _ssa_function func;
        };
} ssa_value;

static inline ssa_value* ssa_get_value_use_value(const ssa_value_use* self)
{
        return self->value;
}

static inline ssa_instr* ssa_get_value_use_instr(const ssa_value_use* self)
{
        return self->instr;
}

static inline ssa_value_use* ssa_get_next_value_use(const ssa_value_use* self)
{
        return (ssa_value_use*)self->node.next;
}

static inline struct _ssa_value_base* _ssa_value_base(ssa_value* self)
{
        assert(self);
        return (struct _ssa_value_base*)self;
}

static inline const struct _ssa_value_base* _ssa_value_cbase(const ssa_value* self)
{
        assert(self);
        return (const struct _ssa_value_base*)self;
}

static inline ssa_value_use* ssa_get_value_uses_begin(const ssa_value* self)
{
        return (ssa_value_use*)_ssa_value_cbase(self)->use_list.head;
}

static inline ssa_value_use* ssa_get_value_uses_end(ssa_value* self)
{
        return (ssa_value_use*)list_end(&_ssa_value_base(self)->use_list);
}

static inline size_t ssa_get_value_uses_size(ssa_value* self)
{
        return ssa_get_value_uses_end(self) - ssa_get_value_uses_begin(self);
}

static inline const ssa_value_use* ssa_get_value_uses_cend(const ssa_value* self)
{
        return (const ssa_value_use*)list_end_c(&_ssa_value_cbase(self)->use_list);
}

static inline ssa_value_kind ssa_get_value_kind(const ssa_value* self)
{
        return _ssa_value_cbase(self)->kind;
}

static inline ssa_id ssa_get_value_id(const ssa_value* self)
{
        return _ssa_value_cbase(self)->id;
}

static inline tree_type* ssa_get_value_type(const ssa_value* self)
{
        return _ssa_value_cbase(self)->type;
}

static inline bool ssa_value_is_used(const ssa_value* self)
{
        return ssa_get_value_uses_begin(self) != ssa_get_value_uses_cend(self);
}

static inline void* ssa_get_value_metadata(const ssa_value* self)
{
        return _ssa_value_cbase(self)->metadata;
}

static inline void ssa_set_value_metadata(ssa_value* self, void* md)
{
        _ssa_value_base(self)->metadata = md;
}

static inline void* ssa_remove_value_metadata(ssa_value* self)
{
        void* md = ssa_get_value_metadata(self);
        ssa_set_value_metadata(self, NULL);
        return md;
}

static inline void ssa_set_value_kind(ssa_value* self, ssa_value_kind kind)
{
        _ssa_value_base(self)->kind = kind;
}

static inline void ssa_set_value_type(ssa_value* self, tree_type* type)
{
        _ssa_value_base(self)->type = type;
}

#define SSA_ASSERT_VALUE(P, K) assert((P) && ssa_get_value_kind(P) == (K))

static inline struct _ssa_global_var* _ssa_global_var(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_GLOBAL_VAR);
        return (struct _ssa_global_var*)self;
}

static inline const struct _ssa_global_var* _ssa_global_cvar(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_GLOBAL_VAR);
        return (const struct _ssa_global_var*)self;
}

static inline tree_decl* ssa_get_global_var_entity(const ssa_value* self)
{
        return _ssa_global_cvar(self)->entity;
}

static inline void ssa_set_global_var_entity(ssa_value* self, tree_decl* var)
{
        _ssa_global_var(self)->entity = var;
}

static inline struct _ssa_constant* _ssa_constant(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (struct _ssa_constant*)self;
}

static inline const struct _ssa_constant* _ssa_cconstant(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_CONSTANT);
        return (const struct _ssa_constant*)self;
}

static inline const avalue* ssa_get_constant_cvalue(const ssa_value* self)
{
        return &_ssa_cconstant(self)->value;
}

static inline avalue* ssa_get_constant_value(ssa_value* self)
{
        return &_ssa_constant(self)->value;
}

static inline void ssa_set_constant_value(ssa_value* self, const avalue* value)
{
        *ssa_get_constant_value(self) = *value;
}

static inline struct _ssa_string* _ssa_string(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_STRING);
        return (struct _ssa_string*)self;
}

static inline const struct _ssa_string* _ssa_cstring(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_STRING);
        return (const struct _ssa_string*)self;
}

static inline tree_id ssa_get_string_value(const ssa_value* self)
{
        return _ssa_cstring(self)->id;
}

static inline void ssa_set_string_value(ssa_value* self, tree_id id)
{
        _ssa_string(self)->id = id;
}

static inline struct _ssa_function* _ssa_function(ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_FUNCTION);
        return (struct _ssa_function*)self;
}

static inline const struct _ssa_function* _ssa_cfunction(const ssa_value* self)
{
        SSA_ASSERT_VALUE(self, SVK_FUNCTION);
        return (const struct _ssa_function*)self;
}

static inline tree_type* ssa_get_function_result_type(const ssa_value* self)
{
        return tree_get_func_type_result(
                tree_get_decl_type(ssa_get_function_entity(self)));
}

static inline tree_decl* ssa_get_function_entity(const ssa_value* self)
{
        return _ssa_cfunction(self)->entity;
}

static inline ssa_block* ssa_get_function_blocks_begin(const ssa_value* self)
{
        return (ssa_block*)_ssa_cfunction(self)->blocks.head;
}

static inline ssa_block* ssa_get_function_blocks_end(ssa_value* self)
{
        return (ssa_block*)list_end(&_ssa_function(self)->blocks);
}

static inline ssa_block* ssa_get_function_blocks_cend(const ssa_value* self)
{
        return (ssa_block*)list_end_c(&_ssa_cfunction(self)->blocks);
}

static inline ssa_value** ssa_get_function_params_begin(const ssa_value* self)
{
        return (ssa_value**)_ssa_cfunction(self)->params.data;
}

static inline ssa_value** ssa_get_function_params_end(const ssa_value* self)
{
        return ssa_get_function_params_begin(self) + _ssa_cfunction(self)->params.size;
}

static inline bool ssa_function_has_body(const ssa_value* self)
{
        return !list_empty(&_ssa_cfunction(self)->blocks);
}

static inline void ssa_set_function_entity(ssa_value* self, tree_decl* func)
{
        _ssa_function(self)->entity = func;
}

#ifdef __cplusplus
}
#endif

#endif