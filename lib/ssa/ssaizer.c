#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/tree/tree-module.h"
#include "scc/core/dseq-instance.h"

#define DSEQ_VALUE_TYPE strmap
#define DSEQ_TYPE strmap_stack
#define DSEQ_INIT strmap_stack_init
#define DSEQ_INIT_ALLOC strmap_stack_init_alloc
#define DSEQ_DISPOSE strmap_stack_dispose
#define DSEQ_GET_SIZE strmap_stack_size
#define DSEQ_GET_CAPACITY strmap_stack_capacity
#define DSEQ_GET_ALLOCATOR strmap_stack_allocator
#define DSEQ_RESERVE strmap_stack_reserve
#define DSEQ_RESIZE strmap_stack_resize
#define DSEQ_GET_BEGIN strmap_stack_begin
#define DSEQ_GET_END strmap_stack_end
#define DSEQ_GET strmap_stack_get
#define DSEQ_SET strmap_stack_set
#define DSEQ_APPEND strmap_stack_append

#include "scc/core/dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

#define HTAB_TYPE ptrset
#define HTAB_IMPL_FN_GENERATOR(NAME) _ptrset_##NAME
#define HTAB_KEY_TYPE const void*
#define HTAB_DELETED_KEY ((const void*)0)
#define HTAB_EMPTY_KEY ((const void*)1)
#define HTAB_VALUE_TYPE uint8_t
#define HTAB_INIT ptrset_init
#define HTAB_INIT_ALLOC ptrset_init_alloc
#define HTAB_DISPOSE ptrset_dispose
#define HTAB_GET_SIZE ptrset_size
#define HTAB_GET_ALLOCATOR ptrset_alloc
#define HTAB_RESERVE ptrset_reserve
#define HTAB_CLEAR ptrset_clear
#define HTAB_ERASE ptrset_erase
#define HTAB_GROW ptrset_grow
#define HTAB_INSERT ptrset_insert
#define HTAB_FIND ptrset_find

#define HTAB_ITERATOR_TYPE ptrset_iter
#define HTAB_ITERATOR_GET_KEY ptrset_iter_key
#define HTAB_ITERATOR_ADVANCE ptrset_iter_advance
#define HTAB_ITERATOR_INIT ptrset_iter_init
#define HTAB_ITERATOR_CREATE ptrset_iter_create
#define HTAB_ITERATOR_IS_VALID ptrset_iter_valid
#define HTAB_ITERATOR_GET_VALUE ptrset_iter_value

#include "scc/core/htab.h"

#undef HTAB_TYPE
#undef HTAB_IMPL_FN_GENERATOR
#undef HTAB_KEY_TYPE 
#undef HTAB_DELETED_KEY
#undef HTAB_EMPTY_KEY
#undef HTAB_VALUE_TYPE
#undef HTAB_INIT
#undef HTAB_INIT_ALLOC
#undef HTAB_DISPOSE
#undef HTAB_GET_SIZE
#undef HTAB_GET_ALLOCATOR
#undef HTAB_RESERVE
#undef HTAB_CLEAR
#undef HTAB_ERASE
#undef HTAB_GROW
#undef HTAB_INSERT
#undef HTAB_FIND
#undef HTAB_ITERATOR_TYPE
#undef HTAB_ITERATOR_GET_KEY
#undef HTAB_ITERATOR_ADVANCE
#undef HTAB_ITERATOR_INIT
#undef HTAB_ITERATOR_CREATE
#undef HTAB_ITERATOR_IS_VALID
#undef HTAB_ITERATOR_GET_VALUE

#define DSEQ_VALUE_TYPE ssa_tm_info
#define DSEQ_TYPE ssa_tm_info_stack
#define DSEQ_INIT ssa_tm_info_stack_init
#define DSEQ_INIT_ALLOC ssa_tm_info_stack_init_alloc
#define DSEQ_DISPOSE ssa_tm_info_stack_dispose
#define DSEQ_GET_SIZE ssa_tm_info_stack_size
#define DSEQ_GET_CAPACITY ssa_tm_info_stack_capacity
#define DSEQ_GET_ALLOCATOR ssa_tm_info_stack_allocator
#define DSEQ_RESERVE ssa_tm_info_stack_reserve
#define DSEQ_RESIZE ssa_tm_info_stack_resize
#define DSEQ_GET_BEGIN ssa_tm_info_stack_begin
#define DSEQ_GET_END ssa_tm_info_stack_end
#define DSEQ_GET ssa_tm_info_stack_get
#define DSEQ_SET ssa_tm_info_stack_set
#define DSEQ_APPEND ssa_tm_info_stack_append

#include "scc/core/dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        self->function = NULL;
        self->module = NULL;

        self->block = NULL;
        ssa_init_builder(&self->builder, self->context, NULL);

        self->tm.read = NULL;
        self->tm.write = NULL;
        self->tm.transaction_type = NULL;
        self->tm.init = NULL;
        self->tm.commit = NULL;
        self->tm.reset = NULL;

        allocator* alloc = ssa_get_alloc(context);
        strmap_stack_init(&self->defs);
        strmap_init_alloc(&self->labels, alloc);
        strmap_init_alloc(&self->globals, alloc);
        ptrset_init_alloc(&self->emitted_records, alloc);
        dseq_init_alloc(&self->continue_stack, alloc);
        dseq_init_alloc(&self->break_stack, alloc);
        dseq_init_alloc(&self->switch_stack, alloc);
        ssa_tm_info_stack_init_alloc(&self->tm_info_stack, alloc);
}

extern void ssaizer_dispose(ssaizer* self)
{
        // todo
}

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block)
{
        self->block = block;
        ssa_builder_set_pos(&self->builder, ssa_get_block_instrs_end(self->block), false);
}

extern void ssaizer_finish_block(ssaizer* self, ssa_block* block)
{
        ssa_add_function_block(self->function, block);
}

extern void ssaizer_finish_current_block(ssaizer* self)
{
        ssaizer_finish_block(self, self->block);
        self->block = NULL;
}

extern ssa_block* ssaizer_new_block(ssaizer* self)
{
        return ssa_new_block(self->context, ssa_builder_gen_uid(&self->builder));
}

extern bool ssaizer_current_block_is_terminated(const ssaizer* self)
{
        assert(self->block);
        ssa_instr* terminator = ssa_get_block_terminator(self->block);
        return terminator && ssa_get_instr_kind(terminator) == SIK_TERMINATOR;
}

static inline strmap* ssaizer_get_last_scope(const ssaizer* self)
{
        assert(dseq_size(&self->defs));
        return strmap_stack_end(&self->defs) - 1;
}

extern void ssaizer_push_scope(ssaizer* self)
{
        strmap_stack_resize(&self->defs, strmap_stack_size(&self->defs) + 1);
        strmap_init_alloc(ssaizer_get_last_scope(self), ssa_get_alloc(self->context));
}

extern void ssaizer_pop_scope(ssaizer* self)
{
        strmap_dispose(ssaizer_get_last_scope(self));
        strmap_stack_resize(&self->defs, strmap_stack_size(&self->defs) - 1);
}

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* def)
{
        assert(def);
        strmap_iter it;
        strmap* last = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        assert(!strmap_find(last, id, &it));
        strmap_insert(last, id, def);
}

extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var)
{
        strmap* first = strmap_stack_begin(&self->defs);
        strmap* it = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);
        strmap_iter res;

        while (it >= first)
        {
                if (strmap_find(it, id, &res))
                        return *strmap_iter_value(&res);
                it--;
        }
        return ssaizer_get_global_decl(self, var);
}

extern ssa_value* ssaizer_get_global_decl(ssaizer* self, const tree_decl* decl)
{
        strmap_iter res;
        return strmap_find(&self->globals, tree_get_decl_name(decl), &res)
                ? *strmap_iter_value(&res)
                : NULL;
}

extern void ssaizer_set_global_decl(ssaizer* self, tree_decl* decl, ssa_value* val)
{
        assert(decl);
        strmap_insert(&self->globals, tree_get_decl_name(decl), val);
}

extern bool ssaizer_record_is_emitted(const ssaizer* self, const tree_decl* record)
{
        ptrset_iter i;
        return ptrset_find(&self->emitted_records, record, &i);
}

extern void ssaizer_set_record_emitted(ssaizer* self, const tree_decl* record)
{
        assert(record);
        ptrset_insert(&self->emitted_records, record, 0);
}

extern ssa_block* ssaizer_get_label_block(ssaizer* self, const tree_decl* label)
{
        tree_id id = tree_get_decl_name(label);
        strmap_iter res;
        if (strmap_find(&self->labels, id, &res))
                return *strmap_iter_value(&res);

        ssa_block* b = ssaizer_new_block(self);
        strmap_insert(&self->labels, id, b);
        return b;
}

extern void ssaizer_push_continue_dest(ssaizer* self, ssa_block* block)
{
        dseq_append(&self->continue_stack, block);
}

extern void ssaizer_push_break_dest(ssaizer* self, ssa_block* block)
{
        dseq_append(&self->break_stack, block);
}

extern void ssaizer_push_switch_instr(ssaizer* self, ssa_instr* switch_instr)
{
        dseq_append(&self->switch_stack, switch_instr);
}

extern void ssaizer_push_tm_info(ssaizer* self, ssa_value* transaction, ssa_block* entry, ssa_block* reset)
{
        ssa_tm_info i = { transaction, entry, reset };
        ssa_tm_info_stack_append(&self->tm_info_stack, i);
}

extern void ssaizer_pop_continue_dest(ssaizer* self)
{
        size_t size = dseq_size(&self->continue_stack);
        assert(size);
        dseq_resize(&self->continue_stack, size - 1);
}

extern void ssaizer_pop_break_dest(ssaizer* self)
{
        size_t size = dseq_size(&self->break_stack);
        assert(size);
        dseq_resize(&self->break_stack, size - 1);
}

extern void ssaizer_pop_switch_instr(ssaizer* self)
{
        size_t size = dseq_size(&self->switch_stack);
        assert(size);
        dseq_resize(&self->switch_stack, size - 1);
}

extern void ssaizer_pop_tm_info(ssaizer* self)
{
        size_t size = ssa_tm_info_stack_size(&self->tm_info_stack);
        assert(size);
        ssa_tm_info_stack_resize(&self->tm_info_stack, size - 1);
}

extern ssa_block* ssaizer_get_continue_dest(ssaizer* self)
{
        return *(dseq_end(&self->continue_stack) - 1);
}

extern ssa_block* ssaizer_get_break_dest(ssaizer* self)
{
        return *(dseq_end(&self->break_stack) - 1);
}

extern ssa_instr* ssaizer_get_switch_instr(ssaizer* self)
{
        return *(dseq_end(&self->switch_stack) - 1);
}

extern ssa_tm_info* ssaizer_get_last_tm_info(ssaizer* self)
{
        return ssa_tm_info_stack_end(&self->tm_info_stack) - 1;
}

extern ssa_tm_info* ssaizer_get_tm_info(ssaizer* self, size_t i)
{
        return ssa_tm_info_stack_begin(&self->tm_info_stack) + i;
}

extern size_t ssaizer_get_tm_info_size(ssaizer* self)
{
        return ssa_tm_info_stack_size(&self->tm_info_stack);
}

extern bool ssaizer_in_atomic_block(const ssaizer* self)
{
        return ssa_tm_info_stack_size(&self->tm_info_stack) != 0;
}

static bool ssaize_tm_decls(ssaizer* self, const tree_module* module, const tree_module* tm_decls)
{
        tree_context* context = self->context->tree;

        enum
        {
                READ_IDX,
                WRITE_IDX,
                INIT_IDX,
                COMMIT_IDX,
                RESET_IDX,
                TRANSACTION_ID_IDX,
                SIZE,
        };

        const char* decl_names[] =
        {
                "_tm_read",
                "_tm_write",
                "_tm_transaction_init",
                "_tm_transaction_commit",
                "_tm_transaction_reset",
                "_tm_transaction_id",
        };

        tree_decl* decls[SIZE];

        // todo: merge modules?
        for (int i = 0; i < SIZE; i++)
                if (!(decls[i] = tree_module_lookup_s(module, TLK_DECL, decl_names[i])))
                        if (!(decls[i] = tree_module_lookup_s(tm_decls, TLK_DECL, decl_names[i])))
                                return false;

        tree_decl* transaction;
        if (!(transaction = tree_module_lookup_s(module, TLK_TAG, "_tm_transaction")))
                if (!(transaction = tree_module_lookup_s(tm_decls, TLK_TAG, "_tm_transaction")))
                        return false;
        if (!tree_decl_is(transaction, TDK_RECORD))
                return false;

        for (int i = 0; i < SIZE; i++)
                if (!ssaize_decl(self, decls[i]))
                        return false;
        if (!ssaize_decl(self, transaction))
                return false;

        self->tm.read = ssaizer_get_global_decl(self, decls[READ_IDX]);
        self->tm.write = ssaizer_get_global_decl(self, decls[WRITE_IDX]);
        self->tm.init = ssaizer_get_global_decl(self, decls[INIT_IDX]);
        self->tm.commit = ssaizer_get_global_decl(self, decls[COMMIT_IDX]);
        self->tm.reset = ssaizer_get_global_decl(self, decls[RESET_IDX]);
        self->tm.transaction_id = ssaizer_get_global_decl(self, decls[TRANSACTION_ID_IDX]);
        self->tm.transaction_type = tree_new_decl_type(context, transaction, true);
        return true;
}

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module, const tree_module* tm_decls)
{
        self->module = ssa_new_module(self->context);

        if (tm_decls && !ssaize_tm_decls(self, module, tm_decls))
                return NULL;

        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_FOREACH_DECL_IN_SCOPE(globals, decl)
                if (!ssaize_decl(self, decl))
                        return NULL;

        return self->module;
}