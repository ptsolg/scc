#ifndef CTOKEN_H
#define CTOKEN_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree-common.h"
#include "c-token-kind.h"

typedef struct _ctree_context ctree_context;
typedef struct _ctoken ctoken;

struct _ctoken_base
{
        ctoken_kind _kind;
        tree_location _loc;
};

extern ctoken* ctoken_new(ctree_context* context, ctoken_kind kind, tree_location loc);

extern ctoken* ctoken_new_ex(
        ctree_context* context, ctoken_kind kind, tree_location loc, ssize size);

static inline struct _ctoken_base* _ctoken_get(ctoken* self);
static inline const struct _ctoken_base* _ctoken_cget(const ctoken* self);

static inline ctoken* ctoken_get_next(const ctoken* self);
static inline ctoken* ctoken_get_prev(const ctoken* self);
static inline ctoken_kind ctoken_get_kind(const ctoken* self);
static inline bool ctoken_is(const ctoken* self, ctoken_kind k);
static inline tree_location ctoken_get_loc(const ctoken* self);
static inline list_node* ctoken_get_node(ctoken* self);
static inline const list_node* ctoken_get_cnode(const ctoken* self);


static inline void ctoken_set_kind(ctoken* self, ctoken_kind k);
static inline void ctoken_set_loc(ctoken* self, tree_location l);

struct _cstring_token
{
        struct _ctoken_base _base;
        tree_id _string;
};

extern ctoken* ctoken_new_string_ex(
        ctree_context* context, ctoken_kind kind, tree_location loc, tree_id ref, ssize size);

extern ctoken* ctoken_new_string(ctree_context* context, tree_location loc, tree_id ref);
extern ctoken* ctoken_new_angle_string(ctree_context* context, tree_location loc, tree_id ref);
extern ctoken* ctoken_new_id(ctree_context* context, tree_location loc, tree_id id);

static inline struct _cstring_token* _cstring_token_get(ctoken* self);
static inline const struct _cstring_token* _cstring_token_cget(const ctoken* self);

static inline tree_id ctoken_get_string(const ctoken* self);
static inline void ctoken_set_string(ctoken* self, tree_id s);

struct _cfloat_token
{
        struct _ctoken_base _base;
        float _value;
};

extern ctoken* ctoken_new_float(ctree_context* context, tree_location loc, float val);

static inline struct _cfloat_token* _cfloat_token_get(ctoken* self);
static inline const struct _cfloat_token* _cfloat_token_cget(const ctoken* self);

static inline float ctoken_get_float(const ctoken* self);
static inline void ctoken_set_float(ctoken* self, float v);

struct _cdouble_token
{
        struct _ctoken_base _base;
        ldouble _value;
};

extern ctoken* ctoken_new_double(ctree_context* context, tree_location loc, ldouble val);

static inline struct _cdouble_token* _cdouble_token_get(ctoken* self);
static inline const struct _cdouble_token* _cdouble_token_cget(const ctoken* self);

static inline ldouble ctoken_get_double(const ctoken* self);
static inline void ctoken_set_double(ctoken* self, ldouble v);

struct _cint_token
{
        struct _ctoken_base _base;
        suint64 _value;
        bool _signed;
        int _ls;
};

extern ctoken* ctoken_new_int(
        ctree_context* context, tree_location loc, suint64 val, bool is_signed, int ls);

static inline struct _cint_token* _cint_token_get(ctoken* self);
static inline const struct _cint_token* _cint_token_cget(const ctoken* self);

static inline suint64 ctoken_get_int(const ctoken* self);
static inline int ctoken_get_int_ls(const ctoken* self);
static inline bool ctoken_is_int_signed(const ctoken* self);

static inline void ctoken_set_int(ctoken* self, suint64 v);
static inline void ctoken_set_int_signed(ctoken* self, bool s);
static inline void ctoken_set_int_ls(ctoken* self, int n);

struct _cchar_token
{
        struct _ctoken_base _base;
        int _value;
};

extern ctoken* ctoken_new_char(ctree_context* context, tree_location loc, int val);

static inline struct _cchar_token* _cchar_token_get(ctoken* self);
static inline const struct _cchar_token* _cchar_token_cget(const ctoken* self);

static inline int ctoken_get_char(const ctoken* self);
static inline void ctoken_set_char(ctoken* self, int v);

struct _cwspace_token
{
        struct _ctoken_base _base;
        int _count;
};

extern ctoken* ctoken_new_wspace(ctree_context* context, tree_location loc, int count);

static inline struct _cwspace_token* _cwspace_token_get(ctoken* self);
static inline const struct _cwspace_token* _cwspace_token_cget(const ctoken* self);

static inline int ctoken_get_spaces(const ctoken* self);
static inline void ctoken_set_spaces(ctoken* self, int count);

typedef struct _ctoken
{
        union
        {
                struct _ctoken_base _token;
                struct _cstring_token _string;
                struct _cfloat_token _float;
                struct _cdouble_token _double;
                struct _cint_token _int;
                struct _cchar_token _char;
                struct _cwspace_token _wspace;
        };
} ctoken;

extern ctoken* ctoken_copy(ctree_context* self, const ctoken* other);
// allocates token with a size sufficient for converting it to float/double/int token
extern ctoken* ctoken_new_pp_num(ctree_context* context, tree_location loc, tree_id ref);

#define CTOKEN_ASSERT(P, K) S_ASSERT((P) && ctoken_is((P), (K)))

static inline struct _ctoken_base* _ctoken_get(ctoken* self)
{
        S_ASSERT(self);
        return (struct _ctoken_base*)self;
}

static inline const struct _ctoken_base* _ctoken_cget(const ctoken* self)
{
        S_ASSERT(self);
        return (const struct _ctoken_base*)self;
}

static inline ctoken_kind ctoken_get_kind(const ctoken* self)
{
        return _ctoken_cget(self)->_kind;
}

static inline bool ctoken_is(const ctoken* self, ctoken_kind k)
{
        return ctoken_get_kind(self) == k;
}

static inline tree_location ctoken_get_loc(const ctoken* self)
{
        return _ctoken_cget(self)->_loc;
}

static inline void ctoken_set_kind(ctoken* self, ctoken_kind k)
{
        _ctoken_get(self)->_kind = k;
}

static inline void ctoken_set_loc(ctoken* self, tree_location l)
{
        _ctoken_get(self)->_loc = l;
}

#define CSTRING_TOKEN_ASSERT(P, K) \
        S_ASSERT((P) && (K) == CTK_ID || (K) == CTK_CONST_STRING || (K) == CTK_PP_NUM \
                || (K) == CTK_ANGLE_STRING)

static inline struct _cstring_token* _cstring_token_get(ctoken* self)
{
        CSTRING_TOKEN_ASSERT(self, self->_token._kind);
        return (struct _cstring_token*)self;
}

static inline const struct _cstring_token* _cstring_token_cget(const ctoken* self)
{
        CSTRING_TOKEN_ASSERT(self, self->_token._kind);
        return (const struct _cstring_token*)self;
}

static inline tree_id ctoken_get_string(const ctoken* self)
{
        return _cstring_token_cget(self)->_string;
}

static inline void ctoken_set_string(ctoken* self, tree_id s)
{
        _cstring_token_get(self)->_string = s;
}

static inline struct _cfloat_token* _cfloat_token_get(ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_FLOAT);
        return (struct _cfloat_token*)self;
}

static inline const struct _cfloat_token* _cfloat_token_cget(const ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_FLOAT);
        return (const struct _cfloat_token*)self;
}

static inline float ctoken_get_float(const ctoken* self)
{
        return _cfloat_token_cget(self)->_value;
}

static inline void ctoken_set_float(ctoken* self, float v)
{
        _cfloat_token_get(self)->_value = v;
}

static inline struct _cdouble_token* _cdouble_token_get(ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_DOUBLE);
        return (struct _cdouble_token*)self;
}

static inline const struct _cdouble_token* _cdouble_token_cget(const ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_DOUBLE);
        return (const struct _cdouble_token*)self;
}

static inline ldouble ctoken_get_double(const ctoken* self)
{
        return _cdouble_token_cget(self)->_value;
}

static inline void ctoken_set_double(ctoken* self, ldouble v)
{
        _cdouble_token_get(self)->_value = v;
}

static inline struct _cint_token* _cint_token_get(ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_INT);
        return (struct _cint_token*)self;
}

static inline const struct _cint_token* _cint_token_cget(const ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_INT);
        return (const struct _cint_token*)self;
}

static inline suint64 ctoken_get_int(const ctoken* self)
{
        return _cint_token_cget(self)->_value;
}

static inline int ctoken_get_int_ls(const ctoken* self)
{
        return _cint_token_cget(self)->_ls;
}

static inline bool ctoken_is_int_signed(const ctoken* self)
{
        return _cint_token_cget(self)->_signed;
}

static inline void ctoken_set_int(ctoken* self, suint64 v)
{
        _cint_token_get(self)->_value = v;
}

static inline void ctoken_set_int_signed(ctoken* self, bool s)
{
        _cint_token_get(self)->_signed = s;
}

static inline void ctoken_set_int_ls(ctoken* self, int n)
{
        S_ASSERT(n >= 0 && n < 3 && "integer suffix can have maximum 2 L's");
        _cint_token_get(self)->_ls = n;
}

static inline struct _cchar_token* _cchar_token_get(ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_CHAR);
        return (struct _cchar_token*)self;
}

static inline const struct _cchar_token* _cchar_token_cget(const ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_CONST_CHAR);
        return (const struct _cchar_token*)self;
}

static inline int ctoken_get_char(const ctoken* self)
{
        return _cchar_token_cget(self)->_value;
}

static inline void ctoken_set_char(ctoken* self, int v)
{
        _cchar_token_get(self)->_value = v;
}

static inline struct _cwspace_token* _cwspace_token_get(ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_WSPACE);
        return (struct _cwspace_token*)self;
}

static inline const struct _cwspace_token* _cwspace_token_cget(const ctoken* self)
{
        CTOKEN_ASSERT(self, CTK_WSPACE);
        return (const struct _cwspace_token*)self;
}

static inline int ctoken_get_spaces(const ctoken* self)
{
        return _cwspace_token_cget(self)->_count;
}

static inline void ctoken_set_spaces(ctoken* self, int count)
{
        _cwspace_token_get(self)->_count = count;
}

#ifdef __cplusplus
}
#endif

#endif // !CTOKEN_H
