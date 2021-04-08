#ifndef SSA_CONST_H
#define SSA_CONST_H

#include "scc/core/num.h"
#include "scc/core/vec.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_const ssa_const;
typedef struct _ssa_value ssa_value;
typedef struct _tree_type tree_type;

typedef enum
{
        SCK_INVALID,
        SCK_ADDRESS,
        SCK_CAST,
        SCK_LITERAL,
        SCK_PTRADD,
        SCK_GETFIELDADDR,
        SCK_LIST,
} ssa_const_kind;

struct _ssa_const_base
{
        ssa_const_kind kind;
        tree_type* type;
};

extern ssa_const* ssa_new_const(
        ssa_context* context,
        ssa_const_kind kind,
        tree_type* type,
        size_t size);

static inline bool ssa_const_is(const ssa_const* self, ssa_const_kind kind);
static inline ssa_const_kind ssa_get_const_kind(const ssa_const* self);
static inline void ssa_set_const_kind(ssa_const* self, ssa_const_kind kind);
static inline tree_type* ssa_get_const_type(const ssa_const* self);
static inline void ssa_set_const_type(ssa_const* self, tree_type* type);

struct _ssa_const_addr
{
        struct _ssa_const_base base;
        ssa_value* var;
};

extern ssa_const* ssa_new_const_addr(
        ssa_context* context,
        tree_type* type,
        ssa_value* var);

static inline struct _ssa_const_addr* _ssa_const_addr(ssa_const* self);
static inline ssa_value* ssa_get_const_addr(const ssa_const* self);
static inline void ssa_set_const_addr(ssa_const* self, ssa_value* var);

struct _ssa_const_expr
{
        struct _ssa_const_base base;
        ssa_const* operands[];
};

extern ssa_const* ssa_new_const_expr(
        ssa_context* context,
        ssa_const_kind kind,
        tree_type* type,
        size_t num_operands);

extern ssa_const* ssa_new_const_cast(
        ssa_context* context,
        tree_type* type,
        ssa_const* operand);

extern ssa_const* ssa_new_const_ptradd(
        ssa_context* context,
        tree_type* type,
        ssa_const* ptr,
        ssa_const* offset);

extern ssa_const* ssa_new_const_ptrsub(
        ssa_context* context,
        tree_type* type,
        ssa_const* ptr,
        ssa_const* offset);

static inline struct _ssa_const_expr* _ssa_const_expr(ssa_const* self);
static inline ssa_const* ssa_get_const_expr_operand(const ssa_const* self, unsigned i);
static inline void ssa_set_const_expr_operand(ssa_const* self, unsigned i, ssa_const* operand);

struct _ssa_const_literal
{
        struct _ssa_const_base base;
        struct num value;
};

extern ssa_const* ssa_new_const_literal(
        ssa_context* context,
        tree_type* type,
        struct num value);

static inline struct _ssa_const_literal* _ssa_const_literal(ssa_const* self);
static inline struct num* ssa_get_const_literal(ssa_const* self);
static inline void ssa_set_const_literal(ssa_const* self, struct num num);

struct _ssa_const_field_addr
{
        struct _ssa_const_base base;
        ssa_const* var;
        unsigned field_index;
};

extern ssa_const* ssa_new_const_field_addr(
        ssa_context* context,
        tree_type* type,
        ssa_const* var,
        unsigned field_index);

static inline struct _ssa_const_field_addr* _ssa_const_field_addr(ssa_const* self);
static inline ssa_const* ssa_get_const_field_addr_var(const ssa_const* self);
static inline void ssa_set_const_field_addr_var(ssa_const* self, ssa_const* var);
static inline unsigned ssa_get_const_field_addr_index(const ssa_const* self);
static inline void ssa_set_const_field_addr_index(ssa_const* self, unsigned index);

struct _ssa_const_list
{
        struct _ssa_const_base base;
        struct vec list;
};

extern ssa_const* ssa_new_const_list(
        ssa_context* context, tree_type* type);

static inline struct _ssa_const_list* _ssa_const_list(ssa_const* self);
static inline struct vec* ssa_get_const_list(ssa_const* self);

typedef struct _ssa_const
{
        union
        {
                struct _ssa_const_base _base;
                struct _ssa_const_addr _addr;
                struct _ssa_const_expr _expr;
                struct _ssa_const_literal _lit;
                struct _ssa_const_field_addr _field_addr;
                struct _ssa_const_list _list;
        };
} ssa_const;

static inline bool ssa_const_is(const ssa_const* self, ssa_const_kind kind)
{
        assert(self);
        return self->_base.kind == kind;
}

static inline ssa_const_kind ssa_get_const_kind(const ssa_const* self)
{
        assert(self);
        return self->_base.kind;
}

static inline void ssa_set_const_kind(ssa_const* self, ssa_const_kind kind)
{
        assert(self);
        self->_base.kind = kind;
}

static inline tree_type* ssa_get_const_type(const ssa_const* self)
{
        assert(self);
        return self->_base.type;
}

static inline void ssa_set_const_type(ssa_const* self, tree_type* type)
{
        assert(self);
        self->_base.type = type;
}

static inline struct _ssa_const_addr* _ssa_const_addr(ssa_const* self)
{
        assert(self && self->_base.kind == SCK_ADDRESS);
        return &self->_addr;
}

static inline const struct _ssa_const_addr* _ssa_const_addr_c(const ssa_const* self)
{
        assert(self && self->_base.kind == SCK_ADDRESS);
        return &self->_addr;
}

static inline ssa_value* ssa_get_const_addr(const ssa_const* self)
{
        return _ssa_const_addr_c(self)->var;
}

static inline void ssa_set_const_addr(ssa_const* self, ssa_value* var)
{
        _ssa_const_addr(self)->var = var;
}

static inline struct _ssa_const_expr* _ssa_const_expr(ssa_const* self)
{
        assert(self && (self->_base.kind == SCK_CAST
                || self->_base.kind == SCK_PTRADD));
        return &self->_expr;
}

static inline const struct _ssa_const_expr* _ssa_const_expr_c(const ssa_const* self)
{
        assert(self && (self->_base.kind == SCK_CAST
                || self->_base.kind == SCK_PTRADD));
        return &self->_expr;
}

static inline ssa_const* ssa_get_const_expr_operand(const ssa_const* self, unsigned i)
{
        assert(i < 2);
        return _ssa_const_expr_c(self)->operands[i];
}

static inline void ssa_set_const_expr_operand(ssa_const* self, unsigned i, ssa_const* operand)
{
        assert(i < 2);
        _ssa_const_expr(self)->operands[i] = operand;
}

static inline struct _ssa_const_literal* _ssa_const_literal(ssa_const* self)
{
        assert(self && self->_base.kind == SCK_LITERAL);
        return &self->_lit;
}

static inline struct num* ssa_get_const_literal(ssa_const* self)
{
        return &_ssa_const_literal(self)->value;
}

static inline void ssa_set_const_literal(ssa_const* self, struct num num)
{
        *ssa_get_const_literal(self) = num;
}

static inline struct _ssa_const_field_addr* _ssa_const_field_addr(ssa_const* self)
{
        assert(self && self->_base.kind == SCK_GETFIELDADDR);
        return &self->_field_addr;
}

static inline const struct _ssa_const_field_addr* _ssa_const_field_addr_c(const ssa_const* self)
{
        assert(self && self->_base.kind == SCK_GETFIELDADDR);
        return &self->_field_addr;
}

static inline ssa_const* ssa_get_const_field_addr_var(const ssa_const* self)
{
        return _ssa_const_field_addr_c(self)->var;
}

static inline void ssa_set_const_field_addr_var(ssa_const* self, ssa_const* var)
{
        _ssa_const_field_addr(self)->var = var;
}

static inline unsigned ssa_get_const_field_addr_index(const ssa_const* self)
{
        return _ssa_const_field_addr_c(self)->field_index;
}

static inline void ssa_set_const_field_addr_index(ssa_const* self, unsigned index)
{
        _ssa_const_field_addr(self)->field_index = index;
}

static inline struct _ssa_const_list* _ssa_const_list(ssa_const* self)
{
        assert(self && self->_base.kind == SCK_LIST);
        return &self->_list;
}

static inline struct vec* ssa_get_const_list(ssa_const* self)
{
        return &_ssa_const_list(self)->list;
}

#endif
