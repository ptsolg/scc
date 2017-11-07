#ifndef CTREE_H
#define CTREE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <libtree/tree.h>
#include <setjmp.h>

typedef struct _cident_policy
{
        bool use_tags;
} cident_policy;

extern void cident_policy_init(cident_policy* self);

extern tree_id cident_policy_to_tag(const cident_policy* self, tree_id id);
extern tree_id cident_policy_from_tag(const cident_policy* self, tree_id tag);
extern tree_id cident_policy_get_orig_decl_name(const cident_policy* self, const tree_decl* decl);

typedef struct _ctree_context_allocator
{
        allocator base;
        allocator* alloc;
        jmp_buf* on_out_of_memory;
} ctree_context_allocator;

extern void ctree_context_allocator_init(ctree_context_allocator* self, allocator* alloc);

typedef struct _ctree_context
{
        tree_context base;
        ctree_context_allocator alloc;
} ctree_context;

extern void ctree_context_init(ctree_context* self);
extern void ctree_context_init_ex(ctree_context* self, allocator* alloc);
extern void ctree_context_dispose(ctree_context* self);
extern void ctree_context_set_on_out_of_memory(ctree_context* self, jmp_buf* b);
extern tree_id ctree_context_add_string(ctree_context* self, const char* s, ssize len);

static inline tree_context* ctree_context_base(ctree_context* self)
{
        return &self->base;
}

static inline const tree_context* ctree_context_cbase(const ctree_context* self)
{
        return &self->base;
}

typedef struct
{
        bool unary;
        tree_location loc;
        union
        {
                tree_type* type;
                tree_expr* expr;
                void* pointer;
        };
} csizeof_rhs;

extern void csizeof_expr_init(csizeof_rhs* self, tree_expr* e);
extern void csizeof_type_init(csizeof_rhs* self, tree_type* t, tree_location loc);

typedef struct
{
        tree_type* entity;
        bool type_iterated;
        tree_decl* member_pos;
        int array_pos;
} cinit_iterator;

extern void cinit_iterator_init(cinit_iterator* self, tree_type* entity);
extern bool cinit_iterator_at_record(const cinit_iterator* self);
extern bool cinit_iterator_at_array(const cinit_iterator* self);
extern bool cinit_iterator_advance(cinit_iterator* self);
extern tree_type* cinit_iterator_get_pos_type(cinit_iterator* self);

typedef struct
{
        // void/char/int/float or double
        tree_builtin_type_kind base;

        // counters for long/short/...
        int nshort;
        int nlong;
        int nsigned;
        int nunsigned;
} cbuiltin_type_info;

extern void cbuiltin_type_info_init(cbuiltin_type_info* self);

// returns false if type specifier combination is invalid
extern bool cbuiltin_type_info_set_base(cbuiltin_type_info* self, tree_builtin_type_kind base);
extern bool cbuiltin_type_info_set_signed(cbuiltin_type_info* self);
extern bool cbuiltin_type_info_set_unsigned(cbuiltin_type_info* self);
extern bool cbuiltin_type_info_set_short(cbuiltin_type_info* self);
extern bool cbuiltin_type_info_set_long(cbuiltin_type_info* self);

extern tree_builtin_type_kind cbuiltin_type_info_get_type(const cbuiltin_type_info* self);

typedef struct
{
        tree_decl_storage_class class_;
        tree_type* typespec;
        tree_function_specifier_kind funcspec;
        tree_xlocation loc;
        bool is_typedef;
} cdecl_specs;

extern void cdecl_specs_init(cdecl_specs* self);
extern void cdecl_specs_set_start_loc(cdecl_specs* self, tree_location start_loc);
extern void cdecl_specs_set_end_loc(cdecl_specs* self, tree_location end_loc);

extern tree_location cdecl_specs_get_start_loc(const cdecl_specs* self);

typedef enum
{
        CDK_UNKNOWN,
        CDK_TYPE_NAME,
        CDK_PARAM,
        CDK_MEMBER,
} cdeclarator_kind;

typedef struct
{
        tree_type* head;
        tree_type* tail;
} ctype_chain;

extern void ctype_chain_init(ctype_chain* self);

typedef struct _cdeclarator
{
        cdeclarator_kind kind;
        ctype_chain type;
        tree_id id;
        tree_xlocation loc;
        tree_location id_loc;
        objgroup params;
        bool params_initialized;
} cdeclarator;

extern void cdeclarator_init(cdeclarator* self, ctree_context* context, cdeclarator_kind k);
extern void cdeclarator_dispose(cdeclarator* self);

extern void cdeclarator_set_id(cdeclarator* self, tree_location id_loc, tree_id id);
extern void cdeclarator_set_initialized(cdeclarator* self);
extern void cdeclarator_set_start_loc(cdeclarator* self, tree_location start_loc);
extern void cdeclarator_set_end_loc(cdeclarator* self, tree_location end_loc);

extern tree_location cdeclarator_get_id_loc_or_begin(const cdeclarator* self);

typedef struct
{
        cdecl_specs specs;
        cdeclarator declarator;
} cparam;

extern cparam* cparam_new(ctree_context* context);
extern void cparam_delete(ctree_context* context, cparam* p);

extern tree_xlocation cparam_get_loc(const cparam* self);

typedef enum _cstmt_context
{
        CSC_NONE = 0,
        CSC_ITERATION = 1,
        CSC_SWITCH = 2,
} cstmt_context;

extern tree_scope_flags cstmt_context_to_scope_flags(cstmt_context c);

#ifdef __cplusplus
}
#endif

#endif // !CTREE_H
