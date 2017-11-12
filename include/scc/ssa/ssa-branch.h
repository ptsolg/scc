#ifndef SSA_BRANCH_H
#define SSA_BRANCH_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _ssa_block ssa_block;
typedef struct _ssa_value ssa_value;
typedef struct _ssa_context ssa_context;
typedef struct _ssa_branch ssa_branch;

typedef enum
{
        SBK_INVALID,
        SBK_IF,
        SBK_SWITCH,
        SBK_RETURN,
        SBK_JUMP,

} ssa_branch_kind;

struct _ssa_branch_base
{
        ssa_branch_kind _kind;
};

extern ssa_branch* ssa_new_branch(ssa_context* context, ssa_branch_kind kind, ssize size);

static inline struct _ssa_branch_base* _ssa_get_branch_base(ssa_branch* self);
static inline const struct _ssa_branch_base* _ssa_get_branch_cbase(const ssa_branch* self);

static inline ssa_branch_kind ssa_get_branch_kind(const ssa_branch* self);
static inline void ssa_set_branch_kind(ssa_branch* self, ssa_branch_kind k);

struct _ssa_if_branch
{
        struct _ssa_branch_base _base;
        ssa_value* _cond;
        ssa_block* _true;
        ssa_block* _false;
};

extern ssa_branch* ssa_new_if_branch(
        ssa_context* context, ssa_value* cond, ssa_block* if_true, ssa_block* if_false);

static inline struct _ssa_if_branch* _ssa_get_if(ssa_branch* self);
static inline const struct _ssa_if_branch* _ssa_get_cif(const ssa_branch* self);

static inline ssa_value* ssa_get_if_cond(const ssa_branch* self);
static inline ssa_block* ssa_get_if_true_block(const ssa_branch* self);
static inline ssa_block* ssa_get_if_false_block(const ssa_branch* self);

static inline void ssa_set_if_cond(ssa_branch* self, ssa_value* val);
static inline void ssa_set_if_true_block(ssa_branch* self, ssa_block* true_block);
static inline void ssa_set_if_false_block(ssa_branch* self, ssa_block* false_block);

struct _ssa_return_branch
{
        struct _ssa_branch_base _base;
        ssa_value* _value;
};

extern ssa_branch* ssa_new_return_branch(ssa_context* context, ssa_value* val);

static inline struct _ssa_return_branch* _ssa_get_return(ssa_branch* self);
static inline const struct _ssa_return_branch* _ssa_get_creturn(const ssa_branch* self);

static inline ssa_value* ssa_get_return_value(const ssa_branch* self);
static inline void ssa_set_return_value(ssa_branch* self, ssa_value* value);

struct _ssa_jump_branch
{
        struct _ssa_branch_base _base;
        ssa_block* _dest;
};

extern ssa_branch* ssa_new_jump_branch(ssa_context* context, ssa_block* dest);

static inline struct _ssa_jump_branch* _ssa_get_jump(ssa_branch* self);
static inline const struct _ssa_jump_branch* _ssa_get_cjump(const ssa_branch* self);

static inline ssa_block* ssa_get_jump_dest(const ssa_branch* self);
static inline void ssa_set_jump_dest(ssa_branch* self, ssa_block* dest);

typedef struct _ssa_branch
{
        union
        {
                struct _ssa_if_branch _if;
                struct _ssa_return_branch _ret;
                struct _ssa_jump_branch _jmp;
        };
} ssa_branch;

#define SSA_ASSERT_BRANCH_BASE(P) S_ASSERT(P)

static inline struct _ssa_branch_base* _ssa_get_branch_base(ssa_branch* self)
{
        SSA_ASSERT_BRANCH_BASE(self);
        return (struct _ssa_branch_base*)self;
}

static inline const struct _ssa_branch_base* _ssa_get_branch_cbase(const ssa_branch* self)
{
        SSA_ASSERT_BRANCH_BASE(self);
        return (const struct _ssa_branch_base*)self;
}

static inline ssa_branch_kind ssa_get_branch_kind(const ssa_branch* self)
{
        return _ssa_get_branch_cbase(self)->_kind;
}

static inline void ssa_set_branch_kind(ssa_branch* self, ssa_branch_kind k)
{
        _ssa_get_branch_base(self)->_kind = k;
}

#define SSA_ASSERT_BRANCH(P, K) S_ASSERT((P) && ssa_get_branch_kind(P) == (K))

static inline struct _ssa_if_branch* _ssa_get_if(ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_IF);
        return (struct _ssa_if_branch*)self;
}

static inline const struct _ssa_if_branch* _ssa_get_cif(const ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_IF);
        return (const struct _ssa_if_branch*)self;
}

static inline ssa_value* ssa_get_if_cond(const ssa_branch* self)
{
        return _ssa_get_cif(self)->_cond;
}

static inline ssa_block* ssa_get_if_true_block(const ssa_branch* self)
{
        return _ssa_get_cif(self)->_true;
}

static inline ssa_block* ssa_get_if_false_block(const ssa_branch* self)
{
        return _ssa_get_cif(self)->_false;
}

static inline void ssa_set_if_cond(ssa_branch* self, ssa_value* val)
{
        _ssa_get_if(self)->_cond = val;
}

static inline void ssa_set_if_true_block(ssa_branch* self, ssa_block* true_block)
{
        _ssa_get_if(self)->_true = true_block;
}

static inline void ssa_set_if_false_block(ssa_branch* self, ssa_block* false_block)
{
        _ssa_get_if(self)->_false = false_block;
}

static inline struct _ssa_return_branch* _ssa_get_return(ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_RETURN);
        return (struct _ssa_return_branch*)self;
}

static inline const struct _ssa_return_branch* _ssa_get_creturn(const ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_RETURN);
        return (const struct _ssa_return_branch*)self;
}

static inline ssa_value* ssa_get_return_value(const ssa_branch* self)
{
        return _ssa_get_creturn(self)->_value;
}

static inline void ssa_set_return_value(ssa_branch* self, ssa_value* value)
{
        _ssa_get_return(self)->_value = value;
}

static inline struct _ssa_jump_branch* _ssa_get_jump(ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_JUMP);
        return (struct _ssa_jump_branch*)self;
}

static inline const struct _ssa_jump_branch* _ssa_get_cjump(const ssa_branch* self)
{
        SSA_ASSERT_BRANCH(self, SBK_JUMP);
        return (const struct _ssa_jump_branch*)self;
}

static inline ssa_block* ssa_get_jump_dest(const ssa_branch* self)
{
        return _ssa_get_cjump(self)->_dest;
}

static inline void ssa_set_jump_dest(ssa_branch* self, ssa_block* dest)
{
        _ssa_get_jump(self)->_dest = dest;
}

#ifdef __cplusplus
}
#endif

#endif