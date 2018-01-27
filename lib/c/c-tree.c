#include "scc/c/c-tree.h"
#include "scc/c/c-token.h"
#include "scc/c/c-error.h"
#include "scc/c/c-source.h"
#include <setjmp.h>

extern void cinit(ccontext* self, tree_context* tree, jmp_buf on_bad_alloc)
{
        cinit_ex(self, tree, on_bad_alloc, STDALLOC);
}

extern void cinit_ex(ccontext* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc)
{
        self->use_tags = true;
        self->tree = tree;

        base_allocator_init_ex(&self->base_alloc, NULL, on_bad_alloc, alloc);
        bump_ptr_allocator_init_ex(&self->node_alloc, cget_alloc(self));
        list_init(&self->sources);
}

extern void cdispose(ccontext* self)
{
        while (!list_empty(&self->sources))
                csource_delete(self, (csource*)list_pop_back(&self->sources));

        bump_ptr_allocator_dispose(&self->node_alloc);
        base_allocator_dispose(&self->base_alloc);
}

extern csource* csource_new(ccontext* context, file_entry* entry)
{
        csource* s = allocate(cget_alloc(context), sizeof(*s));
        s->_begin = TREE_INVALID_LOC;
        s->_end = TREE_INVALID_LOC;
        s->_file = entry;
        list_node_init(&s->_node);
        dseq_init_ex_u32(&s->_lines, cget_alloc(context));
        list_push_back(&context->sources, &s->_node);
        return s;
}

extern void csource_delete(ccontext* context, csource* source)
{
        if (!source)
                return;

        file_close(source->_file);
        list_node_remove(&source->_node);
        deallocate(cget_alloc(context), source);
}

extern hval ctree_id_to_key(const ccontext* self, tree_id id, bool is_tag)
{
        if (!self->use_tags)
                return (hval)id;

        hval key = id >> 1;
        return is_tag
                ? key | (1U << 31)
                : key;
}

extern hval cget_decl_key(const ccontext* self, const tree_decl* decl)
{
        return ctree_id_to_key(self, tree_get_decl_name(decl),
                tree_decl_is(decl, TDK_ENUM) || tree_decl_is(decl, TDK_RECORD));
}

extern void csizeof_expr_init(csizeof_operand* self, tree_expr* e)
{
        self->unary = true;
        self->expr = e;
        self->loc = tree_get_expr_loc(e);
}

extern void csizeof_type_init(csizeof_operand* self, tree_type* t, tree_location loc)
{
        self->unary = false;
        self->type = t;
        self->loc = loc;
}

extern void cinit_iterator_init(cinit_iterator* self, tree_type* entity)
{
        self->entity = tree_desugar_type(entity);
        self->member_pos = NULL;
        self->array_pos = -1;
        self->type_iterated = false;
}

extern bool cinit_iterator_at_record(const cinit_iterator* self)
{
        return tree_declared_type_is(self->entity, TDK_RECORD);
}

extern bool cinit_iterator_at_array(const cinit_iterator* self)
{
        return tree_type_is(self->entity, TTK_ARRAY);
}

extern bool cinit_iterator_advance(cinit_iterator* self)
{
        if (cinit_iterator_at_record(self))
        {
                tree_decl* record = tree_get_decl_type_entity(self->entity);
                if (!self->member_pos)
                {
                        self->member_pos = tree_get_record_members_begin(record);
                        return true;
                }

                if (self->member_pos == tree_get_record_members_end(record))
                        return false;

                self->member_pos = tree_get_next_member(self->member_pos);
                return true;
        }
        else if (cinit_iterator_at_array(self))
        {
                // todo: check range
                self->array_pos++;
                return true;
        }
        else if (!self->type_iterated)
        {
                self->type_iterated = true;
                return true;
        }
        return false;
}

extern tree_type* cinit_iterator_get_pos_type(cinit_iterator* self)
{
        if (cinit_iterator_at_record(self))
                return tree_get_decl_type(self->member_pos);
        else if (cinit_iterator_at_array(self))
                return tree_get_array_eltype(self->entity);
        return self->entity;
}

extern void cbuiltin_type_info_init(cbuiltin_type_info* self)
{
        self->base = TBTK_INVALID;
        self->nshort = 0;
        self->nlong = 0;
        self->nsigned = 0;
        self->nunsigned = 0;
}

extern bool cbuiltin_type_info_set_base(cbuiltin_type_info* self, tree_builtin_type_kind base)
{
        if (self->base != TBTK_INVALID)
                return false; // base is already set

        bool has_size_specs = self->nlong || self->nshort;
        bool has_sign_specs = self->nsigned || self->nunsigned;
        bool has_specs = has_size_specs || has_sign_specs;
        if (base == TBTK_VOID || base == TBTK_FLOAT)
        {
                if (has_specs)
                        return false;
        }
        else if (base == TBTK_INT32)
                ;
        else if (base == TBTK_INT8)
        {
                if (has_size_specs)
                        return false;
        }
        else if (base == TBTK_DOUBLE)
        {
                if (self->nshort || self->nlong > 2)
                        return false;
        }
        else
                S_UNREACHABLE(); // invalid base type

        self->base = base;
        return true;
}

extern tree_builtin_type_kind cbuiltin_type_info_get_type(const cbuiltin_type_info* self)
{
        tree_builtin_type_kind base = self->base;
        if (base == TBTK_VOID || base == TBTK_FLOAT || base == TBTK_DOUBLE)
                return base;
        else if (base == TBTK_INT8)
                return self->nunsigned ? TBTK_UINT8 : TBTK_INT8;
        else if (base == TBTK_INT32 || base == TBTK_INVALID)
        {
                if (base == TBTK_INVALID
                    && !self->nshort
                    && !self->nlong
                    && !self->nsigned
                    && !self->nunsigned)
                        return false; // type info was not set

                tree_builtin_type_kind t = self->nunsigned ? TBTK_UINT32 : TBTK_INT32;
                if (self->nlong == 2)
                        t = self->nunsigned ? TBTK_UINT64 : TBTK_INT64;
                else if (self->nshort)
                        t = self->nunsigned ? TBTK_UINT16 : TBTK_INT16;

                return t;
        }

        S_UNREACHABLE();
        return TBTK_INVALID;
}

static inline bool cbuiltin_type_can_have_size_or_sign_specifiers(tree_builtin_type_kind k)
{
        return k == TBTK_INVALID || k == TBTK_INT8 || k == TBTK_INT32;
}

extern bool cbuiltin_type_info_set_signed(cbuiltin_type_info* self)
{
        if (self->nunsigned || !cbuiltin_type_can_have_size_or_sign_specifiers(self->base))
                return false;

        self->nsigned++;
        return true;
}

extern bool cbuiltin_type_info_set_unsigned(cbuiltin_type_info* self)
{
        if (self->nsigned || !cbuiltin_type_can_have_size_or_sign_specifiers(self->base))
                return false;

        self->nunsigned++;
        return true;
}

extern bool cbuiltin_type_info_set_short(cbuiltin_type_info* self)
{
        if (self->nlong || self->nshort
        || !cbuiltin_type_can_have_size_or_sign_specifiers(self->base))
                return false;

        self->nshort++;
        return true;
}

extern bool cbuiltin_type_info_set_long(cbuiltin_type_info* self)
{

        if (self->nshort || self->nlong == 2
        || !cbuiltin_type_can_have_size_or_sign_specifiers(self->base))
                return false;

        self->nlong++;
        return true;
}

extern void cdecl_specs_init(cdecl_specs* self)
{
        self->class_ = TDSC_NONE;
        self->typespec = NULL;
        self->funcspec = TFSK_NONE;
        self->is_typedef = false;
        self->loc = TREE_INVALID_XLOC;
}

extern void cdecl_specs_set_start_loc(cdecl_specs* self, tree_location start_loc)
{
        self->loc = tree_set_xloc_begin(self->loc, start_loc);
}

extern void cdecl_specs_set_end_loc(cdecl_specs* self, tree_location end_loc)
{
        self->loc = tree_set_xloc_end(self->loc, end_loc);
}

extern tree_location cdecl_specs_get_start_loc(const cdecl_specs* self)
{
        return tree_get_xloc_begin(self->loc);
}

extern void ctype_chain_init(ctype_chain* self)
{
        self->head = NULL;
        self->tail = NULL;
}

extern void cdeclarator_init(cdeclarator* self, ccontext* context, cdeclarator_kind k)
{
        self->kind = k;
        self->id = tree_get_empty_id();
        self->params_initialized = false;
        self->loc = TREE_INVALID_XLOC;
        self->id_loc = TREE_INVALID_LOC;
        self->context = context;

        ctype_chain_init(&self->type);
        dseq_init_ex_ptr(&self->params, cget_alloc(context));
}

extern void cdeclarator_dispose(cdeclarator* self)
{
        for (void** p = dseq_begin_ptr(&self->params);
                p != dseq_end_ptr(&self->params); p++)
        {
                cparam_delete(self->context, *p);
        }
}

extern void cdeclarator_set_id(cdeclarator* self, tree_location id_loc, tree_id id)
{
        self->id = id;
        self->id_loc = id_loc;
}

extern void cdeclarator_set_initialized(cdeclarator* self)
{
        self->params_initialized = true;
}

extern void cdeclarator_set_loc_begin(cdeclarator* self, tree_location start_loc)
{
        self->loc = tree_set_xloc_begin(self->loc, start_loc);
}

extern void cdeclarator_set_loc_end(cdeclarator* self, tree_location end_loc)
{
        self->loc = tree_set_xloc_end(self->loc, end_loc);
}

extern tree_type* cdeclarator_get_type(const cdeclarator* self)
{
        return self->type.head;
}

extern tree_location cdeclarator_get_id_loc_or_begin(const cdeclarator* self)
{
        return self->id_loc == TREE_INVALID_LOC
                ? tree_get_xloc_begin(self->loc)
                : self->id_loc;
}

extern cparam* cparam_new(ccontext* context)
{
        //todo
        cparam* p = callocate(context, sizeof(cparam));
        cdecl_specs_init(&p->specs);
        cdeclarator_init(&p->declarator, context, CDK_PARAM);
        return p;
}

extern void cparam_delete(ccontext* context, cparam* p)
{
        cdeclarator_dispose(&p->declarator);
}

extern tree_type* cparam_get_type(const cparam* self)
{
        return cdeclarator_get_type(&self->declarator);
}

extern tree_xlocation cparam_get_loc(const cparam* self)
{
        return self->specs.loc;
}

extern tree_scope_flags cstmt_context_to_scope_flags(cstmt_context c)
{
        tree_scope_flags flags = TSF_NONE;
        if (c & CSC_ITERATION)
                flags |= TSF_ITERATION;
        else if (c & CSC_SWITCH)
                flags |= TSF_SWITCH;
        return flags;
}