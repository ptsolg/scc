#ifndef C_TOKEN_H
#define C_TOKEN_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree-common.h"
#include "c-token-kind.h"

typedef struct _c_token c_token;
typedef struct _c_context c_context;

struct _c_token_base
{
        c_token_kind kind;
        tree_location loc;
};

extern c_token* c_token_new(c_context* context, c_token_kind kind, tree_location loc);

extern c_token* c_token_new_ex(
        c_context* context, c_token_kind kind, tree_location loc, ssize size);

static inline struct _c_token_base* _c_token_base(c_token* self);
static inline const struct _c_token_base* _c_token_base_c(const c_token* self);

static inline c_token_kind c_token_get_kind(const c_token* self);
static inline bool c_token_is(const c_token* self, c_token_kind k);
static inline tree_location c_token_get_loc(const c_token* self);

static inline void c_token_set_kind(c_token* self, c_token_kind k);
static inline void c_token_set_loc(c_token* self, tree_location l);

struct _c_string_token
{
        struct _c_token_base base;
        tree_id ref;
};

extern c_token* c_token_new_string_ex(
        c_context* context, c_token_kind kind, tree_location loc, tree_id ref, ssize size);

extern c_token* c_token_new_string(c_context* context, tree_location loc, tree_id ref);
extern c_token* c_token_new_angle_string(c_context* context, tree_location loc, tree_id ref);
extern c_token* c_token_new_id(c_context* context, tree_location loc, tree_id id);

static inline struct _c_string_token* _c_string_token(c_token* self);
static inline const struct _c_string_token* _c_string_token_c(const c_token* self);

static inline tree_id c_token_get_string(const c_token* self);
static inline void c_token_set_string(c_token* self, tree_id s);

struct _c_float_token
{
        struct _c_token_base base;
        float value;
};

extern c_token* c_token_new_float(c_context* context, tree_location loc, float val);

static inline struct _c_float_token* _c_float_token(c_token* self);
static inline const struct _c_float_token* _c_float_token_c(const c_token* self);

static inline float c_token_get_float(const c_token* self);
static inline void c_token_set_float(c_token* self, float v);

struct _c_double_token
{
        struct _c_token_base base;
        ldouble value;
};

extern c_token* c_token_new_double(c_context* context, tree_location loc, ldouble val);

static inline struct _c_double_token* _c_double_token(c_token* self);
static inline const struct _c_double_token* _c_double_token_c(const c_token* self);

static inline ldouble c_token_get_double(const c_token* self);
static inline void c_token_set_double(c_token* self, ldouble v);

struct _c_int_token
{
        struct _c_token_base base;
        suint64 value;
        bool signed_;
        int ls;
};

extern c_token* c_token_new_int(
        c_context* context, tree_location loc, suint64 val, bool is_signed, int ls);

static inline struct _c_int_token* _c_int_token(c_token* self);
static inline const struct _c_int_token* _c_int_token_c(const c_token* self);

static inline suint64 c_token_get_int(const c_token* self);
static inline int c_token_get_int_ls(const c_token* self);
static inline bool c_token_int_is_signed(const c_token* self);

static inline void c_token_set_int(c_token* self, suint64 v);
static inline void c_token_set_int_signed(c_token* self, bool s);
static inline void c_token_set_int_ls(c_token* self, int n);

struct _c_char_token
{
        struct _c_token_base base;
        int value;
};

extern c_token* c_token_new_char(c_context* context, tree_location loc, int val);

static inline struct _c_char_token* _c_char_token(c_token* self);
static inline const struct _c_char_token* _c_char_token_c(const c_token* self);

static inline int c_token_get_char(const c_token* self);
static inline void c_token_set_char(c_token* self, int v);

struct _c_wspace_token
{
        struct _c_token_base base;
        int count;
};

extern c_token* c_token_new_wspace(c_context* context, tree_location loc, int count);

static inline struct _c_wspace_token* _cwspace_token_get(c_token* self);
static inline const struct _c_wspace_token* _cwspace_token_get_c(const c_token* self);

static inline int c_token_get_spaces(const c_token* self);
static inline void c_token_set_spaces(c_token* self, int count);

typedef struct _c_token
{
        union
        {
                struct _c_token_base base;
                struct _c_string_token string;
                struct _c_float_token float_token;
                struct _c_double_token double_token;
                struct _c_int_token int_token;
                struct _c_char_token char_token;
                struct _c_wspace_token wspace;
        };
} c_token;

// allocates token with a size sufficient for converting it to float/double/int token
extern c_token* c_token_new_pp_num(c_context* context, tree_location loc, tree_id ref);
extern c_token* c_token_copy(c_context* context, c_token* token);

#define CTOKEN_ASSERT(P, K) S_ASSERT((P) && c_token_is((P), (K)))

static inline struct _c_token_base* _c_token_base(c_token* self)
{
        S_ASSERT(self);
        return (struct _c_token_base*)self;
}

static inline const struct _c_token_base* _c_token_base_c(const c_token* self)
{
        S_ASSERT(self);
        return (const struct _c_token_base*)self;
}

static inline c_token_kind c_token_get_kind(const c_token* self)
{
        return _c_token_base_c(self)->kind;
}

static inline bool c_token_is(const c_token* self, c_token_kind k)
{
        return c_token_get_kind(self) == k;
}

static inline tree_location c_token_get_loc(const c_token* self)
{
        return _c_token_base_c(self)->loc;
}

static inline void c_token_set_kind(c_token* self, c_token_kind k)
{
        _c_token_base(self)->kind = k;
}

static inline void c_token_set_loc(c_token* self, tree_location l)
{
        _c_token_base(self)->loc = l;
}

#define CSTRING_TOKEN_ASSERT(P, K) \
        S_ASSERT((P) && (K) == CTK_ID || (K) == CTK_CONST_STRING || (K) == CTK_PP_NUM \
                || (K) == CTK_ANGLE_STRING)

static inline struct _c_string_token* _c_string_token(c_token* self)
{
        CSTRING_TOKEN_ASSERT(self, self->base.kind);
        return (struct _c_string_token*)self;
}

static inline const struct _c_string_token* _c_string_token_c(const c_token* self)
{
        CSTRING_TOKEN_ASSERT(self, self->base.kind);
        return (const struct _c_string_token*)self;
}

static inline tree_id c_token_get_string(const c_token* self)
{
        return _c_string_token_c(self)->ref;
}

static inline void c_token_set_string(c_token* self, tree_id s)
{
        _c_string_token(self)->ref = s;
}

static inline struct _c_float_token* _c_float_token(c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_FLOAT);
        return (struct _c_float_token*)self;
}

static inline const struct _c_float_token* _c_float_token_c(const c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_FLOAT);
        return (const struct _c_float_token*)self;
}

static inline float c_token_get_float(const c_token* self)
{
        return _c_float_token_c(self)->value;
}

static inline void c_token_set_float(c_token* self, float v)
{
        _c_float_token(self)->value = v;
}

static inline struct _c_double_token* _c_double_token(c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_DOUBLE);
        return (struct _c_double_token*)self;
}

static inline const struct _c_double_token* _c_double_token_c(const c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_DOUBLE);
        return (const struct _c_double_token*)self;
}

static inline ldouble c_token_get_double(const c_token* self)
{
        return _c_double_token_c(self)->value;
}

static inline void c_token_set_double(c_token* self, ldouble v)
{
        _c_double_token(self)->value = v;
}

static inline struct _c_int_token* _c_int_token(c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_INT);
        return (struct _c_int_token*)self;
}

static inline const struct _c_int_token* _c_int_token_c(const c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_INT);
        return (const struct _c_int_token*)self;
}

static inline suint64 c_token_get_int(const c_token* self)
{
        return _c_int_token_c(self)->value;
}

static inline int c_token_get_int_ls(const c_token* self)
{
        return _c_int_token_c(self)->ls;
}

static inline bool c_token_int_is_signed(const c_token* self)
{
        return _c_int_token_c(self)->signed_;
}

static inline void c_token_set_int(c_token* self, suint64 v)
{
        _c_int_token(self)->value = v;
}

static inline void c_token_set_int_signed(c_token* self, bool s)
{
        _c_int_token(self)->signed_ = s;
}

static inline void c_token_set_int_ls(c_token* self, int n)
{
        S_ASSERT(n >= 0 && n < 3 && "integer suffix can have maximum 2 L's");
        _c_int_token(self)->ls = n;
}

static inline struct _c_char_token* _c_char_token(c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_CHAR);
        return (struct _c_char_token*)self;
}

static inline const struct _c_char_token* _c_char_token_c(const c_token* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_CHAR);
        return (const struct _c_char_token*)self;
}

static inline int c_token_get_char(const c_token* self)
{
        return _c_char_token_c(self)->value;
}

static inline void c_token_set_char(c_token* self, int v)
{
        _c_char_token(self)->value = v;
}

static inline struct _c_wspace_token* _cwspace_token_get(c_token* self)
{
        CTOKEN_ASSERT(self, CTK_WSPACE);
        return (struct _c_wspace_token*)self;
}

static inline const struct _c_wspace_token* _cwspace_token_get_c(const c_token* self)
{
        CTOKEN_ASSERT(self, CTK_WSPACE);
        return (const struct _c_wspace_token*)self;
}

static inline int c_token_get_spaces(const c_token* self)
{
        return _cwspace_token_get_c(self)->count;
}

static inline void c_token_set_spaces(c_token* self, int count)
{
        _cwspace_token_get(self)->count = count;
}

#ifdef __cplusplus
}
#endif

#endif
