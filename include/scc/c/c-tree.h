#ifndef CTREE_H
#define CTREE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree.h"
#include <stdio.h>
#include <setjmp.h>

typedef struct _csource csource;
typedef struct _file_entry file_entry;

typedef struct _ccontext
{
        base_allocator base_alloc;
        bump_ptr_allocator node_alloc;
        list_head sources;
        bool use_tags;
        tree_context* tree;
} ccontext;

extern void cinit(ccontext* self, tree_context* tree, jmp_buf on_bad_alloc);
extern void cinit_ex(ccontext* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc);

extern void cdispose(ccontext* self);

extern csource* csource_new(ccontext* context, file_entry* entry);
extern void csource_delete(ccontext* context, csource* source);

extern hval ctree_id_to_key(const ccontext* self, tree_id id, bool is_tag);
extern hval cget_decl_key(const ccontext* self, const tree_decl* decl);

static inline tree_context* cget_tree(const ccontext* self)
{
        return self->tree;
}

static inline allocator* cget_alloc(ccontext* self)
{
        return base_allocator_base(&self->base_alloc);
}

static inline void* callocate(ccontext* self, ssize bytes)
{
        return bump_ptr_allocate(&self->node_alloc, bytes);
}

typedef struct _csizeof_operand
{
        bool unary;
        tree_location loc;
        union
        {
                tree_type* type;
                tree_expr* expr;
                void* pointer;
        };
} csizeof_operand;

extern void csizeof_expr_init(csizeof_operand* self, tree_expr* e);
extern void csizeof_type_init(csizeof_operand* self, tree_type* t, tree_location loc);

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

typedef struct _cdecl_specs
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
        tree_location id_loc;
        tree_xlocation loc;
        dseq params;
        bool params_initialized;
        ccontext* context;
} cdeclarator;

extern void cdeclarator_init(cdeclarator* self, ccontext* context, cdeclarator_kind k);
extern void cdeclarator_dispose(cdeclarator* self);

extern void cdeclarator_set_id(cdeclarator* self, tree_location id_loc, tree_id id);
extern void cdeclarator_set_initialized(cdeclarator* self);
extern void cdeclarator_set_loc_begin(cdeclarator* self, tree_location start_loc);
extern void cdeclarator_set_loc_end(cdeclarator* self, tree_location end_loc);
extern tree_type* cdeclarator_get_type(const cdeclarator* self);

extern tree_location cdeclarator_get_id_loc_or_begin(const cdeclarator* self);

typedef struct _cparam
{
        cdecl_specs specs;
        cdeclarator declarator;
} cparam;

extern cparam* cparam_new(ccontext* context);
extern void cparam_delete(ccontext* context, cparam* p);
extern tree_type* cparam_get_type(const cparam* self);
extern tree_xlocation cparam_get_loc(const cparam* self);

typedef enum _cstmt_context
{
        CSC_NONE = 0,
        CSC_ITERATION = 1,
        CSC_SWITCH = 2,
        CSC_DECL = 4,
} cstmt_context;

extern tree_scope_flags cstmt_context_to_scope_flags(cstmt_context c);

#ifdef __cplusplus
}
#endif

#endif // !CTREE_H
